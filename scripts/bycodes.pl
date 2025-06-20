#!/usr/bin/env perl
# $LynxId: bycodes.pl,v 1.6 2025/06/19 10:01:25 tom Exp $
# retrieve and transform W3C's named entities into two files:
# named-entities.h (for inclusion in Lynx's entities.h)
# named-entities.html (for testing)

use strict;
use warnings;

our $source = "bycodes.html";
our $remote = "https://www.w3.org/TR/xml-entity-names/$source";

our %code2note;
our %name2code;

sub read_file($) {
    my $path = shift;
    my @result;
    if ( open my $fh, $path ) {
        @result = <$fh>;
        chomp @result;
        close $fh;
    }
    return @result;
}

sub get_source() {
    if ( !-f $source ) {
        system("wget $remote");
    }
    if ( !-f $source ) {
        printf "cannot retrieve $remote!\n";
        exit 1;
    }
}

sub trim($) {
    my $value = shift;
    $value =~ s/^\s+//;
    $value =~ s/\s+$//;
    $value =~ s/\s+/ /g;
    return $value;
}

sub make_header() {
    my @data = read_file("tidy -q -wrap 4096 < $source |");
    for my $row ( 0 .. $#data ) {
        my $line = $data[$row];
        next unless ( $line =~ /\<a name="U[[:xdigit:]]{4,}"/ );

        my $code = $line;
        my $p    = index( $line, "U+" );
        next unless $p > 0;
        $code = substr( $line, $p + 2 );
        $code =~ s/\<.*$//;
        $code = hex $code;

        my $note = $line;
        $p = rindex( $line, "</a>," );
        next unless ( $p > 0 );
        $note = substr( $line, $p + 5 );
        $note =~ s/,.*//;
        $note = trim($note);

        $code2note{$code} = $note;

        my $names = $line;
        if ( $names =~ /\s,\s/ ) {
            $names =~ s/^.*\s,\s+//;
        }
        else {
            $names =~ s/^.*,\s+//;
        }
        my @names = split /,\s*/, $names;

        for my $n ( 0 .. $#names ) {
            $name2code{ $names[$n] } = $code;
        }
    }
    my $header = "named-entities.h";
    open( my $fp, ">", $header ) || die "? $header $!";
    for my $name ( sort keys %name2code ) {
        my $code = $name2code{$name};
        my $note = $code2note{$code};
        my $pad  = "";
        $pad = sprintf( "%*s", 16 - length($name), " " )
          if ( length($name) < 16 );
        printf $fp "  { \"%s\",%s%6d}, /* %s */\n", $name, $pad, $code, $note;
    }
    close $fp;
}

sub make_sample() {
    my $sample = "named-entities.html";
    open( my $fp, ">", $sample ) || die "? $sample $!";
    printf $fp "<!DOCTYPE html>\n";
    printf $fp "<html>\n";
    printf $fp "<head>\n";
    printf $fp "<title>Named entities and codes</title>\n";
    printf $fp "<meta\n";
    printf $fp "  http-equiv=\"Content-Type\"\n";
    printf $fp "  content=\"text/html; charset=utf-8\"\n";
    printf $fp ">\n";
    printf $fp "</head>\n";
    printf $fp "<body>\n";
    printf $fp "<pre>\n";

    for my $name ( sort keys %name2code ) {
        my $code = $name2code{$name};
        my $CODE = sprintf( "U+%04X", $code );
        printf $fp
          "%-24s %8s %8d  name \"&%s;\"  hex \"&#x%x;\"  dec \"&#%d;\"\n",
          $name,
          $CODE,
          $code,
          $name,
          $code,
          $code;
    }
    printf $fp "</pre>\n";
    printf $fp "</body>\n";
    printf $fp "</html>\n";
    close $fp;
}

sub doit() {
    &get_source;
    &make_header;
    &make_sample;
}

&doit;

1;
