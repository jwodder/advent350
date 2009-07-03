# Magic/wizard-mode related routines

# These arrays hold the times when adventurers are allowed into Colossal Cave;
# @wkday is for weekdays, @wkend for weekends, and @holid for holidays (days
# with special hours).  If element $n of an array is true, then the hour $n:00
# through $n:59 is considered "prime time," i.e., the cave is closed then.
my bool @wkday[24];
my bool @wkend[24];
my bool @holid[24];

my int $hbegin, $hend;  # start & end of next holiday
my Str $hname;  # name of next holiday
my int $short;  # turns allowed in a short/demonstration game
my Str $magic;  # magic word
my int $magnm;  # magic number
my int $latency;  # time required to wait after saving
my Str $msg;  # MOTD, initially null
my int $saved, $savet = -1, 0;  # date & time game was saved

sub mspeak(int $msg) { speak @magicMsg[$msg] if $msg != 0 }

sub yesm(int $x, int $y, int $z --> Bool) {
 loop {
  mspeak $x if $x != 0;
  my Str ($reply) = getin;
  if $reply eq 'YES' | 'Y' {
   mspeak $y if $y != 0;
   return True;
  } elsif $reply eq 'NO' | 'N' {
   mspeak $z if $z != 0;
   return False;
  } else { say "Please answer the question." }
 }
}

sub start( --> Bool) {
 my($d, $t) = datime;
 if $saved != -1 {
  my int $delay = ($d - $saved) * 1440 + ($t - $savet);
  if $delay < $latency {
   say "This adventure was suspended a mere $delay minutes ago.";
   if $delay < $latency/3 {mspeak 2; exit 0; }
   else {
    mspeak 8;
    if wizard() {$saved = -1; return False; }
    mspeak 9;
    exit 0;
   }
  }
 }
 if ($hbegin <= $d <= $hend ?? @holid !! $d % 7 <= 1 ?? @wkend !! @wkday)\
  [$t idiv 60] {
  # Prime time (cave closed)
  mspeak 3;
  hours;
  mspeak 4;
  if wizard() {$saved = -1; return False; }
  if $saved != -1 {mspeak 9; exit 0; }
  if yesm(5, 7, 7) {$saved = -1; return True; }
  exit 0;
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
  $hname = $*IN.get.substr(0, 20);
 }
 say "Length of short game (null to leave at $short):";
 print "\n> ";
 my int $x = $*IN.get;
 $short = $x if $x > 0;
 mspeak 12;
 $magic = (getin)[0] // $magic;
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
 mspeak 15;  # Say something else?
 $blklin = True;
 ciao;
}

sub wizard( --> Bool) {
 return False if !yesm(16, 0, 7);
 mspeak 17;
 print "\n> ";
 my Str $word = (getin)[0];
 if $word !eq $magic {mspeak 20; return False; }
 my($d, $t) = datime;
 $t = $t * 2 + 1;
 my int @wchrs[5] = 64, *;
 my int @val[5];
 for ^5 -> $y {
  my $x = 79 + $d % 5;
  $d idiv= 5;
  $t = ($t * 1027) % 1048576 for ^$x;
  @wchrs[$y] += @val[$y] = ($t*26) idiv 1048576 + 1;
 }
 if yesm(18, 0, 0) {mspeak 20; return False; }
 .print for ' ', @wchrs.map(*.chr), "\n";
 @wchrs = (getin)[0].comb.map: *.ord;
 # What happens if the inputted word is less than five characters?
 ($d, $t) = datime;
 $t = ($t idiv 60) * 40 + ($t idiv 10) * 10;
 $d = $magnm;
 for ^5 -> $y {
  @wchrs[$y] -= ((@val[$y] - @val[($y+1) % 5]).abs * ($d % 10) + ($t % 10))
   % 26 + 1;
  $t idiv= 10;
  $d idiv= 10;
 }
 if @wchrs.all == 64 {mspeak 19; return True; }
 else {mspeak 20; return False; }
}

sub hours() {
 print "\n";
 hoursx(@wkday, "Mon - Fri:");
 hoursx(@wkend, "Sat - Sun:");
 hoursx(@holid, "Holidays: ");
 my($d, $t) = datime;
 return if $hend < $d | $hbegin;
 if $hbegin > $d {
  $d = $hbegin - $d;
  say "The next holiday will be in $d day", $d == 1 ?? '' !! 's',
   ", namely $hname.";
 } else { say "Today is a holiday, namely $hname." }
}

sub hoursx(bool @hours[24], Str $day) {
 my bool $first = True;
 my int $from = -1;
 if @hours.all == False { say ' ' x 10, "$day Open all day" }
 else {
  loop {
   repeat { $from++ } while @hours[$from] && $from < 24;
   if $from >= 24 {
    say ' ' x 10, $day, ' Closed all day' if $first;
    return;
   } else {
    my int $till = $from;
    repeat { $till++ } until @hours[$till] || $till == 24;
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
 @wkday = newhrx('weekdays:');
 @wkend = newhrx('weekends:');
 @holid = newhrx('holidays:');
 mspeak 22;
 hours;
}

sub newhrx(Str $day --> bool[24] #< Right? > ) {
 my bool @newhrx[24] = False, *;
 say "Prime time on $day";
 loop {
  print "from: ";
  my int $from = $*IN.get.words.[0];
  return @newhrx if $from !~~ 0..^24;
  print "till: ";
  my int $till = $*IN.get.words.[0] - 1;
  return @newhrx if $till !~~ $from..^24;
  @newhrx[$from..$till] = True, *;
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
 @wkday = False xx 8, True xx 10, False xx 6;
 @wkend = False, *;
 @holid = False, *;
 $hbegin = 0;
 $hend = -1;
 $short = 30;
 $magic = 'DWARF';
 $magnm = 11111;
 $latency = 90;
}

sub datime( --> List of int) {
 # This function is supposed to return:
 # - the number of days since 1 Jan 1977 (220924800 in Unix epoch time)
 # - the number of minutes past midnight

 state Temporal::DateTime $start .= new(year => 1977, month => 1, day => 1);
 # The time defaults to midnight, right?

 my Temporal::DateTime $now = Time::gmtime;
 return ($now - $start) idiv 86400, $now.hour * 60 + $now.minute;
  # I assume the difference between two DateTime objects is the number of
  # seconds between them.
}
