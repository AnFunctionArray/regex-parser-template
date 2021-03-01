#define PERL_NO_GET_CONTEXT
#define PERL_EUPXS_ALWAYS_EXPORT
#pragma include_alias( "config.h" , "C:\Users\sasho\perl-5.32.0\win32\config.h" )
#pragma include_alias( <arpa/inet.h> , <C:\Users\sasho\perl-5.32.0\win32\include\arpa\inet.h> )
#pragma include_alias( <sys/socket.h> , <C:\Users\sasho\perl-5.32.0\win32\include\sys\socket.h> )
#pragma include_alias( <dirent.h> , <C:\Users\sasho\perl-5.32.0\win32\include\dirent.h> )
#pragma include_alias( <win32thread.h> , <C:\Users\sasho\perl-5.32.0\win32\win32thread.h> )
#pragma include_alias( <netdb.h> , <C:\Users\sasho\perl-5.32.0\win32\include\netdb.h> )
#pragma include_alias( "win32.h" , "C:\Users\sasho\perl-5.32.0\win32\win32.h" )
#pragma include_alias( "win32iop.h" , "C:\Users\sasho\perl-5.32.0\win32\win32iop.h" )
#include "C:\Users\sasho\perl-5.32.0\EXTERN.h"
#include "C:\Users\sasho\perl-5.32.0\perl.h"
#include "C:\Users\sasho\perl-5.32.0\XSUB.h"
//#define PCRE2_CODE_UNIT_WIDTH 8
//#define PCRE2_STATIC

//#include <pcre2.h>
//#include "main.h"

MODULE = _6a4h8

PROTOTYPES: ENABLE

void startmatching(SV *in1, AV *in2)
PREINIT:
    STRLEN len;
    SSize_t thirdlen;
    char* s;
    STRLEN *szelementslens;
    char **pstrings;
CODE:
    {
        thirdlen = av_len(in2) + 1;
        int secondmain(char* pattern, size_t szpattern, char** entries, size_t *entriessz, size_t sznentreis);
        s = SvPVutf8(in1, len);
        szelementslens = malloc(sizeof(size_t) * thirdlen);
        pstrings = malloc(sizeof(char *) * thirdlen);
        for(size_t i = 0; i < thirdlen; ++i)
            pstrings[i] = SvPVutf8(*av_fetch(in2, i, 0), szelementslens[i]);
        secondmain(s, len, pstrings, szelementslens, thirdlen);
        free(szelementslens);
        free(pstrings);
    }