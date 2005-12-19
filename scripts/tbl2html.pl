#!/usr/bin/perl -w
#
# Translate one or more ".tbl" files into ".html" files which can be used to
# test the charset support in lynx.  Each of the ".html" files will use the
# charset that corresponds to the input ".tbl" file.

use strict;

use Getopt::Std;
use File::Basename;
use POSIX qw(strtod);

sub field($$) {
	my $value = $_[0];
	my $count = $_[1];

	while ( $count > 0 ) {
		$count -= 1;
		$value =~ s/^\S*\s*//;
	}
	$value =~ s/\s.*//;
	return $value;
}

sub notes($) {
	my $value = $_[0];

	$value =~ s/^[^#]*#//;
	$value =~ s/^\s+//;

	return $value;
}

sub make_header($$$) {
	my $source   = $_[0];
	my $charset  = $_[1];
	my $official = $_[2];

	printf FP "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">\n";
	printf FP "<HTML>\n";
	printf FP "<HEAD>\n";
	printf FP "<!-- $source -->\n";
	printf FP "<TITLE>%s table</TITLE>\n", $official;
	printf FP "<META HTTP-EQUIV=\"Content-Type\" CONTENT=\"text/html; charset=%s\">\n", $charset;
	printf FP "</HEAD>\n";
	printf FP "\n";
	printf FP "<BODY> \n";
	printf FP "\n";
	printf FP "<H1 ALIGN=center>%s table</H1> \n", $charset;
	printf FP "\n";
	printf FP "<PRE>\n";
	printf FP "Code  Char  Entity  Render  Description\n";
}

sub make_mark() {
	printf FP "----  ----  ------  ------  -----------------------------------\n";
}

sub make_row($$$) {
	my $old_code = $_[0];
	my $new_code = $_[1];
	my $comments = $_[2];

	my $visible = sprintf("&amp;#%d;      ", $new_code);
	printf FP " %02x    %c    %.13s &#%d;     %s\n",
		$old_code, $old_code,
		$visible, $new_code,
		$comments;
}

sub null_row($$) {
	my $old_code = $_[0];
	my $comments = $_[1];

	printf FP " %02x    %c                     %s\n",
		$old_code, $old_code,
		$comments;
}

sub make_footer() {
	printf FP "</PRE>\n";
	printf FP "</BODY>\n";
	printf FP "</HTML>\n";
}

sub doit($) {
	my $source = $_[0];

	printf "** %s\n", $source;

	my $target = basename($source, ".tbl");

	# Read the file into an array in memory.
	open(FP,$source) || do {
		print STDERR "Can't open input $source: $!\n";
		return;
	};
	my (@input) = <FP>;
	close(FP);

	my $n;
	my $charset = "";
	my $official = "";
	my $empty = 1;

	for $n (0..$#input) {
		$input[$n] =~ s/\s*$//; # trim trailing blanks
		$input[$n] =~ s/^\s*//; # trim leading blanks
		$input[$n] =~ s/^#0x/0x/; # uncomment redundant stuff
		next if $input[$n] =~ /^$/;
		next if $input[$n] =~ /^#.*$/;
		if ( $input[$n] =~ /^M.*/ ) {
			$charset = $input[$n];
			$charset =~ s/^.//;
		} elsif ( $input[$n] =~ /^O.*/ ) {
			$official = $input[$n];
			$official =~ s/^.//;
		} elsif ( $input[$n] =~ /^\d/ ) {
			if ( $empty ) {
				$target = $charset . ".html";
				printf "=> %s\n", $target;
				open(FP,">$target") || do {
					print STDERR "Can't open output $target: $!\n";
					return;
				};

				make_header($source, $charset, $official);
				$empty = 0;
			}

			my $newcode = field($input[$n], 1);

			next if ( $newcode eq "idem" );
			next if ( $newcode eq "" );

			my $oldcode = field($input[$n], 0);
			if ( $oldcode =~ /.*-.*/ ) {
				printf "FIXME range %s\n", $oldcode;
				next;
			}
			my ($old_code, $old_oops) = strtod($oldcode);
			if ( $old_oops ne 0 ) {
				printf "FIXME number %s\n", $oldcode;
				next;
			}
			make_mark if (( $old_code % 8 ) == 0 );

			my $comment = notes($input[$n]);

			$newcode =~ s/^U\+/0x/;
			if ( $newcode =~ /^#.*/ ) {
				null_row($old_code, $comment);
				next;
			}
			my ($new_code, $new_oops) = strtod($newcode);
			if ( $new_oops ne 0 ) {
				printf "FIXME number %s\n", $newcode;
				next;
			}
			make_row($old_code, $new_code, $comment);
		} else {
			next;
		}
	}
	if ( ! $empty ) {
		make_footer();
	}
	close FP;
}

sub usage() {
	# FIXME
	exit(1);
}

if ( $#ARGV < 0 ) {
	usage();
} else {
	while ( $#ARGV >= 0 ) {
		&doit ( shift @ARGV );
	}
}
exit (0);
