#!/usr/bin/perl
#
# See POD docs at end of this file for genereal info

use strict;
use warnings;
use File::Basename;
use Getopt::Long;


use constant PROG_DESCRIPTION => "asteroid orbital elements converter";
use constant PROG_COPYRIGHT   => "(C) 2007; released under the GNU GPL version 2";
use constant PROG_VERSION     => "1.0";
use constant PROG_AUTHOR      => "Matthew Gates, updated 2010, 2011 by Digitalis Education Solutions, Inc.";

my $this_script		= basename($0);

# flags & param vars
my $gs_flg_all		= 0;
my $gs_flg_data_url	= "http://hamilton.dm.unipi.it/~astdys/catalogs/allnum.cat";
my $gs_flg_name_url	= "http://www.minorplanetcenter.net/iau/lists/MPNames.html";
my $gs_flg_id_list	= "1-10";
my $gs_flg_data_age	= 3600 * 24 * 30; # only fetch data if current files older than 1 month
my $gs_flg_ini      = 0; # default to sts format

# process command line options.  Praise Bob, I love Getopt::Long!
GetOptions(
		'all'		=> \$gs_flg_all,
		'data-url=s'	=> \$gs_flg_data_url,
		'force-refresh'	=> sub { $gs_flg_data_age = 0; },
		'help'		=> sub { usage(0,1) },
		'ids=s'		=> \$gs_flg_id_list,
		'name-url=s'	=> \$gs_flg_name_url,
		'version'	=> sub { version_message(); exit 0; },
	    'ini'		=> \$gs_flg_ini,
	   )     or usage(1,0);

# some globals.  yeah yeah, so sue me.
my @ga_field_order = (qw(Epoch SemiMajorAxis Eccentricity Inclination AscendingNode ArgOfPericenter MeanAnomaly mag albedo));
my %gh_names;		# key is asteroid ID
my %gh_data;		# key is asteroid ID
my %gh_selection;	# list of IDs which we want
my $gs_selection_max = 0;
my $gs_data_filename = "./asteroid_orbits.dat";
my $gs_name_filename = "./asteroid_names.dat";
my $gs_unknown_name_counter = 0;

if ( ! $gs_flg_all ) {
	foreach my $i (split(",", $gs_flg_id_list)) {
		# handle ranges if necessary
		my ($l, $u); # lower and upper range values
		if ( $i =~ /^\d+$/ ) { $l = $i; $u = $i; } 
		elsif ( $i =~ /^(\d+)-(\d+)$/ ) { $l = $1; $u = $2; }
		else { next; }
		
		for(my $n=$l; $n<=$u; $n++) {
			$gh_selection{$n} = 1;
			if ( $n > $gs_selection_max ) { $gs_selection_max = $n; }
		}
	}
}

download_data_files();
load_data();
load_names();

if($gs_flg_ini) {
	dump_data_ini();
} else {
	dump_data_sts();
}

###############################################################################
# Subroutines
###############################################################################
sub usage {
	my $level = shift || 0;
	my $verbose = shift || 0;

	if ( $level !~ /^\d+$/ ) { die "argument must be integer"; }

	my $cmd;
	if ( $verbose ) { $cmd = "pod2usage -verbose 1 $this_script"; }
	else { $cmd = "pod2usage $this_script"; }

	open(CMD, "$cmd|") || die "error executing $cmd: $!";
	while(<CMD>) { print; }
	close(CMD);
	exit($level);
}

sub version_message {
	if ( ! defined(&PROG_DESCRIPTION) ) {
		warn "You should define the constant PROG_DESCRIPTION in your program\n";
	} else {
		print &PROG_DESCRIPTION . " ";
	}

	if ( ! defined(&PROG_VERSION) ) {
		warn "You should define the constant PROG_VERSION in your program\n";
	} else {
		print &PROG_VERSION . "\n";
	}

	if ( ! defined(&PROG_COPYRIGHT) ) {
		warn "You should define the constant PROG_COPYRIGHT in your program\n";
	} else {
		print &PROG_COPYRIGHT . "\n";
	}

	if ( defined(&PROG_AUTHOR) ) {
		print "Written by " . &PROG_AUTHOR . "\n";
	}
}

sub download_data_files {
	my $download = 0;
	my @statinfo = stat($gs_data_filename);
	if ( ! defined($statinfo[9]) ) { $download = 1; }
	elsif ( time - $statinfo[9] > $gs_flg_data_age ) { $download = 1; }
	if ( $download ) {
		system("curl '$gs_flg_data_url' > '$gs_data_filename'") && die "cannot download $gs_flg_data_url";
	}
	else {
		print STDERR "No need to download $gs_data_filename - we have a recent copy locally\n";
	}

	$download = 0;
	@statinfo = stat($gs_name_filename);
	if ( ! defined($statinfo[9]) ) { $download = 1; }
	elsif ( time - $statinfo[9] > $gs_flg_data_age ) { $download = 1; }
	if ( $download ) {
		system("curl '$gs_flg_name_url' > '$gs_name_filename'") && die "cannot download $gs_flg_name_url : $!";
	}
	else {
		print STDERR "No need to download $gs_name_filename - we have a recent copy locally\n";
	}

}

sub load_data {
	open(DATA, "<$gs_data_filename") || die "cannot open data file for reading : $!";
	my $header = 1;
	while(<DATA>) {
		if (/END_OF_HEADER/) {
			$header = 0;
			next;
		}
		if ($header) { next };

		# skip comments
		if (/^\s*\!/) { next };

		chomp;
		my($id, @data) = unpack("A10 A17 A25 A25 A25 A25 A25 A25 A6 A6", $_);
		$id =~ s/'//g;
		if ( $id > $gs_selection_max && ! $gs_flg_all ) { last; }

		if ( ! defined($gh_selection{$id}) && ! $gs_flg_all ) { next; }

		my %datahash;
		for(my $i=0; $i<=$#ga_field_order; $i++) {
			$data[$i] =~ s/^\s+//;
			$data[$i] =~ s/\s+$//;

			if($i==0) { $data[0] += 2400000; } # correct for epoch shorthand

			$datahash{$ga_field_order[$i]} = $data[$i];
		}
		$gh_data{$id} = \%datahash;
	}
	close(DATA);
}

sub load_names {
	open(NAMES, "<$gs_name_filename") || die "cannot open name file for reading : $!";
	my $in_pre = 0;
	while(<NAMES>) {
		if ( /<pre>/i ) {
			$in_pre = 1;
			next;
		}

		if ( ! $in_pre ) { next; }

		if ( /<\/pre>/ ) { last; }

		chomp;

		s/<a name=[^>]+>//i;
		s/<\/a>//;

		my(undef, $id, $name) = split(/[\(\)]/, $_);

		if ( defined($gh_data{$id}) ) {
			$name =~ s/^\s+//;
			$name =~ s/\s+$//;
			$gh_names{$id} = $name;
		}
	}
}

sub dump_data_sts {
	foreach my $id (sort {$a <=> $b} keys %gh_data) {
		my $name;
		if (defined($gh_names{$id})) {
			$name = $gh_names{$id};
		}
		else {
			$name = "unknown_name_ID_$id";
			$gs_unknown_name_counter++;
		}
		printf "body action load ", lc($name);
		print "name \"$name\" ";
		print "parent Sun ";
		print "oblateness 0.0 ";
		print "color 1.0,1.0,1.0 ";
		print "coord_func comet_orbit ";
		print "halo false ";
		print "lighting false ";
		print "radius 25 ";
		my $sp = (($gh_data{$id}{"SemiMajorAxis"} ** 3) ** 0.5) * 365.256363051;
# Sometimes Don't want orbits by default - too much!
		print "orbit_visualization_period $sp ";
		foreach my $field (reverse sort @ga_field_order) {
			my $prefix = "";
			if ($field eq "mag") { next; }
			elsif ($field ne "albedo") { $prefix = "orbit_"; }

			my $val = sprintf("%.15f", $gh_data{$id}{$field});
			$val =~ s/0+$//;
			printf "%s%s %s ", $prefix, $field, $val; 
		}
		print "\n";
	}
}


sub dump_data_ini {
	foreach my $id (sort {$a <=> $b} keys %gh_data) {
		my $name;
		if (defined($gh_names{$id})) {
			$name = $gh_names{$id};
		}
		else {
			$name = "unknown_name_ID_$id";
			$gs_unknown_name_counter++;
		}
		printf "[ast-%s]\n", lc($name);
		print "name = $name\n";
		print "parent = Sun\n";
		print "oblateness = 0\n";
		print "color = 1,1,1\n";
		print "coord_func = comet_orbit\n";
		print "halo = false\n";
		print "lighting = false\n";
		print "radius = 25\n";
		my $sp = (($gh_data{$id}{"SemiMajorAxis"} ** 3) ** 0.5) * 365.256363051;
# Sometimes Don't want orbits by default - too much!
		print "orbit_visualization_period = $sp\n";
		foreach my $field (reverse sort @ga_field_order) {
			my $prefix = "";
			if ($field eq "mag") { next; }
			elsif ($field ne "albedo") { $prefix = "orbit_"; }

			my $val = sprintf("%.15f", $gh_data{$id}{$field});
			$val =~ s/0+$//;
			printf "%s%s = %s\n", $prefix, "\L$field", $val; 
		}
		print "\n";
	}
}


__END__

=head1 NAME 

generate-asteroid-sts.pl - generate asteroid orbital data for Stellarium360

=head1 SYNOPSIS

generate-asteroid-sts.pl [options]

=head1 DESCRIPTION

Downloads asteroid data and outputs sections intended for use in a Stellarium360 StratoScript.

=head1 OPTIONS

=over

=item B<--all>

Select all asteroids for output.  Note this is a lot and will 
take some time.

=item B<--data-url>=I<u>

Fetch orbital data from url I<u>.  The default is 
http://hamilton.dm.unipi.it/astdys/catalogs/allnum.cat

=item B<--force-refresh>

Fetch orbital and name data even if the files are less than a month old.

=item B<--help>

Print the command line syntax an option details.

=item B<--ids>=I<i>

Specify the list of asteroid IDs to output.  This is a comma separated list of numbers or ranges of numbers, e.g.

1,3-6,12

Would select asteroids with numbers 1, 3, 4, 5, 6, and 12.  The default is 1-10.

=item B<--ini>

Writes output in ssytem.ini file format.  Default is StratoScript format.

=item B<--name-url>=I<u>

Fetch name data from url I<u>.  The default is 
http://cfa-www.harvard.edu/iau/lists/MPNames.html

=item B<--version>

Print the program description and version.

=back

=head1 ENVIRONMENT

=over

=item STUFF_?_DBLEVEL

Sets debugging levels.  The ? can be D for database, M for module,
or S for script debugging messages.  Generally only S and D are
interesting for users, M is mostly just used during development.

=back

=head1 FILES

=over

=item filename

desc

=back

=head1 LICENSE

generate-asteroid-sts.pl is released under the GNU GPL (version 3 or later).  A
copy of the license should have been provided in the distribution of
the software in a file called "COPYING".  If you can't find this, then
try here: http://www.gnu.org/copyleft/gpl.html

=head1 AUTHOR

Matthew Gates <matthew@porpoisehead.net>

http://porpoisehead.net/

=head1 CHANGELOG

=over

=item Date:2007-05-29 Created, Author MNG

Original version.

=item Date:2007-11-06 Bug fixes, Author MNG

Download error status fixed.  Fixed --all option.  Fixed undefined name error.

=item Date:2010-08-16 Updated URLs, used STS output.  Author RMS


=back

=head1 BUGS

Please report bugs to the author.

=head1 SEE ALSO

stellarium360(1)
stellarium(1)

=cut

