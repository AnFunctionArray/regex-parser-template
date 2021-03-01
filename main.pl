#!/usr/bin/perl

use re 'eval';

use experimental 'switch';

use File::Basename;

$filename = "metaregex.regex";
open my $fh, '<', $filename or die "error opening $filename: $!";

my $metaregex = do { local $/; <$fh> };

close $fh;

$filename = "main.regex";
open my $fh, '<', $filename or die "error opening $filename: $!";

my $main = do { local $/; <$fh> };

close $fh;

my @entries;

my $mainregexfinal = "((*F)";

my $entryregex;

$main =~ /$metaregex/;

parseregexfile((substr $main, length $&), 1);

$mainregexfinal = $mainregexfinal . ")";#|" . $entryregex;  

$mainregexfinal =~s/\s|\n//g if(not $matchinperl);

startmatching($mainregexfinal, \@entries) if(not $matchinperl);

sub parseregexfile {

    if(not defined $_[1])
    {
        my $filename = $_[0];
        open my $fh, '<', $filename or die "error opening $filename: $!";

        $regexfilecontent = do { local $/; <$fh> };

        close $fh;

    } else {
        $regexfilecontent = $_[0];
    }

    my $regexfile = $regexfilecontent; 
    
    $regexfile =~s/(?<!<)#restrictoutsidefacet\b//g;

    $regexfile =~s/\(\?<#restrictoutsidefacet>/(/g;

    $regexfile =~s/[(]\?&(\w+?)#nofacet[)]/(?&$1)/g;

    my $regexfilecontentcopy = $regexfilecontent;

    sub replacefacetgroups {
        $regexfilecontent =~s/\Q$_[0]\E(facet)?>/(/g;
    }

    replacefacetgroups($1, $_[1]) while($regexfilecontentcopy =~/(\(\?<\w+)facet>/g);

    $regexfilecontent =~s/\(\?C(\d++)\)//g;

    $regexfilecontent =~s/[(]\?&(\w+?)(facet)?[)]/(?&$1facet)/g;

    $regexfilecontent =~s/[(]\?&(\w+?)#nofacet[)]/(?&$1)/g;

    $regexfilecontent =~s/(\(\?<\w+)>/$1facet>/g;

    $regexfilecontent =~s/(\(\?<\w+)#restrictoutsidefacet>/$1facet>(*F)/g;

    $regexfilecontent =~s/\(\?<#restrictoutsidefacet>/((*F)/g;

    $mainregexfinal = $mainregexfinal . $regexfile . $regexfilecontent;
}