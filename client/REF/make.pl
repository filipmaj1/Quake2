#!/usr/bin/perl

#--------------------- Defines -------------------------------

if (1) {
	#...... work .......
	$infile = "ref.in";
	$outhdr = "../ref_decl.h";
	$outcode = "../ref_impl.h";
	$outdef = "../ref_defs.h";
} else {
	#...... test .......
	$infile = "ref.in.test";
	$outhdr = "decl.h";
	$outcode = "impl.h";
	$outdef = "defs.h";
}

$strucname = "ri";
$typename = "refImport_t";


#--------------------- Functions -----------------------------


sub getline {
	while ($line = <IN>)
	{
		# remove CR/LF
		$line =~ s/\r//;
		$line =~ s/\n//;
		# remove comments
		$line =~ s/\s*\/\/.*//;
		# ignore empty lines
		next if $line eq "";
		return 1;
	}
	return 0;
}


#------------------- Parsing input file ----------------------


sub EmitStruc {
	printf (HDR "\t%s\t(*_%s) (%s);\n", $type, $func, $args);
}

sub EmitDefine1 {
	printf (DEFS "#define %s\t$strucname._%s\n", $func, $func);
}

sub EmitDefine2 {
	printf (DEFS "//#define %s\t%s_\n", $func, $func);	#??
}

sub EmitDecl {
	printf (DEFS "%s\t%s (%s);\n", $type, $func, $args);
}

sub EmitImpl {
	print (CODE ",") if $num > 0;
	printf (CODE "\n\t%s", $func);
}

# Usage: Parse (<per-header func>)
sub Parse {
	my $sub;
	my $subBegin = @_[0];

	open (IN, $infile) || die "Input file ", $infile, " not found";
	$num = 0;
	while (getline())
	{
		($type, $func, $args, $end) = $line =~ / \s* (\S+ .* [\*\s]) (\w+) \s+ \( (.*) \) \s* (\S*) /x;
		($type) = $type =~ /\s*(\S.*\S)\s*/;	# truncate leading and trailing spaces

		&$subBegin () if defined $subBegin;
		$num++;
	}
	close (IN);
}


#----------------------- Main program --------------------------------

# open files
open (HDR, ">$outhdr") || die "Cannot create file ", $outhdr;
open (CODE, ">$outcode") || die "Cannot create file ", $outcode;
open (DEFS, ">$outdef") || die "Cannot create file ", $outdef;

Parse ();
$numFuncs = $num;
printf ("Found %d functions ...\n", $numFuncs);


#------------------- Creating header part ----------------------------

print (HDR "typedef struct {\n");

print (HDR "\tint\t\tstruc_size;\n");
print (HDR "\tint\t\tapi_version;\n");

Parse ("EmitStruc");
print (HDR "} $typename;\n\n");
print (HDR "extern $typename $strucname;\n\n");

#------------------- Creating defines --------------------------------

print (DEFS "#ifdef DYNAMIC_REF\n\n");
Parse ("EmitDefine1");
print (DEFS "\n#else\n\n");
#Parse ("EmitDefine2");
#print (DEFS "\n\n");
Parse ("EmitDecl");
print (DEFS "\n#endif\n\n");


#------------------- Creating code part ------------------------------

print (CODE "$typename $strucname = {\n");
print (CODE "\tsizeof($typename),\n");
print (CODE "\tAPI_VERSION,");
Parse ("EmitImpl");
print (CODE "\n};\n\n");

# close files
close (HDR);
close (CODE);
close (DEFS);
