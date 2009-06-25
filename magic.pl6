# Magic/wizard-mode related routines

# Because $wkday, $wkend, and $holid have to contain at least 24 bits, we can't
# safely use the native `int' type for them.
my Int $wkday;
my Int $wkend;
my Int $holid;

my int $hbegin;
my int $hend;
my Str $hname;
my int $short;
my Str $magic;
my int $magnm;  # magic number
my int $latency;
my Str $msg;  # MOTD, initially null
my $saved;
my $savet;
my $setup;

sub mspeak(int $msg) { speak @magicMsg[$msg] if $msg != 0 }

sub yesm(int $x, int $y, int $z --> Bool) {
 loop {
  mspeak $x if $x != 0;
  print "\n> ";
  my Str $reply = $*IN.get;
  if $reply ~~ m:i/^^\h*y/ {
   mspeak $y if $y != 0;
   return True;
  } elsif $reply ~~ m:i/^^\h*n/ {
   mspeak $z if $z != 0;
   return False;
  } else { say "Please answer the question." }
 }
}

sub start( --> Bool) {
 my($d, $t) = datime;
 my $primetm = $wkday;
 $primetm = $wkend if $d % 7 <= 1;
 $primetm = $holid if $hbegin <= $d <= $hend;
 my bool $ptime = ($primetm +& 1 +< ($t/60)) != 0;
 my bool $soon = False;
 if $setup < 0 {
  $delay = ($d - $saved) * 1440 + ($t - $savet);
  if $delay < $latency {
   say "This adventure was suspended a mere $delay minutes ago.";
   $soon = True;
   if $delay < $latency/3 {mspeak 2; return False; }
  }
 }
 if $soon {
  mspeak 8;
  if wizard {$saved = -1; return False; }
  mspeak 9;
  return False;
 }
 if $ptime {
  mspeak 3;
  hours;
  mspeak 4;
  if wizard {$saved = -1; return False; }
  if $setup < 0 {mspeak 9; return False; }
  if yesm(5, 7, 7) {$saved = -1; return True; }
  return False;
 }
 $saved = -1;
 return False;
}

sub maint() {
 return if !wizard;
 $blklin = False;
 hours if yesm(10, 0, 0);
 newhrs if yesm(11, 0, 0);
 if yesm(26, 0, 0) {
  mspeak 27;
  print "\n> ";
  $hbegin = $*IN.get;
  mspeak 28;
  print "\n> ";
  $hend = $*IN.get;
  my($d, $t) = datime;
  $hbegin += $d;
  $hend += $hbegin - 1;
  mspeak 29;
  print "\n> ";
  $hname = $*IN.get;
 }
 say "Length of short game (null to leave at $short):";
 print "\n> ";
 my $x = $*IN.get;
 $short = $x if $x > 0;
 mspeak 12;
 print "\n> ";
 $x = $*IN.get.words.[0];
 $magic = $x.substr(0, 5) if $x.defined;
 mspeak 13;
 print "\n> ";
 $x = $*IN.get;
 $magnm = $x if $x > 0;
 say "Latency for restart (null to leave at $latency):";
 print "\n> ";
 $x = $*IN.get;
 mspeak 30 if 0 < $x < 45;
 $latency = 45 max $x if $x > 0;
 motd(True) if yesm(14, 0, 0);
 $saved = 0;
 $setup = 2;
 @abb[1] = 0;
 mspeak 15;
 $blklin = True;
 ciao;
}

sub a5toi(Str $word --> int64) {
 my int64 $i = 0;
 my @chrs = $word.comb.map: *.ord;  # $word must be in ASCII, or else...
 $i += int64(@chrs[$_] // 0) +< (36 - 7 * ($_+1)) for ^5;
 return $i;
}

sub itoa5(int64 $i --> Str) {
 my Str $word = '';
 $word ~= chr(($i +> (36 - 7 * ($_+1))) +& 127) for ^5;
 return $word;
}

sub wizard( --> Bool) {
 return False if !yesm(16, 0, 7);
 mspeak 17;
 print "\n> ";
 my $word = $*IN.get.words.[0].substr(0, 5);
 if $word !eq $magic {mspeak 20; return False; }
 my($d, $t) = datime;
 $t = $t * 2 + 1;
 my int64 $iword = a5toi '@@@@@';
 my int @val[5];
 for 1..5 -> $y {
  my $x = 79 + $d % 5;
  $d /= 5;
  $t = ($t * 1027) % 1048576 for 1..$x;
  @val[$y-1] = ($t*26) / 1048576 + 1;
  $iword += int64(@val[$y-1]) +< (36 - 7*$y);
 }
 if yesm(18, 0, 0) {mspeak 20; return False; }
 say itoa5($iword);
 print "\n> ";
 $iword = a5toi $*IN.get.words.[0];
 ($d, $t) = datime;
 $t = ($t/60)*40 + ($t/10)*10;
 $d = $magnm;
 for 1..5 -> $y {
  my $z = $y % 5 + 1;
  my $x = ((@val[$y-1] - @val[$z-1]).abs * ($d % 10) + ($t % 10)) % 26 + 1;
  $t /= 10;
  $d /= 10;
  $iword -= int64($x) +< (36 - 7*$y);
 }
 if itoa5($iword) eq '@@@@@' {mspeak 19; return True; }
 else {mspeak 20; return False; }
}

sub hours() {
 print "\n";
 hoursx($wkday, "Mon - Fri:");
 hoursx($wkend, "Sat - Sun:");
 hoursx($holid, "Holidays: ");
 my($d, $t) = datime;
 return if $hend < $d | $hbegin;
 if $hbegin > $d {
  $d = $hbegin - $d;
  say "The next holiday will be in $d day", $d == 1 ?? '' !! 's',
   ", namely $hname.";
 } else { say "Today is a holiday, namely $hname." }
}

sub hoursx($h, Str $day) {
 my Bool $first = True;
 my $from = -1;
 if $h == 0 { say ' ' x 10, "$day Open all day" }
 else {
  loop {
   do { $from++ } while $h +& 1 +< $from;
   if $from >= 24 {
    say ' ' x 10, $day, ' Closed all day' if $first;
    return;
   } else {
    my $till = $from;
    do { $till++ } while ($h +& 1 +< $till) == 0 && $till != 24;
    if $first {
     print ' ' x 10, $day;
     printf "%4d:00 to%3d:00\n", $from, $till;
    } else {
     printf ' ' x 20 ~ "%4d:00 to%3d:00\n", $from, $till
    }
    $first = False;
    $from = $till;
   }
  }
 }
}

sub newhrs() {
 mspeak 21;
 $wkday = newhrx('Weekdays:');
 $wkend = newhrx('Weekends:');
 $holid = newhrx('Holidays:');
 mspeak 22;
 hours;
}

sub newhrx(Str $day --> Int) {
 my Int $newhrx = 0;
 say "Prime time on $day";
 loop {
  print "from: ";
  my int $from = $*IN.get.words.[0];
  return $newhrx if $from < 0 || $from >= 24;
  print "till: ";
  my int $till = $*IN.get.words.[0] - 1;
  return $newhrx if $till < $from || $till >= 24;
  $newhrx +|= 1 +< $_ for $from..$till;
 }
}

sub motd(Bool $alter) {
 if $alter {
  $msg = '';
  mspeak 23;
  loop {
   print "> ";
   my Str $next = $*IN.get;
   return if !$next;
   if $next.chars > 70 {mspeak 24; next; }
   $msg ~= $next ~ "\n";
   # This doesn't exactly match the logic used in the original Fortran, but
   # it's close:
   if $msg.chars + 70 >= 500 {mspeak 25; return; }
  }
 } else { print $msg if $msg }
}

sub poof() {
 $wkday = 0o00777400;
 $wkend = 0;
 $holid = 0;
 $hbegin = 0;
 $hend = -1;
 $short = 30;
 $magic = 'DWARF';
 $magnm = 11111;
 $latency = 90;
}

sub datime( --> List of Int) {
 # Return:
 # - number of days since 1 Jan 1977 (220924800 in Unix epoch time)
 # - minutes past midnight

 #<
 my Temporal::DateTime $now = Time::gmtime;
 return
  ... ,
  $now.hour * 60 + $now.minute;
 >#

 !!!
}
