#! @PERL@
# Simple script to convert #asm ... #endasm inline assembly to gcc
# inline assembly

$file = $ARGV[0];
$inasm = 0;
$line = 1;

open(INFILE, "<$file") || die "Unable to open \"$file\"\n";

print "#line $line \"$file\"\n";
$line ++;

while (<INFILE>) {
  if (/#asm/) {
    $inasm = 1;
  } elsif (/#endasm/) {
    $inasm = 0;
    print "#line $line \"$file\"\n";
  } elsif (/#\s*include/) {
    print $_;
    print "#line $line \"$file\"\n";
  } elsif (/#/) {
    print $_;
    print "#line $line \"$file\"\n";
  } elsif ($inasm) {
    s/^\s+//;
    s/\s+$//;
    s/[	]+/	/;
    s/SREG/__SREG__/g;
    s/low|LOW/lo8/g;
    s/high|HIGH/hi8/g;
    print "\t__asm__ __volatile__(\"$_\");\n";
  } else {
    print $_;
  }
  $line ++;
}

close(INFILE);

