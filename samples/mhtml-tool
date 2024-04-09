#!/usr/bin/env perl

use strict;
use warnings;

$| = 1;

=pod

=head1 NAME

B<mhtml-tool> - Unpack a MIME HTML archive


=head1 SYNOPSIS

B<mhtml-tool> unpacks MIME HTML archives that some browsers (such as Opera)
save by default.  The file extensions of such archives are ".mht" or ".mhtml".

The first HTML file in the archive is taken to be the primary web page, the
other contained files for "page requisites" such as images or frames.  The
primary web page is written to the output directory (the current directory by
default), the requisites to a subdirectory named after the primary HTML file
name without extension, with "_files" appended.  Link URLs in all HTML
files referring to requisites are rewritten to point to the saved files.


=head1 OPTIONS

=over

=item B<-e> program

=item B<--exec> program

Unpack the archive into a temporary directory and execute the given program
in that directory, passing the primary HTML (first text/html entry) of the
archive as a parameter.

If B<-o> is given, use that directory location with the program.

If the program is "lynx" or "(e)links(2)", this script uses the "-force-html"
option to help when the entrypoint lacks a ".html" file-suffix.

=item B<-h>

=item B<-?>

=item B<--help>

Print a brief usage summary.

=item B<-l>

=item B<--list>

List archive contents instead of unpacking.  Four columns are output: file
name, MIME type, size and URL.  Unavailable entries are replaced by "(?)".

=item B<-o> I<directory/ or name>

=item B<--output> I<directory/ or name>

If the argument ends in a slash or is an existing directory, unpack to that
directory instead of current directory.  Otherwise the argument is taken as a
path to the file name to write the primary HTML file to.  If the output
directory does not exist, it is created.

=back


=head1 SEE ALSO

https://github.com/ThomasDickey/mhtml-tool

https://github.com/clapautius/mhtml-tool

http://www.volkerschatz.com/unix/uware/unmht.html

=head1 COPYLEFT

B<unmht> is Copyright (c) 2012 Volker Schatz.  It may be copied and/or
modified under the same terms as Perl.

B<mhtml-tool> is Copyright (c) 2018 Tudor M. Pristavu. It may be copied and/or
modified under the same terms as Perl.

B<mhtml-tool> is Copyright (c) 2024 Thomas E. Dickey. It may be copied and/or
modified under the same terms as Perl.

=cut

use Cwd;
use File::Path;
use File::Copy;
use File::Glob;
use File::Temp qw/ tempdir /;
use URI;
use MIME::Base64;
use MIME::QuotedPrint;
use HTML::PullParser;
use HTML::Tagset;
use Getopt::Long;

our $path_limit = 255;
our %shorten_path;

# RFC 7230 (obsoletes RFC 2616) indicates that URI length is limited by the
# server line-length, which it suggests should be at least 8000 octets.  The
# problem is that components of pathnames might be limited to 255 bytes, while
# the URI components can be much longer.  As an additional complication, the
# too-long components can be both directory- and file-names, and truncating
# the components can produce collisions.
sub shorten_path {
    my $actual = $_[0];
    if ( not defined $shorten_path{$actual} ) {
        my @parts = split /\//, $actual;
        for my $n ( 0 .. $#parts ) {
            $parts[$n] = substr( $parts[$n], 0, $path_limit );
        }
        $shorten_path{$actual} = join "/", @parts;
    }
    return $shorten_path{$actual};
}

# URLs can include "&" and other characters which require quoting to use in
# a shell command.  We use system() rather than exec() to allow the temporary
# directory to be cleaned up after running lynx.
sub quoted {
    my $param = $_[0];
    $param =~ s/'/'\"'\"'/g;
    return sprintf( "'%s'", $param );
}

# Add approriate ordinal suffix to a number.
# -> Number
# <- String of number with ordinal suffix
sub ordinal {
    return $_[0] . "th" if $_[0] > 3 && $_[0] < 20;
    my $unitdig = $_[0] % 10;
    return $_[0] . "st" if $unitdig == 1;
    return $_[0] . "nd" if $unitdig == 2;
    return $_[0] . "rd" if $unitdig == 3;
    return $_[0] . "th";
}

{
    my %taken;

 # Find unique file name.
 # -> Preferred file name, or undef
 #    MHT archive name (as a fallback if no name given)
 # <- File name not conflicting with names returned by previous calls, but which
 #    may exist!
    sub unique_name {
        my ( $fname, $mhtname ) = @_;
        my ( $trunc, $ext );

        if ( defined $fname ) {
            $fname =~ s/^\s+//;
            $fname =~ s/\s+$//;
            $taken{$fname} = 1, return $fname unless $taken{$fname};
            ( $trunc, $ext ) = $fname =~ /^(.*?)(\.\w+)?$/;
            $ext //= "";
        }
        else {
            $trunc = $mhtname || "unpack";
            $trunc =~ s/\.mht(?:ml?)?$//i;
            $ext = "";
        }
        for my $suff ( 1 .. 9999 ) {
            $fname = "${trunc}_$suff$ext";
            $taken{$fname} = 1, return $fname unless $taken{$fname};
            ++$suff;
        }
        return undef;
    }

}

# Output error message and exit with return value 1.
sub abort {
    print STDERR "$_[0]\n";
    exit 1;
}

# Generate output file directories and primary HTML file name depending on
# --output option and primary HTML file name from archive.  In case the primary
# HTML file is not the first file in the archive, the secondary files directory
# is renamed or the files moved on the second call, when the primary HTML file
# name is known.
# -> Value of --output option (or  undef)
#    Primary HTML file name from MHT archive
#    Flag indicating if .._files subdirectory should be created
#    Hash reference to store resulting paths to
sub mkfiledir {
    my ( $outputopt, $firsthtmlname, $needfilesdir, $out ) = @_;

    my $prevfilespath = $$out{filespath};
    $firsthtmlname = "unpackmht-$$" unless defined $firsthtmlname;
    if ( !defined $outputopt ) {
        $$out{toppath} = ".";
    }
    elsif ( -d $outputopt || $outputopt =~ m!/$! ) {
        $$out{toppath} = $outputopt;
    }
    else {
        ( $$out{toppath}, $firsthtmlname ) = $outputopt =~ m!^(.*/)?([^/]+)$!;
        abort "Empty output file name." unless defined $firsthtmlname;
        $firsthtmlname .= ".html"       unless $firsthtmlname =~ /\./;
        $$out{toppath} = "."            unless defined $$out{toppath};
    }
    $$out{toppath} =~ s!/$!!;
    $$out{firstout} = "$$out{toppath}/$firsthtmlname";
    $$out{filesdir} = $firsthtmlname;
    $$out{filesdir} =~ s/\.[^.]+$//;
    $$out{filesdir} = substr( $$out{filesdir}, 0, $path_limit - 6 );
    $$out{filesdir} .= "_files";
    $$out{filespath} = "$$out{toppath}/$$out{filesdir}";

    if ( defined $prevfilespath ) {
        return unless $prevfilespath ne $$out{filespath};
        return unless -d $prevfilespath;
        if ( !-d $$out{filespath} ) {
            File::Copy::move( $prevfilespath, $$out{filespath} )
              or abort "Could not rename secondary files directory.";
        }
        else {
            for ( File::Glob::bsd_glob("$prevfilespath/*") ) {
                File::Copy::move( $_, $$out{filespath} )
                  or abort "Could not move secondary files.";
            }
        }
    }
    else {
        my $createall = $needfilesdir ? $$out{filespath} : $$out{toppath};
        if ( !-d $createall ) {
            File::Path::make_path($createall)
              or abort "Could not create output directory $createall.";
        }
    }
}

my %opt;
my @optdescr = ( 'exec|e=s', 'output|o=s', 'list|l!', 'help|h|?!', 'debug|d!' );
my %config;

my $status = GetOptions( \%opt, @optdescr );

if ( !$status || $opt{help} ) {
    print <<EOF;
Usage: mhtml-tool [options] <MHT file>

By default, mhtml-tool unpacks an MHT archive (an archive type saved by some
browsers) to the current directory.  The first HTML file in the archive is
taken for the primary web page, and all other contained files are written to a
directory named after that HTML file.

Options:
-e, --exec    Execute the given program on a temporarily-unpacked archive
-l, --list    List archive contents (file name, MIME type, size and URL)
-o, --output  Unpack to directory <dir/> or to file <name>.html

Use the command "pod2man mhtml-tool > mhtml-tool.1" or
"pod2html mhtml-tool > mhtml-tool.html" to extract the manual.
EOF
    exit !$status;
}

my $origin  = getcwd;
my $tempdir = "";
if ( $opt{exec} and not defined $opt{output} ) {
    $tempdir = tempdir( CLEANUP => 1 );
    $opt{output} = $tempdir . "/";
}

my $orig_sep  = $/;
my $crlf_file = 0;

# Print a message to stdout (if debug is enabled).
sub debug_msg {
    if ( $opt{debug} ) {
        print ":debug: $_[0]\n";
    }
}

# Read next line. Remove line endings (both unix & windows style).
sub read_next_line {
    my $cur_line = <>;
    chomp $cur_line;
    if ( $cur_line =~ /\r$/ ) {
        debug_msg("It's a CRLF file") if ( $crlf_file == 0 );
        $crlf_file++;
    }
    $cur_line =~ s/\r//;
    return $cur_line;
}

# Parse headers.
# Handle multi-line headers (lines starting with spaces are part of a
# multi-line header).
# Read data from <> until the first empty line.
# Set $crlf_file to 1 if it's a CRLF (windoze) file.
sub parse_headers {
    my %headers;
    my $sep = $/;
    $/ = $orig_sep;
    my $cur_line  = read_next_line;
    my $full_line = $cur_line;

    while ( not( $cur_line =~ /^$/ ) ) {
        if ( $cur_line =~ /;$/ ) {    # if a continued line...
            while ( ( $cur_line = read_next_line ) =~ /^\h+/ ) {
                $cur_line =~ s/^\h+//;
                $full_line = $full_line . " " . $cur_line;
            }
            if ( $full_line =~ s/^([-\w]+): (.*)$// ) {
                $headers{$1} = $2;
                debug_msg("(1) New header $1 with value $2");
            }
        }
        else {
            if ( $full_line =~ s/^([-\w]+): (.*)$// ) {
                $headers{$1} = $2;
                debug_msg("(2) New header $1 with value $2");
            }
        }
        $cur_line  = read_next_line;
        $full_line = $cur_line;
    }
    $/ = $sep;
    return %headers;
}

my %global_headers = parse_headers();

abort "Can't find Content-Type header - not a MIME HTML file?"
  if ( !defined( $global_headers{'Content-Type'} ) );
debug_msg("Global Content-Type: $global_headers{'Content-Type'}");

my $boundary = '';
my $endcode  = '';

# FIXME: - add other possible mime types
# Types implemented so far are:
#   Content-Type: multipart/related; boundary="----=_NextPart_01D8BCC9.47C2B260"
#   Content-Type: text/html; charset="utf-8"
if ( $global_headers{'Content-Type'} =~
    m!multipart/related;.*\h+boundary="?([^"]*)"?$! )
{
    $endcode = $boundary = $1;
    $endcode =~ s/\s+$//;
    if ($crlf_file) {
        $/ = "\r\n--$boundary\r\n";
    }
    else {
        $/ = "\n--$boundary\n";
    }
    debug_msg("Boundary: $boundary");
}
elsif ( $global_headers{'Content-Type'} =~ m!text/html! ) {
    $/ = '';
}
else {
    die "Error: Unknown Content-Type: $global_headers{'Content-Type'}\n";
}

my %by_url;
my @htmlfiles;
my $fh;

{
    my $fileind = 1;
    my $leading = "--$boundary\n";
    while ( defined( my $data = <> ) ) {
        chomp $data;
        $data =~ s/\R/\n/g;   # handle various other line-endings (windows, mac)
        while ( index( $data, $leading ) == 0 ) {
            $data = substr( $data, length($leading) );
        }
        my %headers;
        while ( $data =~ s/^([-\w]+): (.*)\n// ) {
            $headers{$1} = $2;
            debug_msg("New header $1 with value $2");

          # read (and ignore atm) lines starting with space (multi-line headers)
            while ( $data =~ s/^\h+.*\n// ) {
                debug_msg("empty line");
            }
        }
        if ( scalar(%headers) == 0 ) {
            %headers = %global_headers
              ;   # fallback to the global headers as needed, usually for "text"
        }
        $data =~ s/^\n//;
        $data =~ s/\n--$endcode--\r?\n$/\n/s;
        my ( $type, $origname );
        if ( defined( $headers{"Content-Type"} ) ) {
            ($type) = $headers{"Content-Type"} =~ /^(\w+\/\w+)\b/;
            debug_msg("type=$type");
        }
        else {
            print "Error: No Content-Type found, skipping chunk\n";

        # we could print the bad chunk here...
        # but it's probably just a "warning" about
        #   "if you see this your software isn't recognizing a web archive file"
            next;
        }
        if ( defined( $headers{"Content-Type"} )
            && $headers{"Content-Type"} =~ /\bname=([^;]*)/ )
        {
            $origname = $1;
            ($type) = $headers{"Content-Type"} =~ /^(\w+\/\w+)\b/;
            $type //= "";
        }
        elsif ( defined( $headers{"Content-Disposition"} )
            && $headers{"Content-Disposition"} =~ /\bfilename=([^;]*)/ )
        {
            $origname = $1;
            if ( !defined($type) ) {
                $type = $origname =~ /\.html?$/i ? "text/html" : "";
            }
        }
        elsif ( defined( $headers{"Content-Location"} ) ) {
            $origname = $headers{"Content-Location"};

            # for unknown reasons, files generated by IE11 may contain some
            # local paths with '\' instead of '/'
            if ( $origname =~ m!^file://.*\\! ) {
                debug_msg("Windows-style path detected: $origname");
                $origname =~ s!^.*\\!!;
            }
            $origname =~ s!^.*/!!;
            if ( !defined($type) ) {
                $type = "";
            }
        }

        $origname = "noname" unless defined $origname;
        $origname = "noname" if ( $origname eq "" );
        my $fname = unique_name( $origname, $ARGV[0] );
        if ( !defined( $headers{"Content-Transfer-Encoding"} ) ) {
            print STDERR "Info: Encoding of ", ordinal($fileind),
              " file not found - leaving as-is.\n";
        }
        elsif ( $headers{"Content-Transfer-Encoding"} =~ /\bbase64\b/i ) {
            $data = MIME::Base64::decode($data);
        }
        elsif (
            $headers{"Content-Transfer-Encoding"} =~ /\bquoted-printable\b/i )
        {
            $data = MIME::QuotedPrint::decode($data);
        }
        debug_msg("origname=$origname; fname=$fname; type=$type");
        debug_msg( "Content-Type: " . $headers{"Content-Type"} );
        debug_msg( "Data size: " . length($data) );
        if ( $opt{list} ) {
            $origname =~ s/\s+$// if defined $origname;
            my $size = length($data);
            print $fname // "(?)", "\t", $type || "(?)", "\t$size\t",
              $headers{"Content-Location"} // "(?)", "\n";
            next;
        }
        $headers{fname} = $fname;
        if ( $headers{"Content-Location"} ) {
            $headers{url} = $headers{"Content-Location"};
            $headers{url} =~ s/\s+$//;
            $by_url{ $headers{url} } = \%headers;
            debug_msg("Content-Location: $headers{url}");
        }
        else {
            debug_msg("?? no location");
        }
        if ( $type eq "text/html" ) {
            $headers{data} = $data;
            if ( scalar @htmlfiles == 0 ) {   # first html file must have a name
                if ( !$headers{fname} ) {
                    $headers{fname} = "index.html";
                }
            }
            push @htmlfiles, \%headers;
            debug_msg("New html file in list: $headers{fname}");
        }
        else {
            mkfiledir( $opt{output}, $htmlfiles[0]->{fname}, 1, \%config );
            $fname = shorten_path "$config{filespath}/$fname";
            open $fh, ">$fname" or abort "Could not create file $fname.";
            print $fh $data;
            close $fh;
        }
    }
    continue {
        debug_msg("");
        ++$fileind;
    }
}

if ( $opt{list} ) {
    exit(0);
}

mkfiledir( $opt{output}, $htmlfiles[0]->{fname}, 0, \%config );

my $entrypoint  = ".";
my $filesprefix = $config{filesdir} . "/";
my $outname     = $config{firstout};
print "primary html output name: $outname\n";

for my $html (@htmlfiles) {
    my $linksubst = "";
    my $p         = HTML::PullParser->new(
        doc     => \$html->{data},
        "start" => 'text, attr, tagname',
        "text"  => 'text',
        "end"   => 'text'
    );
    while ( defined( my $tok = $p->get_token() ) ) {
        my $linkary;
        my @linkattrs;
        if (   ref( $tok->[1] )
            && ( $linkary   = $HTML::Tagset::linkElements{ $tok->[2] } )
            && ( @linkattrs = grep $tok->[1]->{$_}, @$linkary ) )
        {
            for my $attr (@linkattrs) {
                my $uri = URI->new( $tok->[1]->{$attr} );
                next unless defined $uri;
                next unless defined $html->{url};
                $uri = $uri->abs( $html->{url} );
                $tok->[1]->{$attr} =
                  "$filesprefix" . $by_url{ $uri->as_string() }->{fname}
                  if $by_url{ $uri->as_string() };
            }
            delete $tok->[1]->{"/"};
            $linksubst .=
              "<$tok->[2] "
              . join( " ",
                map( "$_=\"$tok->[1]->{$_}\"", keys %{ $tok->[1] } ) )
              . ">";
        }
        else {
            $linksubst .= $tok->[0];
        }
    }
    $outname = "$config{filespath}/$html->{fname}" unless defined $outname;
    $outname = shorten_path $outname;
    open $fh, ">$outname" or abort "Could not create file $outname.";
    print $fh $linksubst;
    close $fh;
    $entrypoint = $outname if ( $entrypoint eq "." );

    # for all except the first HTML file:
    $filesprefix = "";
    $outname     = undef;
}

if ( $tempdir ne "" ) {
    chdir $tempdir;
    my $enforce = "";
    $enforce = "-force-html"
      if (  $entrypoint ne "."
        and $entrypoint !~ /\.htm(l)?$/
        and $opt{exec}  =~ /(lynx|((e)?links(2)?))/ );
    system( sprintf( "%s %s %s", $opt{exec}, $enforce, quoted $entrypoint ) );
    chdir $origin;
}

1;
