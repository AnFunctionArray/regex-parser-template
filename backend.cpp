#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <filesystem>
#include <optional>
#include <range/v3/view/span.hpp>
#include <range/v3/view/iota.hpp>
#include <range/v3/algorithm/transform.hpp>
#include <range/v3/view.hpp>
#include <range/v3/algorithm/find_if.hpp>
#include <range/v3/algorithm/contains.hpp>
#include <algorithm>
#include <functional>
#include <array>
#include <cassert>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/graph/labeled_graph.hpp>
#include <variant>

extern "C" {
#define PCRE2_CODE_UNIT_WIDTH 8
#define PCRE2_STATIC
#include <pcre2.h>
}

unsigned constexpr stringhash(char const* input) {
	return *input ? static_cast<unsigned int> (*input) +
		33 * stringhash(input + 1)
		: 5381;
}

constexpr inline auto operator"" _h(char const* p, size_t) {
	return stringhash(p);
}

using namespace std::string_literals;

struct regexcontext {

	std::map<std::string, int> nametoindex;

	std::vector<std::optional<std::string>> matches;

	std::string line;

	std::string pattern;

	pcre2_code* pcode;

	int patternflags, origin;
};

std::string  basepattern;

static long long szline{ 1 };

static long long linedebug = 334; //negative - start from line, positive - debug that line, zero - no debug

static std::vector<std::pair<std::string, bool>> entrypoints;

int buildmatchvector(size_t(*offset_vector)[2], int max, regexcontext* pcontext);

std::string getregexbyentrypointnumber(int n);

int compile_pattern(const uint8_t* pattern, size_t szpattern, regexcontext* pcontext);

extern "C" int do_matching(const uint8_t * pattern, size_t szpattern, const uint8_t * subject, size_t szsubject, regexcontext * pcontext);

int callout(pcre2_callout_block* pcalloutblock, void* pvoid) {

	regexcontext* pcontext = (regexcontext*)pvoid;

	buildmatchvector((size_t(*)[2])pcalloutblock->offset_vector, pcalloutblock->capture_top, pcontext);

	if (pcontext->patternflags & PCRE2_AUTO_CALLOUT)

		std::cout << pcontext->pattern.substr(pcalloutblock->pattern_position, pcalloutblock->next_item_length) << std::endl,

		std::cout << pcontext->line.substr(pcalloutblock->current_position) << std::endl;

	if (pcalloutblock->callout_number != 255 && pcontext->origin == 0)
		std::cout << "callout id: " << pcalloutblock->callout_number << std::endl;


	/*switch goes here*/

	return 0;
}

int compile_pattern(const uint8_t* pattern, size_t szpattern, regexcontext* pcontext) {
	int errorcode[1];
	size_t erroroffset[1];
	uint8_t errormsg[0xFF];
	pcontext->pcode = pcre2_compile(pattern, szpattern, pcontext->patternflags, errorcode, erroroffset, 0);
	if (*errorcode != 100)
		pcre2_get_error_message(*errorcode, errormsg, 0xFF),
		printf("pattern error %d at %s : %.*s\n", *errorcode, errormsg, (unsigned int)(szpattern - *erroroffset), pattern + *erroroffset),
		exit(0);
	uint8_t* name_table;
	int namecount, name_entry_size;
	pcre2_pattern_info(pcontext->pcode, PCRE2_INFO_NAMETABLE, &name_table);
	pcre2_pattern_info(pcontext->pcode, PCRE2_INFO_NAMECOUNT, &namecount);
	pcre2_pattern_info(pcontext->pcode, PCRE2_INFO_NAMEENTRYSIZE, &name_entry_size);

	uint8_t* tabptr = name_table;

	int n;

	for (; namecount--; tabptr += name_entry_size)
	{
		n = (tabptr[0] << 8) | tabptr[1];

		pcontext->nametoindex.insert({ std::string{(char*)tabptr + 2}, n });
	}
}

extern "C" int do_matching(const uint8_t * pattern, size_t szpattern, const uint8_t * subject, size_t szsubject, regexcontext * pcontext) {

	pcre2_match_context* match_context = pcre2_match_context_create(0);
	pcre2_match_data* pmatch_data = pcre2_match_data_create(0xFFFF, 0);
	pcre2_set_callout(match_context, callout, pcontext);
	int rc = pcre2_match(pcontext->pcode, subject, szsubject, 0, 0, pmatch_data, match_context);
	//printf("%d\n", rc);
	pcre2_match_data_free(pmatch_data);
	pcre2_match_context_free(match_context);
	return 0;
};

std::string getregexbyentrypointnumber(int n) {
	std::string iregex = basepattern;
	iregex += "|" + (entrypoints[n].second ? std::string{ "^" } : "") + "(?&" + entrypoints[n].first + ")";
	return iregex;
}

extern "C" int secondmain(char* pattern, size_t szpattern, char** entries, size_t * entriessz, size_t sznentreis) {
	extern char** pargv;;
	std::string iregex{ pattern, szpattern };
	std::ifstream isubject{ pargv[0] };

	basepattern = iregex;

	regexcontext contextmain{};

	for (size_t i : ranges::iota_view{ (size_t)0, sznentreis })
		if (std::string entry{ entries[i], entriessz[i] }; entry[0] == '^')
			entrypoints.push_back({ entry.substr(1), true });
		else entrypoints.push_back({ entry, false });

	iregex = getregexbyentrypointnumber(0);

	contextmain.pattern = iregex;
	contextmain.patternflags = linedebug <= 0 ? 0 : PCRE2_AUTO_CALLOUT;

	compile_pattern((uint8_t*)iregex.data(), iregex.size(), &contextmain);

	for (; std::getline(isubject, contextmain.line); ++szline) {
		if (linedebug && (linedebug > 0 ? szline != linedebug : szline < -linedebug)) continue;
		do_matching((uint8_t*)iregex.data(), iregex.size(), (uint8_t*)contextmain.line.data(), contextmain.line.size(), &contextmain);
		if (linedebug > 0) return 0;
	}
	return 0;
}

int buildmatchvector(size_t(*offset_vector)[2], int max, regexcontext* pcontext) {
	pcontext->matches.clear();
	ranges::transform(ranges::span{ offset_vector, max }, std::back_inserter(pcontext->matches), [&](const auto& refarr) {
		return refarr[0] == -1 ? std::optional<std::string> {} : std::optional<std::string>{ pcontext->line.substr(refarr[0], refarr[1] - refarr[0]) };
		});
	return 0;
}