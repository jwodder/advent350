#!/usr/bin/env perl6
use v6;

# Initialize database (lazily):

sub indexLines(Str *@lines --> List of Str) {
 gather for @lines {
  FIRST { take Mu }
  state Str $text = '';
  state int $i = 1;
# According to Larry Wall[1], these C<state> variables should be associated
# with the closure created by the C<for> loop and not with the C<indexLines>
# function; i.e., they are reinitialized for each invocation of C<indexLines>,
# not just the first time it's called.
# [1] <http://www.nntp.perl.org/group/perl.perl6.users/2009/09/msg1148.html>
  my($num, $t) = .split: "\t", 2;
  if $i == $num { $text ~= $t }
  else {
   take ($text ~~ /\>\$\</ ?? Mu !! $text), Mu xx $num - $i - 1;
   ($text, $i) = ($t, $num);
  }
  LAST { take $text }
 }
}

my Str @longDesc <== indexLines <== $=adventData01.lines(:!chomp);

my Str @shortDesc <== indexLines <== $=adventData02.lines(:!chomp);

my int @travel[*;*;*]
 <== map { .defined ?? .split("\n")».split("\t") !! Mu }
 <== indexLines <== $=adventData03.lines(:!chomp);

(my Array of int %vocab).push: $=adventData04.lines.map: { .split("\t").[1,0] }

my Array of Str @itemDesc = gather for $=adventData05.lines(:!chomp) {
 state int $i = 0;
 state int $j = -1;
 state Str @accum = ();
 my($n, $msg) = .split: "\t", 2;
 if 0 < $n < 100 {
  take @accum.map({ /\>\$\</ ?? Mu !! $_ }), [] xx $n - $i - 1;
  ($i, $j) = ($n, -1);
  @accum = $msg, ;
 } else {
  @accum.push: '' if $n / 100 != $j;
  @accum[*-1] ~= $msg;
  $j = $n / 100;
 }
 LAST { take @accum.map: { /\>\$\</ ?? Mu !! $_ } }
}

my Str @rmsg <== indexLines <== $=adventData06.lines(:!chomp);

# Section 7 (containing the initial locations of the items) is only read in
# when starting a new game (see MAIN below).

my int @actspk[32] <== indexLines <== $=adventData08.lines;

my int @cond = 0, *;
for $=adventData09.lines {
 my($bit, @locs) = .split: "\t";
 @cond[$_] +|= 1 +< $bit for @locs;
}

my Pair @classes
 <== map { [=>] .split("\t") }
 <== $=adventData10.lines(:!chomp);

my int @hints[*;4] <== map { .defined ?? .split("\t") !! Mu }
 <== indexLines <== $=adventData11.lines;

my Str @magicMsg <== indexLines <== $=adventData12.lines(:!chomp);


# Object & verb numbers:

enum obj « :KEYS(1) LAMP GRATE CAGE ROD ROD2 STEPS BIRD DOOR PILLOW SNAKE
 FISSUR TABLET CLAM OYSTER MAGZIN DWARF KNIFE FOOD BOTTLE WATER OIL MIRROR
 PLANT PLANT2 :AXE(28) :DRAGON(31) CHASM TROLL TROLL2 BEAR MESSAG VOLCANO VEND
 BATTER :NUGGET(50) :COINS(54) CHEST EGGS TRIDENT VASE EMERALD PYRAM PEARL RUG
 SPICES CHAIN »;

enum movement « :BACK(8) :NULL(21) :LOOK(57) :DEPRESSION(63) :ENTRANCE(64)
 :CAVE(67) »;

enum action « :TAKE(1) DROP SAY OPEN NOTHING LOCK ON OFF WAVE CALM WALK KILL
 POUR EAT DRINK RUB THROW QUIT FIND INVENT FEED FILL BLAST SCORE FOO BRIEF READ
 BREAK WAKE SUSPEND HOURS RESUME »;


# Global variables:

constant int MAXDIE, CHLOC, CHLOC2 = 3, 114, 140;
my int $goto = 0;
my bool $blklin = True;
my bool $gaveup = False;
my bool $demo = False;
my int $bonus = 0;
my int $verb, $obj;
my Str $in1, $in2, $word1, $word2;

# These arrays hold the times when adventurers are allowed into Colossal Cave;
# @wkday is for weekdays, @wkend for weekends, and @holid for holidays (days
# with special hours).  If element $n of an array is true, then the hour $n:00
# through $n:59 is considered "prime time," i.e., the cave is closed then.
my bool @wkday[24] = False xx 8, True xx 10, False xx 6;
my bool @wkend[24] = False, *;
my bool @holid[24] = False, *;

my int $hbegin, $hend = 0, -1;  # start & end of next holiday
my Str $hname;  # name of next holiday
my int $short = 30;  # turns allowed in a short/demonstration game
my Str $magic = 'DWARF';  # magic word
my int $magnm = 11111;  # magic number
my int $latency = 90;  # time required to wait after saving
my Str $msg;  # MOTD, initially null

constant Str $magicFile = "%*ENV<HOME>/.advmagic";
# file in which the current magic values are stored

# User's game data:
my int $loc, $newloc, $oldloc, $oldloc2, $limit;
my int $turns, $iwest, $knifeloc, $detail = 0, *;
my int $numdie, $holding, $foobar = 0, *;
my int $tally = 15;
my int $tally2 = 0;
my int $abbnum = 5;
my int $clock1 = 30;
my int $clock2 = 50;
my bool $wzdark, $closing, $lmwarn, $panic, $closed = False, *;
my int @prop[65] = 0 xx 50, -1 xx *;
my int @abb[141] = 0, *;
my int @hintlc[10] = 0, *;
my bool @hinted[10] = False, *;
my int @dloc[6] = 19, 27, 33, 44, 64, CHLOC;
my int @odloc[6];
my bool @dseen[6];
my int $dflag, $dkill = 0, *;
my int @place[65];
my int @fixed[65];
my int @atloc[141;*];
my int $saved, $savet = -1, 0;
# Although $saved and $savet are only used in the magic version of the game,
# they are declared in and saved & restored by both forms of the game in order
# to make the save files compatible.


# Functions:

sub toting(int $item --> Bool) { @place[$item] == -1 }
sub here(int $item --> Bool) { @place[$item] == $loc || toting $item }
sub at(int $item --> Bool) { $loc == @place[$item] | @fixed[$item] }
sub liq2(int $p --> int) { (WATER, 0, OIL)[$p] }
sub liq( --> int) { liq2(@prop[BOTTLE] max -1-@prop[BOTTLE]) }
sub liqloc(int $loc --> int) { liq2(@cond[$loc] +& 4 ?? @cond[$loc] +& 2 !! 1) }
sub bitset(int $loc, int $n --> Bool) { @cond[$loc] +& 1 +< $n }
sub forced(int $loc --> Bool) { @travel[$loc;0;1] == 1 }
sub dark( --> Bool) { !(@cond[$loc] +& 1 || (@prop[LAMP] && here(LAMP))) }
sub pct(int $x --> Bool) { (^100).pick < $x }

sub speak(Str $s) {
 return if !$s;
 print "\n" if $blklin;
 print $s;
}

sub pspeak(int $item, int $state) { speak @itemDesc[$item;$state+1] }
sub rspeak(int $msg) { speak @rmsg[$msg] if $msg != 0 }

sub getin( --> List of Str) {
 print "\n" if $blklin;
 loop {
  print "> ";
  my Str $raw1, $raw2 = $*IN.get.words;
  next if !$raw1.defined && $blklin;
  my Str $word1, $word2 = ($raw1, $raw2).map:
   { .defined ?? .substr(0, 5).uc !! Mu };
  return $word1, $raw1, $word2, $raw2;
 }
}

sub yes(int $x, int $y, int $z --> Bool) {
 loop {
  rspeak $x;
  my Str ($reply) = getin;
  if $reply eq 'YES' | 'Y' {
   rspeak $y;
   return True;
  } elsif $reply eq 'NO' | 'N' {
   rspeak $z;
   return False;
  } else { say "\nPlease answer the question." }
 }
}

sub carry(int $obj, int $where) {
 if $obj <= 100 {
  return if @place[$obj] == -1;
  @place[$obj] = -1;
  $holding++;
 }
 @atloc[$where].splice:
  @atloc[$where].keys.first({ @atloc[$where;$_] == $obj }), 1;
}

sub drop(int $obj, int $where) {
 if $obj > 100 {
  @fixed[$obj-100] = $where
 } else {
  $holding-- if @place[$obj] == -1;
  @place[$obj] = $where;
 }
 @atloc[$where].unshift: $obj if $where > 0;
}

sub move(int $obj, int $where) {
 my int $from = $obj > 100 ?? @fixed[$obj-100] !! @place[$obj];
 carry $obj, $from if 0 < $from <= 300;
 drop $obj, $where;
}

sub put(int $obj, int $where, int $pval --> int) {
 move $obj, $where;
 return -1 - $pval;
}

sub destroy(int $obj) { move $obj, 0 }

sub juggle(int $obj) {
 move $obj, @place[$obj];
 move $obj+100, @fixed[$obj];
}

sub bug(int $num) {
 say "Fatal error, see source code for interpretation.";
# Given the above message, I suppose I should list the possible bug numbers in
# the source somewhere, and right here is as good a place as any:
# 5 - Required vocabulary word not found
# 20 - Special travel number (500>L>300) is outside of defined range
# 22 - Vocabulary type (N/1000) not between 0 and 3
# 23 - Intransitive action verb not defined
# 24 - Transitive action verb not defined
# 26 - Location has no travel entries
 say "Probable cause: erroneous info in database.";
 say "Error code = $num\n";
 exit -1;
}

sub vocab(Str $word, int $type --> int) {
 my int @matches = %vocab{$word};
 if $type >= 0 { @matches.=grep(* div 1000 == $type) }
 if !@matches {
  if $type >= 0 { bug 5 }
  return -1;
 } else { return $type >= 0 ?? @matches[0] % 1000 !! [min] @matches }
 # When returning values of a specified type, there can be no more than one
 # match; if there is more than one, someone's been messing with the data
 # sections.
}

sub domove(int $motion) {
# 8:
 $goto = 2;
 $newloc = $loc;
 bug 26 if !@travel[$loc];
 given $motion {
  when NULL { return }
  when BACK {
   my int $k = forced($oldloc) ?? $oldloc2 !! $oldloc;
   ($oldloc2, $oldloc) = ($oldloc, $loc);
   if $k == $loc { rspeak 91 }
   else {
    my int $k2 = 0;
    for @travel[$loc].keys -> $kk {
     my int $ll = @travel[$loc;$kk;0] % 1000;
     if $ll == $k {
      dotrav(@travel[$loc;$kk;1]);
      return;
     } elsif $ll <= 300 {
      $k2 = $kk if forced($ll) && @travel[$ll;0;0] % 1000 == $k
     }
    }
    if $k2 != 0 { dotrav(@travel[$loc;$k2;1]) }
    else { rspeak 140 }
   }
  }
  when LOOK {
   rspeak 15 if $detail++ < 3;
   $wzdark = False;
   @abb[$loc] = 0;
  }
  when CAVE { rspeak($loc < 8 ?? 57 !! 58) }
  default {($oldloc2, $oldloc) = ($oldloc, $loc); dotrav($motion); }
 }
 # next bigLoop;
}

sub dotrav(int $motion) {
# 9:
 my int $rdest = -1;
 for @travel[$loc] -> $kk {
  if $kk[1..*].any == 1 | $motion ff * {
   my int $rcond = $kk[0] div 1000;
   my int $robj = $rcond % 100;
   given $rcond {
    when 0 | 100 { $rdest = $kk[0] % 1000 }
    when 0 ^..^ 100 { $rdest = $kk[0] % 1000 if pct $_ }
    when 100 ^.. 200 { $rdest = $kk[0] % 1000 if toting $robj }
    when 200 ^.. 300 { $rdest = $kk[0] % 1000 if toting($robj) || at($robj) }
    default { $rdest = $kk[0] % 1000 if @prop[$robj] != $_ div 100 - 3 }
   }
   last if $rdest != -1;
  }
 }
 given $rdest {
  when -1 {
   given $motion {
    when 29 | 30 | (43..50) { rspeak 9 }
    when  7 | 36 | 37 { rspeak 10 }
    when 11 | 19 { rspeak 11 }
    when 62 | 65 { rspeak 42 }
    when 17 { rspeak 80 }
    default { rspeak($verb == FIND | INVENT ?? 59 !! 12) }
   }
  }
  when 0..300 { $newloc = $rdest }
  when 301 {
   if !$holding || $holding == 1 && toting EMERALD { $newloc = 99 + 100 - $loc }
   else {$newloc = $loc; rspeak 117; }
  }
  when 302 {
   drop EMERALD, $loc;
   $newloc = $loc == 33 ?? 100 !! 33;
  }
  when 303 {
   if @prop[TROLL] == 1 {
    pspeak TROLL, 1;
    @prop[TROLL] = 0;
    move TROLL2, 0;
    move TROLL2+100, 0;
    move TROLL, 117;
    move TROLL+100, 122;
    juggle CHASM;
    $newloc = $loc;
   } else {
    $newloc = $loc == 117 ?? 122 !! 117;
    @prop[TROLL] = 1 if @prop[TROLL] == 0;
    if toting BEAR {
     rspeak 162;
     @prop[CHASM] = 1;
     @prop[TROLL] = 2;
     drop BEAR, $newloc;
     @fixed[BEAR] = -1;
     @prop[BEAR] = 3;
     $tally2++ if @prop[SPICES] < 0;
     $oldloc2 = $newloc;
     death;
    }
   }
  }
  when 500 ^..* { rspeak $rdest-500 }
  default { bug 20 }
 }
}

sub death() {
# 99:
 if $closing {
  rspeak 131;
  $numdie++;
  normend;
 } else {
  my $yea = yes(81 + $numdie*2, 82 + $numdie*2, 54);
  $numdie++;
  normend if $numdie == MAXDIE || !$yea;
  @place[WATER, OIL] = 0, 0;
  @prop[LAMP] = 0 if toting LAMP;
  drop $_, $_ == LAMP ?? 1 !! $oldloc2 for (^65).grep(*.toting).reverse;
  ($loc, $oldloc) = 3, 3;
  $goto = 2000;
  # next bigLoop;
 }
}

sub score(Bool $scoring --> int) {
 my int $score = 0;
 for 50...64 -> $i {
  $score += 2 if @prop[$i] >= 0;
  $score += $i == CHEST ?? 12 !! $i > CHEST ?? 14 !! 10
   if @place[$i] == 3 && @prop[$i] == 0;
 }
 $score += (MAXDIE - $numdie) * 10;
 $score += 4 if !($scoring || $gaveup);
 $score += 25 if $dflag != 0;
 $score += 25 if $closing;
 if $closed {
  given $bonus {
   when 0 { $score += 10 }
   when 133 { $score += 45 }
   when 134 { $score += 30 }
   when 135 { $score += 25 }
  }
 }
 $score++ if @place[MAGZIN] == 108;
 $score += 2;
 $score -= @hints[$_;1] if @hinted[$_] for 1...9;
 return $score;
}

sub normend() {
 my $score = score(False);
 say "\n\n\nYou scored $score out of a possible 350 using $turns turns.";
 my($rank, $next) = @classes.grep({ .key >= $score }).[0,1];
 if $rank {
  speak $rank.value;
  if $next {
   my $diff = $rank.key - $score + 1;
   say "\nTo achieve the next higher rating, you need $diff more point",
    $diff == 1 ?? ".\n" !! "s.\n";
  } else {
   say "\nTo achieve the next higher rating would be a neat trick!\n";
   say "Congratulations!!\n";
  }
 } else { say "\nYou just went off my scale!!\n" }
 exit 0;
}

sub doaction() {
# 5010:
 if $word2 {
# 2800:
  ($word1, $in1) = ($word2, $in2);
  $word2 = $in2 = Mu;
  $goto = 2610;
 } elsif $verb { transitive }
 else {
  say "\nWhat do you want to do with the $in1?";
  $goto = 2600;
 }
 # next bigLoop;
}

sub writeInt(IO $out, int32 $i) {
 #$out.write(Buf.new($i, size => 32), 4)
 # As far as anyone seems to know, the binary IO routines are currently only
 # defined for buf8's.
 $out.write(Buf.new(:size(8), (^4).map: { $i +> 8*(3-$_) +& 0xFF }), 4)
}

sub writeBool(IO $out, bool *@bits) {
 my Buf $data .= new: :size(8), (0, *+8 ... @bits.end).map:
  -> $i { [+|] (^8).map: { $i+$^j < @bits ?? @bits[$i+$^j] +< $^j !! 0 } };
 $out.write: $data, #`[ $data.elems ??? ] (@bits/8).ceiling;
}

sub readInt(IO $in --> int32) {
 my Buf $raw;
 $in.read: $raw, 4;
 [+|] (^4).map: { $raw[$^i] +< 8*(3-$^i) };
}

sub readBool(IO $in, int $qty --> List of bool) {
 my Buf $raw;
 $in.read: $raw, ($qty/8).ceiling;
 (^$qty).map: { $raw[$^i div 8] +& 1 +< ($^i % 8) };
}

sub writeStr(IO $out, Str $str) {
 my $utf = $str.encode: 'UTF-8';
 writeInt($out, $utf.elems);
 $out.write: $utf, $utf.elems;
}

sub readStr(IO $in --> Str) {
 my $len = readInt($in);
 my $utf;
 $in.read: $utf, $len;
 $utf.decode: 'UTF-8';
}

sub mspeak(int $msg) { speak @magicMsg[$msg] if $msg != 0 }

sub ciao() {mspeak 32; exit 0; }

sub yesm(int $x, int $y, int $z --> Bool) {
 loop {
  mspeak $x;
  my Str ($reply) = getin;
  if $reply eq 'YES' | 'Y' {
   mspeak $y;
   return True;
  } elsif $reply eq 'NO' | 'N' {
   mspeak $z;
   return False;
  } else { say "\nPlease answer the question." }
 }
}

sub datime( --> List of int) {
 # This function is supposed to return:
 # - the number of days since 1 Jan 1977 (220924800 in Unix epoch time)
 # - the number of minutes past midnight
 state DateTime $start .= new(year => 1977, month => 1, day => 1);
  # The time defaults to midnight, right?
 my DateTime $now = localtime;
 return ($now - $start) div 86400, $now.hour * 60 + $now.minute;
  # I assume the difference between two DateTime objects (when cast to a
  # Num-like, at least) is the number of seconds between them.
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
 if ($d ~~ $hbegin..$hend ?? @holid !! $d % 7 <= 1 ?? @wkend !! @wkday)\
  [$t div 60] {
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
  print "> ";
  $hbegin = $*IN.get;
  mspeak 28;
  print "> ";
  $hend = $*IN.get;
  my($d, $t) = datime;
  $hbegin += $d;
  $hend += $hbegin - 1;
  mspeak 29;
  print "> ";
  $hname = $*IN.get.substr(0, 20);
 }
 say "Length of short game (null to leave at $short):";
 print "> ";
 my int $x = $*IN.get;
 $short = $x if $x > 0;
 mspeak 12;
 $magic = (getin)[0] // $magic;
 mspeak 13;
 print "> ";
 $x = $*IN.get;
 $magnm = $x if $x > 0;
 say "Latency for restart (null to leave at $latency):";
 print "> ";
 $x = $*IN.get;
 mspeak 30 if 0 < $x < 45;
 $latency = 45 max $x if $x > 0;
 motd(True) if yesm(14, 0, 0);
 mspeak 15;  # Say something else?
 $blklin = True;

 # Save values to $magicFile
 my IO $abra;
 try {
  $abra = open $magicFile, :w, :bin;
  CATCH {$*ERR.say: "\nError: could not write to $magicFile: $!"; exit 1; }
 }
 writeBool $abra, @wkday;
 writeBool $abra, @wkend;
 writeBool $abra, @holid;
 writeInt $abra, $hbegin;
 writeInt $abra, $hend;
 writeStr $abra, $hname;
 writeInt $abra, $short;
 writeStr $abra, $magic;
 writeInt $abra, $magnm;
 writeInt $abra, $latency;
 writeStr $abra, $msg;
 $abra.close;

 ciao;
}

sub wizard( --> Bool) {
 return False if !yesm(16, 0, 7);
 mspeak 17;
 my Str $word = (getin)[0];
 if $word !eq $magic {mspeak 20; return False; }
 my($d, $t) = datime;
 $t = $t * 2 + 1;
 my int @wchrs[5] = 64, *;
 my int @val[5];
 for ^5 -> $y {
  my $x = 79 + $d % 5;
  $d div= 5;
  $t = ($t * 1027) % 1048576 for ^$x;
  @wchrs[$y] += @val[$y] = ($t*26) div 1048576 + 1;
 }
 if yesm(18, 0, 0) {mspeak 20; return False; }
 # .print for "\n", @wchrs».chr, "\n";
 say "\n{ chr |@wchrs }";
 @wchrs = (getin)[0].ord;
 # What happens if the inputted word is less than five characters?
 ($d, $t) = datime;
 $t = ($t div 60) * 40 + ($t div 10) * 10;
 $d = $magnm;
 for ^5 -> $y {
  @wchrs[$y] -= ((@val[$y] - @val[($y+1) % 5]).abs * ($d % 10) + ($t % 10))
   % 26 + 1;
  $t div= 10;
  $d div= 10;
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
  say "\nThe next holiday will be in $d day", $d == 1 ?? '' !! 's',
   ", namely $hname.";
 } else { say "\nToday is a holiday, namely $hname." }
}

sub hoursx(bool @hours[24], Str $day) {
 my bool $first = True;
 my int $from = -1;
 if @hours.all == False { say ' ' x 10, "$day  Open all day" }
 else {
  loop {
   repeat { $from++ } while @hours[$from] && $from < 24;
   if $from >= 24 {
    say ' ' x 10, $day, '  Closed all day' if $first;
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

sub newhrx(Str $day --> bool[24] #`< Right? > ) {
 my bool @newhrx[24] = False, *;
 say "Prime time on $day";
 loop {
  print "from: ";
  my int $from = $*IN.get.words.[0];
  return @newhrx if $from !~~ 0..^24;
  print "till: ";
  my int $till = $*IN.get.words.[0] - 1;
  return @newhrx if $till !~~ $from..^24;
  @newhrx[$from...$till] = True, *;
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
 # Read in values from $magicFile (see the declarations for the default values)
 my IO $abra;
 try {
  $abra = open $magicFile, :r, :bin;
  # If $magicFile cannot be opened, assume it does not exist and quietly leave
  # the default magic values in place.
  CATCH { return }
 }
 @wkday = readBool $abra, +@wkday;
 @wkend = readBool $abra, +@wkend;
 @holid = readBool $abra, +@holid;
 $hbegin = readInt $abra;
 $hend = readInt $abra;
 $hname = readStr $abra;
 $short = readInt $abra;
 $magic = readStr $abra;
 $magnm = readInt $abra;
 $latency = readInt $abra;
 $msg = readStr $abra;
 $abra.close;
}


sub MAIN(Str $oldGame?) {
 poof;
 if $oldGame.defined { vresume($oldGame) or exit 1 }
 else {
  $demo = start;
  motd(False);
  # Read in the item locations from data section 7:
  for $=adventData07.lines {
   my($obj, $p, $f) = .split: "\t";
   @place[$obj] = $p;
   @fixed[$obj] = $f // 0;
  }
  for @fixed.keys: { @fixed[$_] > 0 }.reverse -> $k {
   drop $k + 100, @fixed[$k];
   drop $k, @place[$k];
  }
  drop $_, @place[$_]
   for @fixed.keys: { @place[$_] != 0 && @fixed[$_] <= 0 }.reverse;
   # Yes, the above is valid.  Search for "bare closure" in S12.
  $newloc = 1;
  $limit = (@hinted[3] = yes(65, 1, 0)) ?? 1000 !! 330;
 }

 # ...and begin!

 # A note on the flow control used in this program:

 # Although the large loop below (cleverly named "bigLoop") contains the logic
 # for a single turn, not all of it is evaluated every turn; for example, after
 # most non-movement verbs, control passes to the original Fortran's label 2012
 # rather than to label 2 (the start of the loop).  In the original Fortran,
 # this was all handled by a twisty little maze of GOTO statements, all
 # different, but since GOTOs are heavily frowned upon nowadays, and because
 # this port of Adventure is intended to be an exercise in modern programming
 # techniques rather than in ancient ones, I had to come up with a better way.

 # (Side note: In the BDS C port of Adventure, all of the turn code is
 # evaluated every turn, and you are very likely to get killed by a dwarf when
 # picking up the axe in the middle of battle.)

 # My best idea was to divide the loop up at the necessary GOTO labels, put
 # each part inside a "when" block with a "proceed" at the end, and introduce a
 # global variable (named "$goto", of course) to switch on that indicated what
 # part of the loop to start the next iteration at.  (My other ideas were (a) a
 # state machine in which each section of the loop was a function that returned
 # a number representing the next function to call and (b) something involving
 # exceptions.)  This works, but it was not what I had hoped for.  Perl 6 seems
 # like it should have a more elegant solution to this problem, but I couldn't
 # find anything better in the Synopses.  If you know of something better, let
 # me know.

 # In summary: I apologize for the code that you are about to see.

 bigLoop: loop {
  given $goto {
   when *..2 {
    if 0 < $newloc < 9 && $closing {
     rspeak 130;
     $newloc = $loc;
     $clock2 = 15 if !$panic;
     $panic = True;
    }
    if $newloc != $loc && !forced($loc) && !bitset($loc, 3)
     && { @odloc[$^i] == $newloc && @dseen[$^i] }(any ^5) {
     $newloc = $loc;
     rspeak 2;
    }
    $loc = $newloc;
    # Dwarven logic:
    proceed if $loc == 0 || forced($loc) || bitset $newloc, 3;
    if $dflag == 0 {
     $dflag = 1 if $loc >= 15;
     proceed;
    }
    if $dflag == 1 {
     proceed if $loc < 15 || pct 95;
     $dflag = 2;
     @dloc[(^5).pick] = 0 if pct 50 for 1, 2;
     for ^5 -> $i {
      @dloc[$i] = 18 if @dloc[$i] == $loc;
      @odloc[$i] = @dloc[$i];
     }
     rspeak 3;
     drop AXE, $loc;
     proceed;
    }
    my int $dtotal, $attack, $stick = 0, *;
    dwarfLoop: for ^6 -> $i {  # The individual dwarven movement loop
     next if @dloc[$i] == 0;
     my int @tk = grep {
      $_ ~~ 15..300 && $_ != @odloc[$i] & @dloc[$i] && !forced($_)
       && !($i == 5 && bitset($_, 3))
     }, @travel[@dloc[$i];*;0].grep(* div 1000 != 100) »%» 1000;
     @tk.push: @odloc[$i] if !@tk;
     (@odloc[$i], @dloc[$i]) = @dloc[$i], @tk.pick;
     @dseen[$i] = (@dseen[$i] && $loc >= 15) || @dloc[$i] | @odloc[$i] == $loc;
     if @dseen[$i] {
      @dloc[$i] = $loc;
      if $i == 5 {
       # Pirate logic:
       next if $loc == CHLOC || @prop[CHEST] >= 0;
       my Bool $k = False;
       for 50...64 -> $j {
	next if $j == PYRAM && $loc == 100 | 101;
	if toting $j {
	 rspeak 128;
	 move CHEST, CHLOC if @place[MESSAG] == 0;
	 move MESSAG, CHLOC2;
	 for 50...64 -> $j {
	  next if $j == PYRAM && $loc == 100 | 101;
	  carry $j, $loc if at($j) && @fixed[$j] == 0;
	  drop $j, CHLOC if toting $j;
	 }
	 @dloc[5] = @odloc[5] = CHLOC;
	 @dseen[5] = False;
	 next dwarfLoop;
	}
	$k = True if here $j;
       }
       if $tally == $tally2 + 1 && !$k && @place[CHEST] == 0 && here(LAMP) 
	&& @prop[LAMP] == 1 {
	rspeak 186;
	move CHEST, CHLOC;
	move MESSAG, CHLOC2;
	@dloc[5] = @odloc[5] = CHLOC;
	@dseen[5] = False;
       } elsif @odloc[5] != @dloc[5] && pct 20 { rspeak 127 }
      } else {
       $dtotal++;
       if @odloc[$i] == @dloc[$i] {
	$attack++;
	$knifeloc = $loc if $knifeloc >= 0;
	$stick++ if (^1000).pick < 95 * ($dflag - 2);
       }
      }
     }
    } # end of individual dwarf loop
    proceed if $dtotal == 0;
    if $dtotal == 1 { rspeak 4 }
    else {
     say "\nThere are $dtotal threatening little dwarves in the room with you."
    }
    proceed if $attack == 0;
    $dflag = 3 if $dflag == 2;
    my int $k;
    if $attack == 1 {rspeak 5; $k = 52; }
    else {say "\n$attack of them throw knives at you!"; $k = 6; }
    if $stick <= 1 {
     rspeak $k + $stick;
     proceed if $stick == 0;
    } else { say "\n$stick of them get you!" }
    $oldloc2 = $loc;
    death;
    # If the player is reincarnated after being killed by a dwarf, they GOTO
    # label 2000 using fallthrough rather than with any special flow control.
    proceed;
   }

   when *..2000 {
    if $loc == 0 {death; next bigLoop; }
    my Str $kk = @shortdesc[$loc];
    $kk = @longdesc[$loc] if @abb[$loc] !% $abbnum || !$kk.defined;
    if !forced($loc) && dark() {
     if $wzdark && pct 35 {
      rspeak 23;
      $oldloc2 = $loc;
      death;
      next bigLoop;
     }
     $kk = @rmsg[16];
    }
    rspeak 141 if toting BEAR;
    speak $kk;
    if forced $loc {domove 1; next bigLoop; }
    rspeak 8 if $loc == 33 && pct(25) && !$closing;
    if !dark() {
     @abb[$loc]++;
     for @atloc[$loc] -> $obj {
      $obj -= 100 if $obj > 100;
      next if $obj == STEPS && toting NUGGET;
      if @prop[$obj] < 0 {
       next if $closed;
       @prop[$obj] = $obj == RUG | CHAIN;
       $tally--;
       $limit = 35 min $limit if $tally == $tally2 && $tally != 0;
      }
      pspeak $obj, $obj == STEPS && $loc == @fixed[STEPS] ?? 1 !! @prop[$obj];
     }
    }
    proceed;
   }

   when *..2012 {($verb, $obj) = 0, 0; proceed; }

   when *..2600 {
    hintLoop: for 4...9 -> $hint {
     next if @hinted[$hint];
     @hintlc[$hint] = -1 if !bitset $loc, $hint;
     @hintlc[$hint]++;
     if @hintlc[$hint] >= @hints[$hint;0] {
      given $hint {
       when 4 {
	if @prop[GRATE] != 0 || here(KEYS) {@hintlc[$hint] = 0; next hintLoop; }
       }
       when 5 { next hintLoop if !here(BIRD) || !toting(ROD) || $obj != BIRD }
       when 6 {
	if !here(SNAKE) || here(BIRD) {@hintlc[$hint] = 0; next hintLoop; }
       }
       when 7 {
	if @atloc[$loc, $oldloc, $oldloc2] || $holding <= 1 {
	# This ^^ is supposed to check whether there is at least one item at
	# any of the given locations; does it work right?
	 @hintlc[$hint] = 0;
	 next hintLoop;
	}
       }
       when 8 {
	if @prop[EMERALD] == -1 || @prop[PYRAM] != -1 {
	 @hintlc[$hint] = 0;
	 next hintLoop;
	}
       }
      }
      @hintlc[$hint] = 0;
      next hintLoop if !yes(@hints[$hint;2], 0, 54);
      say "\nI am prepared to give you a hint, but it will cost you ",
       @hints[$hint;1], " points.";
      @hinted[$hint] = yes(175, @hints[$hint;3], 54);
      limit += 30 * @hints[$hint;1] if @hinted[$hint] && $limit > 30;
     }
    }
    if $closed {
     pspeak OYSTER, 1 if @prop[OYSTER] < 0 && toting OYSTER;
     @prop[$_] = -1-@prop[$_] for grep { toting($_) && @prop[$_] < 0 }, 1...64;
    }
# 2605:
    $wzdark = dark;
    $knifeloc = 0 if 0 < $knifeloc != $loc;
    ($word1, $in1, $word2, $in2) = getin;
    proceed;
   }

   when *..2608 {
    $foobar = 0 min -$foobar;
    maint if $turns == 0 && $word1 eq 'MAGIC' && $word2 eq 'MODE';
    $turns++;
    if $demo && $turns >= $short {mspeak 1; normend; }
    $verb = 0 if $verb == SAY && $word2;
    if $verb == SAY {vsay; next bigLoop; }
    $clock1-- if $tally == 0 && 15 <= $loc != 33;
    if $clock1 == 0 {
     @prop[GRATE, FISSUR] = 0, 0;
     @dloc = 0, *;
     @dseen = False, *;
     move TROLL, 0;
     move TROLL+100, 0;
     move TROLL2, 117;  # There are no trolls in *Troll 2*.
     move TROLL2+100, 122;
     juggle CHASM;
     destroy BEAR if @prop[BEAR] != 3;
     @prop[CHAIN, AXE] = 0, 0;
     @fixed[CHAIN, AXE] = 0, 0;
     rspeak 129;
     $clock1 = -1;
     $closing = True;
     proceed;  # GOTO 19999, a.k.a. 2609
    }
    $clock2-- if $clock1 < 0;
    if $clock2 == 0 {
     @prop[$_] = put $_, 115, $_ == BOTTLE ?? 1 !! 0
      for BOTTLE, PLANT, OYSTER, LAMP, ROD, DWARF;
     ($loc, $oldloc, $newloc) = 115, *;
     put GRATE, 116, 0;
     @prop[$_] = put $_, 116, $_ == SNAKE | BIRD ?? 1 !! 0
      for SNAKE, BIRD, CAGE, ROD2, PILLOW;
     @prop[MIRROR] = put MIRROR, 115, 0;
     @fixed[MIRROR] = 116;
     destroy $_ for grep { toting $_ }, 1...64;
     # Could this be written as ".destroy for (1...64).grep: *.toting" ?
     rspeak 132;
     $closed = True;
     $goto = 2;
     next bigLoop;
    }
    $limit-- if @prop[LAMP] == 1;
    if $limit <= 30 && here(BATTER) && @prop[BATTER] == 0 && here(LAMP) {
     rspeak 188;
     @prop[BATTER] = 1;
     drop BATTER, $loc if toting BATTER;
     $limit += 2500;
     $lmwarn = False;
    } elsif $limit == 0 {
     $limit = -1;
     @prop[LAMP] = 0;
     rspeak 184 if here LAMP;
    } elsif $limit < 0 && $loc <= 8 {
     rspeak 185;
     $gaveup = True;
     normend;
    } elsif $limit <= 30 && !$lmwarn && here LAMP {
     $lmwarn = True;
     rspeak(@place[BATTER] == 0 ?? 183 !! @prop[BATTER] == 1 ?? 189 !! 187);
    }
    proceed;
   }

   when *..2609 {
# This label is 19999 in the original Fortran, but it is being treated here as
# 2609 so that fall-through will work correctly.
    if $word1 eq 'ENTER' && $word2 eq 'STREA' | 'WATER' {
     rspeak(liqloc($loc) == WATER ?? 70 !! 43);
     $goto = 2012;
     next bigLoop;
    }
    if $word1 eq 'ENTER' && $word2 {
     ($word1, $in1) = ($word2, $in2);
     $word2 = $in2 = Mu;
    } elsif $word1 eq 'WATER' | 'OIL' && $word2 eq 'PLANT' | 'DOOR' {
     $word2 = 'POUR' if at vocab($word2, 1)
    }
    proceed;
   }

   when *..2610 {
    rspeak 17 if $word1 eq 'WEST' && ++$iwest == 10;
    proceed;
   }

   when *..2630 {
    my int $i = vocab $word1, -1;
    if $i == -1 {
     rspeak(pct(20) ?? 61 !! pct(20) ?? 13 !! 60);
     $goto = 2600;
     next bigLoop;
    }
    my int $k = $i % 1000;
    given $i div 1000 {
     when 0 { domove $k }
     when 1 {
# 5000:
      $obj = $k;
      if @fixed[$obj] == $loc || here $obj { doaction }
      else {
       # You would think that this part would be better expressed as a "given"
       # block, but that turns out to be far less concise.
       if $obj == GRATE {
	$k = DEPRESSION if $loc == 1 | 4 | 7;
	$k = ENTRANCE if 9 < $loc < 15;
	if $k != GRATE { domove $k }
	elsif $verb == FIND | INVENT && !$word2 { doaction }
	else {say "\nI see no $in1 here."; $goto = 2012; }
       } elsif $obj == DWARF && $dflag >= 2 && @dloc[^5].any == $loc
        || $obj == liq() && here(BOTTLE) || $obj == liqloc($loc) {
        doaction
       } elsif $obj == PLANT && at(PLANT2) && @prop[PLANT2] != 0 {
	$obj = PLANT2;
	doaction;
       } elsif $obj == KNIFE && $knifeloc == $loc {
	$knifeloc = -1;
	rspeak 116;
	$goto = 2012;
       } elsif $obj == ROD && here ROD2 {$obj = ROD2; doaction; }
       elsif $verb == FIND | INVENT && !$word2 { doaction }
       else {say "\nI see no $in1 here."; $goto = 2012; }
      }
     }
     when 2 {
# 4000:
      $verb = $k;
      if $verb == SAY | SUSPEND | RESUME { $obj = $word2.defined }
      # This assignment just indicates whether an object was supplied.
      elsif $word2 {
       ($word1, $in1) = ($word2, $in2);
       $word2 = $in2 = Mu;
       $goto = 2610;
       next bigLoop;
      }
      $obj ?? transitive !! intransitive;
     }
     when 3 {rspeak $k; $goto = 2012; }
     default { bug 22 }
    }
   }

  }
 } # end of bigLoop

}


# Verb functions:

sub intransitive() {
# Label 4080 (intransitive verb handling):

# As this function is only called at a single point in the code, it doesn't
# actually need to be a separate routine, but I think we can all agree that
# MAIN is complicated enough as it is without stuffing yet another 120-line
# "given" block inside of it.

 $goto = 2012;
 given $verb {
  when NOTHING { rspeak 54 }
  when WALK { rspeak @actspk[$verb] }
  when DROP | SAY | WAVE | CALM | RUB | THROW | FIND | FEED | BREAK | WAKE {
# 8000:
   say "\n$in1 what?";
   $obj = 0;
   $goto = 2600;
  }
  when TAKE {
   if @atloc[$loc] != 1 || $dflag >= 2 && @dloc[^5].any == $loc {
    say "\n$in1 what?";
    $obj = 0;
    $goto = 2600;
   } else {
    $obj = @atloc[$loc;0];
    vtake;
   }
  }
  when OPEN | LOCK {
   $obj = CLAM if here CLAM;
   $obj = OYSTER if here OYSTER;
   $obj = DOOR if at DOOR;
   $obj = GRATE if at GRATE;
   if $obj != 0 && here CHAIN {
    say "\n$in1 what?";
    $obj = 0;
    $goto = 2600;
   } else {
    $obj = CHAIN if here CHAIN;
    if $obj == 0 { rspeak 28 }
    else { vopen }
   }
  }
  when EAT {
   if here FOOD {
    destroy FOOD;
    rspeak 72;
   } else {
    say "\n$in1 what?";
    $obj = 0;
    $goto = 2600;
   }
  }
  # Yes, this is supposed to be an assignment:
  when QUIT { normend if $gaveup = yes(22, 54, 54) }
  when INVENT {
   my int $spk = 98;
   for ^65 -> $i {
    next if $i == BEAR || !toting $i;
    rspeak 99 if $spk == 98;
    $blklin = False;
    pspeak $i, -1;
    $blklin = True;
    $spk = 0;
   }
   $spk = 141 if toting BEAR;
   rspeak $spk;
  }
  when SCORE {
   my int $score = score(True);
   say "\nIf you were to quit now, you would score $score out of a possible",
    " 350.";
   normend if $gaveup = yes(143, 54, 54);
  }
  when FOO {
   my $k = vocab $word1, 3;
   if $foobar == 1-$k {
    $foobar = $k;
    if $k != 4 {rspeak 54; return; }
    $foobar = 0;
    if @place[EGGS] == 92 || toting(EGGS) && $loc == 92 { rspeak 42 }
    else {
     @prop[TROLL] = 1 if @place[EGGS] & @place[TROLL] & @prop[TROLL] == 0;
     $k = $loc == 92 ?? 0 !! here(EGGS) ?? 1 !! 2;
     move EGGS, 92;
     pspeak EGGS, $k;
    }
   } else { rspeak($foobar ?? 151 !! 42) }
  }
  when BRIEF {
   $abbnum = 10000;
   $detail = 3;
   rspeak 156;
  }
  when READ {
   $obj = MAGZIN if here MAGZIN;
   $obj = $obj * 100 + TABLET if here TABLET;
   $obj = $obj * 100 + MESSAG if here MESSAG;
   $obj = OYSTER if $closed && toting OYSTER;
   if $obj > 100 || $obj == 0 || dark {
    say "\n$in1 what?"; $obj = 0; $goto = 2600;
   } else { vread }
  }
  when SUSPEND { vsuspend("%*ENV<HOME>/.adventure") }
  when RESUME { vresume("%*ENV<HOME>/.adventure") }
  when HOURS {mspeak 6; hours; }
  when ON { von }
  when OFF { voff }
  when KILL { vkill }
  when POUR { vpour }
  when DRINK { vdrink }
  when FILL { vfill }
  when BLAST { vblast }
  default { bug 23 }
 }
}

sub transitive() {
# Label 4090 (transitive verb handling):
 $goto = 2012;
 given $verb {
  when TAKE { vtake }
  when DROP { vdrop }
  when SAY { vsay }
  when OPEN | LOCK { vopen }
  when NOTHING { rspeak 54 }
  when ON { von }
  when OFF { voff }
  when WAVE {
# 9090:
   if !toting($obj) && !($obj == ROD && toting(ROD2)) { rspeak 29 }
   elsif $obj != ROD || !at(FISSUR) || !toting($obj) || $closing {
    rspeak @actspk[$verb]
   } else {
    @prop[FISSUR] = 1 - @prop[FISSUR];
    pspeak FISSUR, 2 - @prop[FISSUR];
   }
  }
  when CALM | WALK | QUIT | SCORE | FOO | BRIEF | HOURS {
   rspeak @actspk[$verb]
  }
  when KILL { vkill }
  when POUR { vpour }
  when EAT {
# 9140:
   if $obj == FOOD {destroy FOOD; rspeak 72; }
   elsif $obj == BIRD | SNAKE | CLAM | OYSTER | DWARF | DRAGON | TROLL | BEAR {
    rspeak 71
   } else { rspeak @actspk[$verb] }
  }
  when DRINK { vdrink }
  when RUB { rspeak($obj == LAMP ?? @actspk[$verb] !! 76) }
  when THROW {
# 9170:
   $obj = ROD2 if toting(ROD2) && $obj == ROD && !toting(ROD);
   if !toting $obj { rspeak @actspk[$verb] }
   elsif 50 <= $obj < 65 && at(TROLL) {
    drop $obj, 0;
    move TROLL, 0;
    move TROLL+100, 0;
    drop TROLL2, 117;
    drop TROLL2+100, 122;
    juggle CHASM;
    rspeak 159;
   } elsif $obj == FOOD && here BEAR {$obj = BEAR; vfeed; }
   elsif $obj == AXE {
    my int $i = (^5).first({ @dloc[$_] == $loc }) // -1;
    if $i != -1 {
     if (^3).pick == 0 { rspeak 48 }
     else {
      @dseen[$i] = False;
      @dloc[$i] = 0;
      rspeak(++$dkill == 1 ?? 149 !! 47);
     }
     drop AXE, $loc;
     domove NULL;
    } elsif at(DRAGON) && @prop[DRAGON] == 0 {
     rspeak 152;
     drop AXE, $loc;
     domove NULL;
    } elsif at(TROLL) {
     rspeak 158;
     drop AXE, $loc;
     domove NULL;
    } elsif here(BEAR) && @prop[BEAR] == 0 {
     drop AXE, $loc;
     @fixed[AXE] = -1;
     @prop[AXE] = 1;
     juggle BEAR;  # Don't try this at home, kids.
     rspeak 164;
    } else {$obj = 0; vkill; }
   } else { vdrop }
  }
  when FIND | INVENT {
# 9190:
   if toting $obj { rspeak 24 }
   elsif $closed { rspeak 138 }
   elsif $obj == DWARF && $dflag >= 2 && @dloc[^5].any == $loc { rspeak 94 }
   elsif at($obj) || (liq == $obj && at(BOTTLE)) || $obj == liqloc($loc) {
    rspeak 94
   } else { rspeak @actspk[$verb] }
  }
  when FEED { vfeed }
  when FILL { vfill }
  when BLAST { vblast }
  when READ { vread }
  when BREAK {
# 9280:
   if $obj == VASE && @prop[VASE] == 0 {
    drop VASE, $loc if toting VASE;
    @prop[VASE] = 2;
    @fixed[VASE] = -1;
    rspeak 198;
   } elsif $obj != MIRROR { rspeak @actspk[$verb] }
   elsif !$closed { rspeak 148 }
   else {rspeak 197; rspeak 136; normend; }
  }
  when WAKE {
# 9290:
   if $obj == DWARF && $closed {rspeak 199; rspeak 136; normend; }
   else { rspeak @actspk[$verb] }
  }
  when SUSPEND { vsuspend($in2) }
  when RESUME { vresume($in2) }
  default { bug 24 }
 }
}

sub vtake() {
# 9010:
 if toting $obj {rspeak @actspk[$verb]; return; }
 my int $spk = 25;
 $spk = 115 if $obj == PLANT && @prop[PLANT] <= 0;
 $spk = 169 if $obj == BEAR && @prop[BEAR] == 1;
 $spk = 170 if $obj == CHAIN && @prop[BEAR] != 0;
 if @fixed[$obj] {rspeak $spk; return; }
 if $obj == WATER | OIL {
  if !here(BOTTLE) || liq != $obj {
   $obj = BOTTLE;
   if toting(BOTTLE) && @prop[BOTTLE] == 1 { vfill }
   else {
    $spk = 105 if @prop[BOTTLE] != 1;
    $spk = 104 if !toting BOTTLE;
    rspeak $spk;
   }
   return;
  }
  $obj = BOTTLE;
 }
 if $holding >= 7 {rspeak 92; return; }
 if $obj == BIRD && @prop[BIRD] == 0 {
  if toting ROD {rspeak 26; return; }
  if !toting CAGE {rspeak 27; return; }
  @prop[BIRD] = 1;
 }
 carry BIRD+CAGE-$obj, $loc if $obj == BIRD | CAGE && @prop[BIRD] != 0;
 carry $obj, $loc;
 my $k = liq;
 @place[$k] = -1 if $obj == BOTTLE && $k != 0;
 rspeak 54;
}

sub vopen() {
# 9040:
 my int $spk = @actspk[$verb];
 given $obj {
  when CLAM | OYSTER {
   my $k = ($obj == OYSTER);
   $spk = 124 + $k;
   $spk = 120 + $k if toting $obj;
   $spk = 122 + $k if !toting TRIDENT;
   $spk = 61 if $verb == LOCK;
   if $spk == 124 {
    destroy CLAM;
    drop OYSTER, $loc;
    drop PEARL, 105;
   }
  }
  when DOOR { $spk = @prop[DOOR] == 1 ?? 54 !! 111 }
  when CAGE { $spk = 32 }
  when KEYS { $spk = 55 }
  when CHAIN {
   if !here KEYS { $spk = 31 }
   elsif $verb == LOCK {
    $spk = 172;
    $spk = 34 if @prop[CHAIN] != 0;
    $spk = 173 if $loc != 130;
    if $spk == 172 {
     @prop[CHAIN] = 2;
     drop CHAIN, $loc if toting CHAIN;
     @fixed[CHAIN] = -1;
    }
   } else {
    $spk = 171;
    $spk = 41 if @prop[BEAR] == 0;
    $spk = 37 if @prop[CHAIN] == 0;
    if $spk == 171 {
     @prop[CHAIN] = @fixed[CHAIN] = 0;
     @prop[BEAR] = 2 if @prop[BEAR] != 3;
     @fixed[BEAR] = 2 - @prop[BEAR];
    }
   }
  }
  when GRATE {
   if !here KEYS { $spk = 31 }
   elsif $closing {
    $spk = 130;
    $clock2 = 15 if !$panic;
    $panic = True;
   } else {
    $spk = 34 + @prop[GRATE];
    @prop[GRATE] = ($verb != LOCK);
    $spk += 2 * @prop[GRATE];
   }
  }
 }
 rspeak $spk;
}

sub vread() {
# 9270:
 if dark() { say "\nI see no $in1 here." }
 else {
  my int $spk = @actspk[$verb];
  $spk = 190 if $obj == MAGZIN;
  $spk = 196 if $obj == TABLET;
  $spk = 191 if $obj == MESSAG;
  $spk = 194 if $obj == OYSTER && @hinted[2] && toting OYSTER;
  if $obj != OYSTER || @hinted[2] || !toting(OYSTER) || !$closed { rspeak $spk }
  else { @hinted[2] = yes(192, 193, 54) }
 }
}

sub vkill() {
# 9120:
 if $obj == 0 {
  $obj = DWARF if $dflag >= 2 && @dloc[^5].any == $loc;
  $obj = $obj * 100 + SNAKE if here SNAKE;
  $obj = $obj * 100 + DRAGON if at(DRAGON) && @prop[DRAGON] == 0;
  $obj = $obj * 100 + TROLL if at TROLL;
  $obj = $obj * 100 + BEAR if here(BEAR) && @prop[BEAR] == 0;
  if $obj > 100 {say "\n$in1 what?"; $obj = 0; $goto = 2600; return; }
  if $obj == 0 {
   $obj = BIRD if here(BIRD) && $verb != THROW;
   $obj = $obj * 100 + CLAM if here(CLAM | OYSTER);
   if $obj > 100 {say "\n$in1 what?"; $obj = 0; $goto = 2600; return; }
  }
 }
 given $obj {
  when BIRD {
   if $closed { rspeak 137 }
   else {
    destroy BIRD;
    @prop[BIRD] = 0;
    $tally2++ if @place[SNAKE] == 19;
    rspeak 45;
   }
  }
  when 0 { rspeak 44 }
  when CLAM | OYSTER { rspeak 150 }
  when SNAKE { rspeak 46 }
  when DWARF {
   if $closed {rspeak 136; normend; }
   else { rspeak 49 }
  }
  when DRAGON {
   if @prop[DRAGON] != 0 { rspeak 167 }
   else {
    rspeak 49;
    ($verb, $obj) = (0, 0);
    ($word1, $in1, $word2, $in2) = getin;
    if !($word1 eq 'YES' | 'Y') {$goto = 2608; return; }
    pspeak DRAGON, 1;
    @prop[DRAGON, RUG] = 2, 0;
    move DRAGON+100, -1;
    move RUG+100, 0;
    move DRAGON, 120;
    move RUG, 120;
    move $_, 120 for grep { @place[$_] == 119 | 121 }, ^65;
    $loc = 120;
    domove NULL;
   }
  }
  when TROLL { rspeak 157 }
  when BEAR { rspeak(165 + (@prop[BEAR]+1) div 2) }
  default { rspeak @actspk[$verb] }
 }
}

sub vpour() {
# 9130:
 $obj = liq if $obj == BOTTLE | 0;
 if $obj == 0 {say "\n$in1 what?"; $obj = 0; $goto = 2600; }
 elsif !toting $obj { rspeak @actspk[$verb] }
 elsif !($obj == OIL | WATER) { rspeak 78 }
 else {
  @prop[BOTTLE] = 1;
  @place[$obj] = 0;
  if at DOOR {
   @prop[DOOR] = ($obj == OIL);
   rspeak 113 + @prop[DOOR];
  } elsif at PLANT {
   if $obj != WATER { rspeak 112 }
   else {
    pspeak PLANT, @prop[PLANT] + 1;
    @prop[PLANT] = (@prop[PLANT] + 2) % 6;
    @prop[PLANT2] = @prop[PLANT] div 2;
    domove NULL;
   }
  } else { rspeak 77 }
 }
}

sub vdrink() {
# 9150:
 if $obj == 0 && liqloc($loc) != WATER && (liq != WATER || !here BOTTLE) {
  say "\n$in1 what?";
  $obj = 0;
  $goto = 2600;
 } elsif $obj == 0 | WATER {
  if liq == WATER && here BOTTLE {
   @prop[BOTTLE] = 1;
   @place[WATER] = 0;
   rspeak 74;
  } else { rspeak @actspk[$verb] }
 } else { rspeak 110 }
}

sub vfill() {
# 9220:
 if $obj == VASE {
  if liqloc($loc) == 0 { rspeak 144 }
  elsif !toting VASE { rspeak 29 }
  else {
   rspeak 145;
   @prop[VASE] = 2;
   @fixed[VASE] = -1;
# In the original Fortran, when the vase is filled with water or oil, its
# property is set so that it breaks into pieces, *but* the code then branches
# to label 9024 to actually drop the vase.  Once you cut out the unreachable
# states, it turns out that the vase remains intact if the pillow is present,
# but even if it survives it is still marked as a fixed object and can't be
# picked up again.  This is probably a bug in the original code, but who am I
# to fix it?
   @prop[VASE] = 0 if at PILLOW;
   pspeak VASE, @prop[VASE] + 1;
   drop $obj, $loc;
  }
 } else {
  if !($obj == 0 | BOTTLE) { rspeak @actspk[$verb] }
  elsif $obj == 0 && !here BOTTLE {
   say "\n$in1 what?";
   $obj = 0;
   $goto = 2600;
  } elsif liq != 0 { rspeak 105 }
  elsif liqloc($loc) == 0 { rspeak 106 }
  else {
   @prop[BOTTLE] = @cond[$loc] +& 2;
   @place[liq] = -1 if toting BOTTLE;
   rspeak(liq == OIL ?? 108 !! 107);
  }
 }
}

sub vblast() {
# 9230:
 if @prop[ROD2] < 0 || !$closed { rspeak @actspk[$verb] }
 else {
  $bonus = 133;
  $bonus = 134 if $loc == 115;
  $bonus = 135 if here ROD2;
  rspeak $bonus;
  normend;
  # Fin
 }
}

sub von() {
# 9070:
 if !here LAMP { rspeak @actspk[$verb] }
 elsif $limit < 0 { rspeak 184 }
 else {
  @prop[LAMP] = 1;
  rspeak 39;
  $goto = 2000 if $wzdark;
 }
}

sub voff() {
# 9080:
 if !here LAMP { rspeak @actspk[$verb] }
 else {
  @prop[LAMP] = 0;
  rspeak 40;
  rspeak 16 if dark;
 }
}

sub vdrop() {
# 9020:
 $obj = ROD2 if toting(ROD2) && $obj == ROD && !toting ROD;
 if !toting $obj {rspeak @actspk[$verb]; return; }
 if $obj == BIRD && here SNAKE {
  rspeak 30;
  if $closed {rspeak 136; normend; }
  destroy SNAKE;
  @prop[SNAKE] = 1;
 } elsif $obj == COINS && here VEND {
  destroy COINS;
  drop BATTER, $loc;
  pspeak BATTER, 0;
  return;
 } elsif $obj == BIRD && at(DRAGON) && @prop[DRAGON] == 0 {
  rspeak 154;
  destroy BIRD;
  @prop[BIRD] = 0;
  $tally2++ if @place[SNAKE] == 19;
  return;
 } elsif $obj == BEAR && at(TROLL) {
  rspeak 163;
  move TROLL, 0;
  move TROLL+100, 0;
  move TROLL2, 117;
  move TROLL2+100, 122;
  juggle CHASM;
  @prop[TROLL] = 2;
 } elsif $obj == VASE && $loc != 96 {
  @prop[VASE] = at(PILLOW) ?? 0 !! 2;
  pspeak VASE, @prop[VASE] + 1;
  @fixed[VASE] = -1 if @prop[VASE] != 0;
 } else { rspeak 54 }
 my int $k = liq;
 $obj = BOTTLE if $k == $obj;
 @place[$k] = 0 if $obj == BOTTLE && $k != 0;
 drop BIRD, $loc if $obj == CAGE && @prop[BIRD] != 0;
 @prop[BIRD] = 0 if $obj == BIRD;
 drop $obj, $loc;
}

sub vfeed() {
# 9210:
 given $obj {
  when BIRD { rspeak 100 }
  when SNAKE {
   if !$closed && here BIRD {
    destroy BIRD;
    @prop[BIRD] = 0;
    $tally2++;
    rspeak 101;
   } else { rspeak 102 }
  }
  when TROLL { rspeak 182 }
  when DRAGON { rspeak(@prop[DRAGON] != 0 ?? 110 !! 102) }
  when DWARF {
   if !here FOOD { rspeak @actspk[$verb] }
   else {$dflag++; rspeak 103; }
  }
  when BEAR {
   if !here FOOD { rspeak (102, @actspk[$verb] xx 2, 110)[@prop[BEAR]] }
   else {
    destroy FOOD;
    @prop[BEAR] = 1;
    @fixed[AXE] = 0;
    @prop[AXE] = 0;
    rspeak 168;
   }
  }
  default { rspeak 14 }
 }
}

sub vsay() {
# 9030:
 my Str $tk = $in2 // $in1;
 $word1 = $word2 // $word1;
 if vocab($word1, -1) == 62 | 65 | 71 | 2025 {
  $word2 = Mu;
  $obj = 0;
  $goto = 2630;
 } else { say "\nOkay, \"$tk\"." }
}


# Below are the routines for saving & restoring games.  All user data is
# written out in binary form in the order that the variables are declared.
# @atloc is stored by preceding each sub-array by the number of elements within
# it.  As pack() and unpack() have not been fully specified for Perl 6 yet (and
# thus certainly won't be available in Rakudo for a while), the data is written
# & read using homemade routines.

sub vsuspend(Str $file) {
 if $demo {rspeak 201; return; }
 say "\nI can suspend your adventure for you so that you can resume later, but";
 say "you will have to wait at least $latency minutes before continuing.";
 return if !yes(200, 54, 54);
 ($saved, $savet) = datime;
 say "\nSaving to $file ...";
 my IO $adv;
 try {
  $adv = open $file, :w, :bin;
  CATCH {$*ERR.say: "\nError: could not write to $file: $!"; return; }
 }
 # Don't use any CATCH blocks for the below lines; if they fail, exception
 # handling won't help you out.
 writeInt $adv, $_ for $loc, $newloc, $oldloc, $oldloc2, $limit, $turns,
  $iwest, $knifeloc, $detail, $numdie, $holding, $foobar, $tally, $tally2,
  $abbnum, $clock1, $clock2;
 writeBool $adv, $wzdark, $closing, $lmwarn, $panic, $closed;
 writeInt $adv, $_ for @prop, @abb, @hintlc;
 writeBool $adv, @hinted;
 writeInt $adv, $_ for @dloc, @odloc;
 writeBool $adv, @dseen;
 writeInt $adv, $_ for $dflag, $dkill, @place, @fixed;
 for @atloc {
  writeInt $adv, .elems;
  writeInt $adv, $_ for @($_);
 }
 writeInt $adv, $saved;
 writeInt $adv, $savet;
 $adv.close;
 ciao;
}

sub vresume(Str $file --> Bool) {
 if $demo {mspeak 9; return False; }
 if $turns > 1 {
  say "\nTo resume an earlier Adventure, you must abandon the current one.";
  # This message is taken from the 430 pt. version of Adventure (version 2.5).
  return False if !yes(200, 54, 54);
 }
 say "\nRestoring from $file ...";
 my IO $adv;
 try {
  $adv = open $file, :r, :bin;
  CATCH {$*ERR.say: "\nError: could not read $file: $!"; return False; }
 }
 # Don't use any CATCH blocks for the below lines; if they fail, exception
 # handling won't help you out.
 $loc = readInt $adv;
 $newloc = readInt $adv;
 $oldloc = readInt $adv;
 $oldloc2 = readInt $adv;
 $limit = readInt $adv;
 $turns = readInt $adv;
 $iwest = readInt $adv;
 $knifeloc = readInt $adv;
 $detail = readInt $adv;
 $numdie = readInt $adv;
 $holding = readInt $adv;
 $foobar = readInt $adv;
 $tally = readInt $adv;
 $tally2 = readInt $adv;
 $abbnum = readInt $adv;
 $clock1 = readInt $adv;
 $clock2 = readInt $adv;
 ($wzdark, $closing, $lmwarn, $panic, $closed) = readBool $adv, 5;
 # Doesn't Perl 6 have some built-in way to initialize an array of known length
 # with the return values from calling a function repeatedly?  I thought I saw
 # something like that somewhere in the Synopses.
 @prop[$_] = readInt $adv for ^@prop;
 @abb[$_] = readInt $adv for ^@abb;
 @hintlc[$_] = readInt $adv for ^@hintlc;
 @hinted = readBool $adv, +@hinted;
 @dloc[$_] = readInt $adv for ^@dloc;
 @odloc[$_] = readInt $adv for ^@odloc;
 @dseen = readBool $adv, +@dseen;
 $dflag = readInt $adv;
 $dkill = readInt $adv;
 @place[$_] = readInt $adv for ^@place;
 @fixed[$_] = readInt $adv for ^@fixed;
 for ^@atloc -> $i {
  my int $qty = readInt $adv;
  @atloc[$i;$_] = readInt $adv for ^$qty;
 }
 $saved = readInt($adv);
 $savet = readInt($adv);
 $adv.close;
 start;
 domove NULL;
 return True;
}


# Here be data sections

=begin adventData01
1	You are standing at the end of a road before a small brick building.
1	Around you is a forest.  A small stream flows out of the building and
1	down a gully.
2	You have walked up a hill, still in the forest.  The road slopes back
2	down the other side of the hill.  There is a building in the distance.
3	You are inside a building, a well house for a large spring.
4	You are in a valley in the forest beside a stream tumbling along a
4	rocky bed.
5	You are in open forest, with a deep valley to one side.
6	You are in open forest near both a valley and a road.
7	At your feet all the water of the stream splashes into a 2-inch slit
7	in the rock.  Downstream the streambed is bare rock.
8	You are in a 20-foot depression floored with bare dirt.  Set into the
8	dirt is a strong steel grate mounted in concrete.  A dry streambed
8	leads into the depression.
9	You are in a small chamber beneath a 3x3 steel grate to the surface.
9	A low crawl over cobbles leads inward to the west.
10	You are crawling over cobbles in a low passage.  There is a dim light
10	at the east end of the passage.
11	You are in a debris room filled with stuff washed in from the surface.
11	A low wide passage with cobbles becomes plugged with mud and debris
11	here, but an awkward canyon leads upward and west.  A note on the wall
11	says "MAGIC WORD XYZZY".
12	You are in an awkward sloping east/west canyon.
13	You are in a splendid chamber thirty feet high.  The walls are frozen
13	rivers of orange stone.  An awkward canyon and a good passage exit
13	from east and west sides of the chamber.
14	At your feet is a small pit breathing traces of white mist.  An east
14	passage ends here except for a small crack leading on.
15	You are at one end of a vast hall stretching forward out of sight to
15	the west.  There are openings to either side.  Nearby, a wide stone
15	staircase leads downward.  The hall is filled with wisps of white mist
15	swaying to and fro almost as if alive.  A cold wind blows up the
15	staircase.  There is a passage at the top of a dome behind you.
16	The crack is far too small for you to follow.
17	You are on the east bank of a fissure slicing clear across the hall.
17	The mist is quite thick here, and the fissure is too wide to jump.
18	This is a low room with a crude note on the wall.  The note says,
18	"You won't get it up the steps".
19	You are in the Hall of the Mountain King, with passages off in all
19	directions.
20	You are at the bottom of the pit with a broken neck.
21	You didn't make it.
22	The dome is unclimbable.
23	You are at the west end of the Twopit Room.  There is a large hole in
23	the wall above the pit at this end of the room.
24	You are at the bottom of the eastern pit in the Twopit Room.  There is
24	a small pool of oil in one corner of the pit.
25	You are at the bottom of the western pit in the Twopit Room.  There is
25	a large hole in the wall about 25 feet above you.
26	You clamber up the plant and scurry through the hole at the top.
27	You are on the west side of the fissure in the Hall of Mists.
28	You are in a low n/s passage at a hole in the floor.  The hole goes
28	down to an e/w passage.
29	You are in the south side chamber.
30	You are in the west side chamber of the Hall of the Mountain King.
30	A passage continues west and up here.
31	>$<
32	You can't get by the snake.
33	You are in a large room, with a passage to the south, a passage to the
33	west, and a wall of broken rock to the east.  There is a large "Y2" on
33	a rock in the room's center.
34	You are in a jumble of rock, with cracks everywhere.
35	You're at a low window overlooking a huge pit, which extends up out of
35	sight.  A floor is indistinctly visible over 50 feet below.  Traces of
35	white mist cover the floor of the pit, becoming thicker to the right.
35	Marks in the dust around the window would seem to indicate that
35	someone has been here recently.  Directly across the pit from you and
35	25 feet away there is a similar window looking into a lighted room.  A
35	shadowy figure can be seen there peering back at you.
36	You are in a dirty broken passage.  To the east is a crawl.  To the
36	west is a large passage.  Above you is a hole to another passage.
37	You are on the brink of a small clean climbable pit.  A crawl leads
37	west.
38	You are in the bottom of a small pit with a little stream, which
38	enters and exits through tiny slits.
39	You are in a large room full of dusty rocks.  There is a big hole in
39	the floor.  There are cracks everywhere, and a passage leading east.
40	You have crawled through a very low wide passage parallel to and north
40	of the Hall of Mists.
41	You are at the west end of Hall of Mists.  A low wide crawl continues
41	west and another goes north.  To the south is a little passage 6 feet
41	off the floor.
42	You are in a maze of twisty little passages, all alike.
43	You are in a maze of twisty little passages, all alike.
44	You are in a maze of twisty little passages, all alike.
45	You are in a maze of twisty little passages, all alike.
46	Dead end
47	Dead end
48	Dead end
49	You are in a maze of twisty little passages, all alike.
50	You are in a maze of twisty little passages, all alike.
51	You are in a maze of twisty little passages, all alike.
52	You are in a maze of twisty little passages, all alike.
53	You are in a maze of twisty little passages, all alike.
54	Dead end
55	You are in a maze of twisty little passages, all alike.
56	Dead end
57	You are on the brink of a thirty foot pit with a massive orange column
57	down one wall.  You could climb down here but you could not get back
57	up.  The maze continues at this level.
58	Dead end
59	You have crawled through a very low wide passage parallel to and north
59	of the Hall of Mists.
60	You are at the east end of a very long hall apparently without side
60	chambers.  To the east a low wide crawl slants up.  To the north a
60	round two foot hole slants down.
61	You are at the west end of a very long featureless hall.  The hall
61	joins up with a narrow north/south passage.
62	You are at a crossover of a high n/s passage and a low e/w one.
63	Dead end
64	You are at a complex junction.  A low hands and knees passage from the
64	north joins a higher crawl from the east to make a walking passage
64	going west.  There is also a large room above.  The air is damp here.
65	You are in Bedquilt, a long east/west passage with holes everywhere.
65	To explore at random select north, south, up, or down.
66	You are in a room whose walls resemble swiss cheese.  Obvious passages
66	go west, east, ne, and nw.  Part of the room is occupied by a large
66	bedrock block.
67	You are at the east end of the Twopit Room.  The floor here is
67	littered with thin rock slabs, which make it easy to descend the pits.
67	There is a path here bypassing the pits to connect passages from east
67	and west.  There are holes all over, but the only big one is on the
67	wall directly over the west pit where you can't get to it.
68	You are in a large low circular chamber whose floor is an immense slab
68	fallen from the ceiling (Slab Room).  East and west there once were
68	large passages, but they are now filled with boulders.  Low small
68	passages go north and south, and the south one quickly bends west
68	around the boulders.
69	You are in a secret n/s canyon above a large room.
70	You are in a secret n/s canyon above a sizable passage.
71	You are in a secret canyon at a junction of three canyons, bearing
71	north, south, and se.  The north one is as tall as the other two
71	combined.
72	You are in a large low room.  Crawls lead north, se, and sw.
73	Dead end crawl.
74	You are in a secret canyon which here runs e/w.  It crosses over a
74	very tight canyon 15 feet below.  If you go down you may not be able
74	to get back up.
75	You are at a wide place in a very tight n/s canyon.
76	The canyon here becomes too tight to go further south.
77	You are in a tall e/w canyon.  A low tight crawl goes 3 feet north and
77	seems to open up.
78	The canyon runs into a mass of boulders -- dead end.
79	The stream flows out through a pair of 1 foot diameter sewer pipes.
79	It would be advisable to use the exit.
80	You are in a maze of twisty little passages, all alike.
81	Dead end
82	Dead end
83	You are in a maze of twisty little passages, all alike.
84	You are in a maze of twisty little passages, all alike.
85	Dead end
86	Dead end
87	You are in a maze of twisty little passages, all alike.
88	You are in a long, narrow corridor stretching out of sight to the
88	west.  At the eastern end is a hole through which you can see a
88	profusion of leaves.
89	There is nothing here to climb.  Use "up" or "out" to leave the pit.
90	You have climbed up the plant and out of the pit.
91	You are at the top of a steep incline above a large room.  You could
91	climb down here, but you would not be able to climb up.  There is a
91	passage leading back to the north.
92	You are in the Giant Room.  The ceiling here is too high up for your
92	lamp to show it.  Cavernous passages lead east, north, and south.  On
92	the west wall is scrawled the inscription, "FEE FIE FOE FOO" [sic].
93	The passage here is blocked by a recent cave-in.
94	You are at one end of an immense north/south passage.
95	You are in a magnificent cavern with a rushing stream, which cascades
95	over a sparkling waterfall into a roaring whirlpool which disappears
95	through a hole in the floor.  Passages exit to the south and west.
96	You are in the soft room.  The walls are covered with heavy curtains,
96	the floor with a thick pile carpet.  Moss covers the ceiling.
97	This is the oriental room.  Ancient oriental cave drawings cover the
97	walls.  A gently sloping passage leads upward to the north, another
97	passage leads se, and a hands and knees crawl leads west.
98	You are following a wide path around the outer edge of a large cavern.
98	Far below, through a heavy white mist, strange splashing noises can be
98	heard.  The mist rises up through a fissure in the ceiling.  The path
98	exits to the south and west.
99	You are in an alcove.  A small nw path seems to widen after a short
99	distance.  An extremely tight tunnel leads east.  It looks like a very
99	tight squeeze.  An eerie light can be seen at the other end.
100	You're in a small chamber lit by an eerie green light.  An extremely
100	narrow tunnel exits to the west.  A dark corridor leads ne.
101	You're in the dark-room.  A corridor leading south is the only exit.
102	You are in an arched hall.  A coral passage once continued up and east
102	from here, but is now blocked by debris.  The air smells of sea water.
103	You're in a large room carved out of sedimentary rock.  The floor and
103	walls are littered with bits of shells embedded in the stone.  A
103	shallow passage proceeds downward, and a somewhat steeper one leads
103	up.  A low hands and knees passage enters from the south.
104	You are in a long sloping corridor with ragged sharp walls.
105	You are in a cul-de-sac about eight feet across.
106	You are in an anteroom leading to a large passage to the east.  Small
106	passages go west and up.  The remnants of recent digging are evident.
106	A sign in midair here says "Cave under construction beyond this point.
106	Proceed at own risk.  [Witt Construction Company]"
107	You are in a maze of twisty little passages, all different.
108	You are at Witt's End.  Passages lead off in *ALL* directions.
109	You are in a north/south canyon about 25 feet across.  The floor is
109	covered by white mist seeping in from the north.  The walls extend
109	upward for well over 100 feet.  Suspended from some unseen point far
109	above you, an enormous two-sided mirror is hanging parallel to and
109	midway between the canyon walls.  (The mirror is obviously provided
109	for the use of the dwarves, who as you know, are extremely vain.)  A
109	small window can be seen in either wall, some fifty feet up.
110	You're at a low window overlooking a huge pit, which extends up out of
110	sight.  A floor is indistinctly visible over 50 feet below.  Traces of
110	white mist cover the floor of the pit, becoming thicker to the left.
110	Marks in the dust around the window would seem to indicate that
110	someone has been here recently.  Directly across the pit from you and
110	25 feet away there is a similar window looking into a lighted room.  A
110	shadowy figure can be seen there peering back at you.
111	A large stalactite extends from the roof and almost reaches the floor
111	below.  You could climb down it, and jump from it to the floor, but
111	having done so you would be unable to reach it to climb back up.
112	You are in a little maze of twisting passages, all different.
113	You are at the edge of a large underground reservoir.  An opaque cloud
113	of white mist fills the room and rises rapidly upward.  The lake is
113	fed by a stream, which tumbles out of a hole in the wall about 10 feet
113	overhead and splashes noisily into the water somewhere within the
113	mist.  The only passage goes back toward the south.
114	Dead end
115	You are at the northeast end of an immense room, even larger than the
115	Giant Room.  It appears to be a repository for the "Adventure"
115	program.  Massive torches far overhead bathe the room with smoky
115	yellow light.  Scattered about you can be seen a pile of bottles (all
115	of them empty), a nursery of young beanstalks murmuring quietly, a bed
115	of oysters, a bundle of black rods with rusty stars on their ends, and
115	a collection of brass lanterns.  Off to one side a great many dwarves
115	are sleeping on the floor, snoring loudly.  A sign nearby reads: "Do
115	not disturb the dwarves!"  An immense mirror is hanging against one
115	wall, and stretches to the other end of the room, where various other
115	sundry objects can be glimpsed dimly in the distance.
116	You are at the southwest end of the repository.  To one side is a pit
116	full of fierce green snakes.  On the other side is a row of small
116	wicker cages, each of which contains a little sulking bird.  In one
116	corner is a bundle of black rods with rusty marks on their ends.  A
116	large number of velvet pillows are scattered about on the floor.  A
116	vast mirror stretches off to the northeast.  At your feet is a large
116	steel grate, next to which is a sign which reads, "Treasure vault.
116	Keys in main office."
117	You are on one side of a large, deep chasm.  A heavy white mist rising
117	up from below obscures all view of the far side.  A sw path leads away
117	from the chasm into a winding corridor.
118	You are in a long winding corridor sloping out of sight in both
118	directions.
119	You are in a secret canyon which exits to the north and east.
120	You are in a secret canyon which exits to the north and east.
121	You are in a secret canyon which exits to the north and east.
122	You are on the far side of the chasm.  A ne path leads away from the
122	chasm on this side.
123	You're in a long east/west corridor.  A faint rumbling noise can be
123	heard in the distance.
124	The path forks here.  The left fork leads northeast.  A dull rumbling
124	seems to get louder in that direction.  The right fork leads southeast
124	down a gentle slope.  The main corridor enters from the west.
125	The walls are quite warm here.  From the north can be heard a steady
125	roar, so loud that the entire cave seems to be trembling.  Another
125	passage leads south, and a low crawl goes east.
126	You are on the edge of a breath-taking view.  Far below you is an
126	active volcano, from which great gouts of molten lava come surging
126	out, cascading back down into the depths.  The glowing rock fills the
126	farthest reaches of the cavern with a blood-red glare, giving every-
126	thing an eerie, macabre appearance.  The air is filled with flickering
126	sparks of ash and a heavy smell of brimstone.  The walls are hot to
126	the touch, and the thundering of the volcano drowns out all other
126	sounds.  Embedded in the jagged roof far overhead are myriad twisted
126	formations composed of pure white alabaster, which scatter the murky
126	light into sinister apparitions upon the walls.  To one side is a deep
126	gorge, filled with a bizarre chaos of tortured rock which seems to
126	have been crafted by the devil himself.  An immense river of fire
126	crashes out from the depths of the volcano, burns its way through the
126	gorge, and plummets into a bottomless pit far off to your left.  To
126	the right, an immense geyser of blistering steam erupts continuously
126	from a barren island in the center of a sulfurous lake, which bubbles
126	ominously.  The far right wall is aflame with an incandescence of its
126	own, which lends an additional infernal splendor to the already
126	hellish scene.  A dark, foreboding passage exits to the south.
127	You are in a small chamber filled with large boulders.  The walls are
127	very warm, causing the air in the room to be almost stifling from the
127	heat.  The only exit is a crawl heading west, through which is coming
127	a low rumbling.
128	You are walking along a gently sloping north/south passage lined with
128	oddly shaped limestone formations.
129	You are standing at the entrance to a large, barren room.  A sign
129	posted above the entrance reads:  "Caution!  Bear in room!"
130	You are inside a barren room.  The center of the room is completely
130	empty except for some dust.  Marks in the dust lead away toward the
130	far end of the room.  The only exit is the way you came in.
131	You are in a maze of twisting little passages, all different.
132	You are in a little maze of twisty passages, all different.
133	You are in a twisting maze of little passages, all different.
134	You are in a twisting little maze of passages, all different.
135	You are in a twisty little maze of passages, all different.
136	You are in a twisty maze of little passages, all different.
137	You are in a little twisty maze of passages, all different.
138	You are in a maze of little twisting passages, all different.
139	You are in a maze of little twisty passages, all different.
140	Dead end
=end adventData01

=begin adventData02
1	You're at end of road again.
2	You're at hill in road.
3	You're inside building.
4	You're in valley.
5	You're in forest.
6	You're in forest.
7	You're at slit in streambed.
8	You're outside grate.
9	You're below the grate.
10	You're in cobble crawl.
11	You're in debris room.
13	You're in bird chamber.
14	You're at top of small pit.
15	You're in Hall of Mists.
17	You're on east bank of fissure.
18	You're in nugget of gold room.
19	You're in Hall of Mt King.
23	You're at west end of Twopit Room.
24	You're in east pit.
25	You're in west pit.
33	You're at "Y2".
35	You're at window on pit.
36	You're in dirty passage.
39	You're in dusty rock room.
41	You're at west end of Hall of Mists.
57	You're at brink of pit.
60	You're at east end of long hall.
61	You're at west end of long hall.
64	You're at complex junction.
66	You're in Swiss Cheese Room.
67	You're at east end of Twopit Room.
68	You're in Slab Room.
71	You're at junction of three secret canyons.
74	You're in secret e/w canyon above tight canyon.
88	You're in narrow corridor.
91	You're at steep incline above large room.
92	You're in Giant Room.
95	You're in cavern with waterfall.
96	You're in Soft Room.
97	You're in Oriental Room.
98	You're in misty cavern.
99	You're in alcove.
100	You're in Plover Room.
101	You're in dark-room.
102	You're in arched hall.
103	You're in Shell Room.
106	You're in anteroom.
108	You're at Witt's End.
109	You're in Mirror Canyon.
110	You're at window on pit.
111	You're at top of stalactite.
113	You're at reservoir.
115	You're at ne end.
116	You're at sw end.
117	You're on sw side of chasm.
118	You're in sloping corridor.
122	You're on ne side of chasm.
123	You're in corridor.
124	You're at fork in path.
125	You're at junction with warm walls.
126	You're at breath-taking view.
127	You're in Chamber of Boulders.
128	You're in limestone passage.
129	You're in front of Barren Room.
130	You're in Barren Room.
=end adventData02

=begin adventData03
1	2	2	44	29
1	3	3	12	19	43
1	4	5	13	14	46	30
1	5	6	45	43
1	8	63
2	1	2	12	7	43	45	30
2	5	6	45	46
3	1	3	11	32	44
3	11	62
3	33	65
3	79	5	14
4	1	4	12	45
4	5	6	43	44	29
4	7	5	46	30
4	8	63
5	4	9	43	30
5	50005	6	7	45
5	6	6
5	5	44	46
6	1	2	45
6	4	9	43	44	30
6	5	6	46
7	1	12
7	4	4	45
7	5	6	43	44
7	8	5	15	16	46
7	595	60	14	30
8	5	6	43	44	46
8	1	12
8	7	4	13	45
8	303009	3	19	30
8	593	3
9	303008	11	29
9	593	11
9	10	17	18	19	44
9	14	31
9	11	51
10	9	11	20	21	43
10	11	19	22	44	51
10	14	31
11	303008	63
11	9	64
11	10	17	18	23	24	43
11	12	25	19	29	44
11	3	62
11	14	31
12	303008	63
12	9	64
12	11	30	43	51
12	13	19	29	44
12	14	31
13	303008	63
13	9	64
13	11	51
13	12	25	43
13	14	23	31	44
14	303008	63
14	9	64
14	11	51
14	13	23	43
14	150020	30	31	34
14	15	30
14	16	33	44
15	18	36	46
15	17	7	38	44
15	19	10	30	45
15	150022	29	31	34	35	23	43
15	14	29
15	34	55
16	14	1
17	15	38	43
17	312596	39
17	412021	7
17	412597	41	42	44	69
17	27	41
18	15	38	11	45
19	15	10	29	43
19	311028	45	36
19	311029	46	37
19	311030	44	7
19	32	45
19	35074	49
19	211032	49
19	74	66
20	0	1
21	0	1
22	15	1
23	67	43	42
23	68	44	61
23	25	30	31
23	648	52
24	67	29	11
25	23	29	11
25	724031	56
25	26	56
26	88	1
27	312596	39
27	412021	7
27	412597	41	42	43	69
27	17	41
27	40	45
27	41	44
28	19	38	11	46
28	33	45	55
28	36	30	52
29	19	38	11	45
30	19	38	11	43
30	62	44	29
31	524089	1
31	90	1
32	19	1
33	3	65
33	28	46
33	34	43	53	54
33	35	44
33	159302	71
33	100	71
34	33	30	55
34	15	29
35	33	43	55
35	20	39
36	37	43	17
36	28	29	52
36	39	44
36	65	70
37	36	44	17
37	38	30	31	56
38	37	56	29	11
38	595	60	14	30	4	5
39	36	43	23
39	64	30	52	58
39	65	70
40	41	1
41	42	46	29	23	56
41	27	43
41	59	45
41	60	44	17
42	41	29
42	42	45
42	43	43
42	45	46
42	80	44
43	42	44
43	44	46
43	45	43
44	43	43
44	48	30
44	50	46
44	82	45
45	42	44
45	43	45
45	46	43
45	47	46
45	87	29	30
46	45	44	11
47	45	43	11
48	44	29	11
49	50	43
49	51	44
50	44	43
50	49	44
50	51	30
50	52	46
51	49	44
51	50	29
51	52	43
51	53	46
52	50	44
52	51	43
52	52	46
52	53	29
52	55	45
52	86	30
53	51	44
53	52	45
53	54	46
54	53	44	11
55	52	44
55	55	45
55	56	30
55	57	43
56	55	29	11
57	13	30	56
57	55	44
57	58	46
57	83	45
57	84	43
58	57	43	11
59	27	1
60	41	43	29	17
60	61	44
60	62	45	30	52
61	60	43
61	62	45
61	100107	46
62	60	44
62	63	45
62	30	43
62	61	46
63	62	46	11
64	39	29	56	59
64	65	44	70
64	103	45	74
64	106	43
65	64	43
65	66	44
65	80556	46
65	68	61
65	80556	29
65	50070	29
65	39	29
65	60556	45
65	75072	45
65	71	45
65	80556	30
65	106	30
66	65	47
66	67	44
66	80556	46
66	77	25
66	96	43
66	50556	50
66	97	72
67	66	43
67	23	44	42
67	24	30	31
68	23	46
68	69	29	56
68	65	45
69	68	30	61
69	331120	46
69	119	46
69	109	45
69	113	75
70	71	45
70	65	30	23
70	111	46
71	65	48
71	70	46
71	110	45
72	65	70
72	118	49
72	73	45
72	97	48	72
73	72	46	17	11
74	19	43
74	331120	44
74	121	44
74	75	30
75	76	46
75	77	45
76	75	45
77	75	43
77	78	44
77	66	45	17
78	77	46
79	3	1
80	42	45
80	80	44
80	80	46
80	81	43
81	80	44	11
82	44	46	11
83	57	46
83	84	43
83	85	44
84	57	45
84	83	44
84	114	50
85	83	43	11
86	52	29	11
87	45	29	30
88	25	30	56	43
88	20	39
88	92	44	27
89	25	1
90	23	1
91	95	45	73	23
91	72	30	56
92	88	46
92	93	43
92	94	45
93	92	46	27	11
94	92	46	27	23
94	309095	45	3	73
94	611	45
95	94	46	11
95	92	27
95	91	44
96	66	44	11
97	66	48
97	72	44	17
97	98	29	45	73
98	97	46	72
98	99	44
99	98	50	73
99	301	43	23
99	100	43
100	301	44	23	11
100	99	44
100	159302	71
100	33	71
100	101	47	22
101	100	46	71	11
102	103	30	74	11
103	102	29	38
103	104	30
103	114618	46
103	115619	46
103	64	46
104	103	29	74
104	105	30
105	104	29	11
105	103	74
106	64	29
106	65	44
106	108	43
107	131	46
107	132	49
107	133	47
107	134	48
107	135	29
107	136	50
107	137	43
107	138	44
107	139	45
107	61	30
108	95556	43	45	46	47	48	49	50	29	30
108	106	43
108	626	44
109	69	46
109	113	45	75
110	71	44
110	20	39
111	70	45
111	40050	30	39	56
111	50053	30
111	45	30
112	131	49
112	132	45
112	133	43
112	134	50
112	135	48
112	136	47
112	137	44
112	138	30
112	139	29
112	140	46
113	109	46	11	109
114	84	48
115	116	49
116	115	47
116	593	30
117	118	49
117	233660	41	42	69	47
117	332661	41
117	303	41
117	332021	39
117	596	39
118	72	30
118	117	29
119	69	45	11
119	653	43	7
120	69	45
120	74	43
121	74	43	11
121	653	45	7
122	123	47
122	233660	41	42	69	49
122	303	41
122	596	39
122	124	77
122	126	28
122	129	40
123	122	44
123	124	43	77
123	126	28
123	129	40
124	123	44
124	125	47	36
124	128	48	37	30
124	126	28
124	129	40
125	124	46	77
125	126	45	28
125	127	43	17
126	125	46	23	11
126	124	77
126	610	30	39
127	125	44	11	17
127	124	77
127	126	28
128	124	45	29	77
128	129	46	30	40
128	126	28
129	128	44	29
129	124	77
129	130	43	19	40	3
129	126	28
130	129	44	11
130	124	77
130	126	28
131	107	44
131	132	48
131	133	50
131	134	49
131	135	47
131	136	29
131	137	30
131	138	45
131	139	46
131	112	43
132	107	50
132	131	29
132	133	45
132	134	46
132	135	44
132	136	49
132	137	47
132	138	43
132	139	30
132	112	48
133	107	29
133	131	30
133	132	44
133	134	47
133	135	49
133	136	43
133	137	45
133	138	50
133	139	48
133	112	46
134	107	47
134	131	45
134	132	50
134	133	48
134	135	43
134	136	30
134	137	46
134	138	29
134	139	44
134	112	49
135	107	45
135	131	48
135	132	30
135	133	46
135	134	43
135	136	44
135	137	49
135	138	47
135	139	50
135	112	29
136	107	43
136	131	44
136	132	29
136	133	49
136	134	30
136	135	46
136	137	50
136	138	48
136	139	47
136	112	45
137	107	48
137	131	47
137	132	46
137	133	30
137	134	29
137	135	50
137	136	45
137	138	49
137	139	43
137	112	44
138	107	30
138	131	43
138	132	47
138	133	29
138	134	44
138	135	45
138	136	46
138	137	48
138	139	49
138	112	50
139	107	49
139	131	50
139	132	43
139	133	44
139	134	45
139	135	30
139	136	48
139	137	29
139	138	46
139	112	47
140	112	45	11
=end adventData03

=begin adventData04
2	ROAD
2	HILL
3	ENTER
4	UPSTR
5	DOWNS
6	FORES
7	FORWA
7	CONTI
7	ONWAR
8	BACK
8	RETUR
8	RETRE
9	VALLE
10	STAIR
11	OUT
11	OUTSI
11	EXIT
11	LEAVE
12	BUILD
12	HOUSE
13	GULLY
14	STREA
15	ROCK
16	BED
17	CRAWL
18	COBBL
19	INWAR
19	INSID
19	IN
20	SURFA
21	NULL
21	NOWHE
22	DARK
23	PASSA
23	TUNNE
24	LOW
25	CANYO
26	AWKWA
27	GIANT
28	VIEW
29	UPWAR
29	UP
29	U
29	ABOVE
29	ASCEN
30	D
30	DOWNW
30	DOWN
30	DESCE
31	PIT
32	OUTDO
33	CRACK
34	STEPS
35	DOME
36	LEFT
37	RIGHT
38	HALL
39	JUMP
40	BARRE
41	OVER
42	ACROS
43	EAST
43	E
44	WEST
44	W
45	NORTH
45	N
46	SOUTH
46	S
47	NE
48	SE
49	SW
50	NW
51	DEBRI
52	HOLE
53	WALL
54	BROKE
55	Y2
56	CLIMB
57	LOOK
57	EXAMI
57	TOUCH
57	DESCR
58	FLOOR
59	ROOM
60	SLIT
61	SLAB
61	SLABR
62	XYZZY
63	DEPRE
64	ENTRA
65	PLUGH
66	SECRE
67	CAVE
69	CROSS
70	BEDQU
71	PLOVE
72	ORIEN
73	CAVER
74	SHELL
75	RESER
76	MAIN
76	OFFIC
77	FORK
1001	KEYS
1001	KEY
1002	LAMP
1002	HEADL
1002	LANTE
1003	GRATE
1004	CAGE
1005	ROD
1006	ROD	(MUST BE NEXT OBJECT AFTER "REAL" ROD)
1007	STEPS
1008	BIRD
1009	DOOR
1010	PILLO
1010	VELVE
1011	SNAKE
1012	FISSU
1013	TABLE
1014	CLAM
1015	OYSTE
1016	MAGAZ
1016	ISSUE
1016	SPELU
1016	"SPEL
1017	DWARF
1017	DWARV
1018	KNIFE
1018	KNIVE
1019	FOOD
1019	RATIO
1020	BOTTL
1020	JAR
1021	WATER
1021	H2O
1022	OIL
1023	MIRRO
1024	PLANT
1024	BEANS
1025	PLANT	(MUST BE NEXT OBJECT AFTER "REAL" PLANT)
1026	STALA
1027	SHADO
1027	FIGUR
1028	AXE
1029	DRAWI
1030	PIRAT
1031	DRAGO
1032	CHASM
1033	TROLL
1034	TROLL	(MUST BE NEXT OBJECT AFTER "REAL" TROLL)
1035	BEAR
1036	MESSA
1037	VOLCA
1037	GEYSE	(SAME AS VOLCANO)
1038	MACHI
1038	VENDI
1039	BATTE
1040	CARPE
1040	MOSS
1050	GOLD
1050	NUGGE
1051	DIAMO
1052	SILVE
1052	BARS
1053	JEWEL
1054	COINS
1055	CHEST
1055	BOX
1055	TREAS
1056	EGGS
1056	EGG
1056	NEST
1057	TRIDE
1058	VASE
1058	MING
1058	SHARD
1058	POTTE
1059	EMERA
1060	PLATI
1060	PYRAM
1061	PEARL
1062	RUG
1062	PERSI
1063	SPICE
1064	CHAIN
2001	CARRY
2001	TAKE
2001	KEEP
2001	CATCH
2001	STEAL
2001	CAPTU
2001	GET
2001	TOTE
2002	DROP
2002	RELEA
2002	FREE
2002	DISCA
2002	DUMP
2003	SAY
2003	CHANT
2003	SING
2003	UTTER
2003	MUMBL
2004	UNLOC
2004	OPEN
2005	NOTHI
2006	LOCK
2006	CLOSE
2007	LIGHT
2007	ON
2008	EXTIN
2008	OFF
2009	WAVE
2009	SHAKE
2009	SWING
2010	CALM
2010	PLACA
2010	TAME
2011	WALK
2011	RUN
2011	TRAVE
2011	GO
2011	PROCE
2011	CONTI
2011	EXPLO
2011	GOTO
2011	FOLLO
2011	TURN
2012	ATTAC
2012	KILL
2012	FIGHT
2012	HIT
2012	STRIK
2013	POUR
2014	EAT
2014	DEVOU
2015	DRINK
2016	RUB
2017	THROW
2017	TOSS
2018	QUIT
2019	FIND
2019	WHERE
2020	INVEN
2021	FEED
2022	FILL
2023	BLAST
2023	DETON
2023	IGNIT
2023	BLOWU
2024	SCORE
2025	FEE
2025	FIE
2025	FOE
2025	FOO
2025	FUM
2026	BRIEF
2027	READ
2027	PERUS
2028	BREAK
2028	SHATT
2028	SMASH
2029	WAKE
2029	DISTU
2030	SUSPE
2030	PAUSE
2030	SAVE
2031	HOURS
2032	RESUM
2032	RESTA
2032	RESTO
2032	LOAD
3001	FEE
3002	FIE
3003	FOE
3004	FOO
3005	FUM
3050	SESAM
3050	OPENS
3050	ABRA
3050	ABRAC
3050	SHAZA
3050	HOCUS
3050	POCUS
3051	HELP
3051	?
3064	TREE
3064	TREES
3066	DIG
3066	EXCAV
3068	LOST
3069	MIST
3079	FUCK
3139	STOP
3142	INFO
3142	INFOR
3147	SWIM
=end adventData04

=begin adventData05
1	Set of keys
000	There are some keys on the ground here.
2	Brass lantern
000	There is a shiny brass lamp nearby.
100	There is a lamp shining nearby.
3	*grate
000	The grate is locked.
100	The grate is open.
4	Wicker cage
000	There is a small wicker cage discarded nearby.
5	Black rod
000	A three foot black rod with a rusty star on an end lies nearby.
6	Black rod
000	A three foot black rod with a rusty mark on an end lies nearby.
7	*steps
000	Rough stone steps lead down the pit.
100	Rough stone steps lead up the dome.
8	Little bird in cage
000	A cheerful little bird is sitting here singing.
100	There is a little bird in the cage.
9	*rusty door
000	The way north is barred by a massive, rusty, iron door.
100	The way north leads through a massive, rusty, iron door.
10	Velvet pillow
000	A small velvet pillow lies on the floor.
11	*snake
000	A huge green fierce snake bars the way!
100	>$<  (chased away)
12	*fissure
000	>$<
100	A crystal bridge now spans the fissure.
200	The crystal bridge has vanished!
13	*stone tablet
000	A massive stone tablet embedded in the wall reads:
000	"Congratulations on bringing light into the dark-room!"
14	Giant clam  >GRUNT!<
000	There is an enormous clam here with its shell tightly closed.
15	Giant oyster  >GROAN!<
000	There is an enormous oyster here with its shell tightly closed.
100	Interesting.  There seems to be something written on the underside of
100	the oyster.
16	"Spelunker Today"
000	There are a few recent issues of "Spelunker Today" magazine here.
19	Tasty food
000	There is food here.
20	Small bottle
000	There is a bottle of water here.
100	There is an empty bottle here.
200	There is a bottle of oil here.
21	Water in the bottle
22	Oil in the bottle
23	*mirror
000	>$<
24	*plant
000	There is a tiny little plant in the pit, murmuring "water, water, ..."
100	The plant spurts into furious growth for a few seconds.
200	There is a 12-foot-tall beanstalk stretching up out of the pit,
200	bellowing "WATER!! WATER!!"
300	The plant grows explosively, almost filling the bottom of the pit.
400	There is a gigantic beanstalk stretching all the way up to the hole.
500	You've over-watered the plant!  It's shriveling up!  It's, it's...
25	*phony plant (seen in Twopit Room only when tall enough)
000	>$<
100	The top of a 12-foot-tall beanstalk is poking out of the west pit.
200	There is a huge beanstalk growing out of the west pit up to the hole.
26	*stalactite
000	>$<
27	*shadowy figure
000	The shadowy figure seems to be trying to attract your attention.
28	Dwarf's axe
000	There is a little axe here.
100	There is a little axe lying beside the bear.
29	*cave drawings
000	>$<
30	*pirate
000	>$<
31	*dragon
000	A huge green fierce dragon bars the way!
100	Congratulations!  You have just vanquished a dragon with your bare
100	hands!  (Unbelievable, isn't it?)
200	The body of a huge green dead dragon is lying off to one side.
32	*chasm
000	A rickety wooden bridge extends across the chasm, vanishing into the
000	mist.  A sign posted on the bridge reads, "Stop! Pay troll!"
100	The wreckage of a bridge (and a dead bear) can be seen at the bottom
100	of the chasm.
33	*troll
000	A burly troll stands by the bridge and insists you throw him a
000	treasure before you may cross.
100	The troll steps out from beneath the bridge and blocks your way.
200	>$<  (chased away)
34	*phony troll
000	The troll is nowhere to be seen.
35	>$<  (bear uses rtext 141)
000	There is a ferocious cave bear eying you from the far end of the room!
100	There is a gentle cave bear sitting placidly in one corner.
200	There is a contented-looking bear wandering about nearby.
300	>$<  (dead)
36	*message in second maze
000	There is a message scrawled in the dust in a flowery script, reading:
000	"This is not the maze where the pirate leaves his treasure chest."
37	*volcano and/or geyser
000	>$<
38	*vending machine
000	There is a massive vending machine here.  The instructions on it read:
000	"Drop coins here to receive fresh batteries."
39	Batteries
000	There are fresh batteries here.
100	Some worn-out batteries have been discarded nearby.
40	*carpet and/or moss
000	>$<
50	Large gold nugget
000	There is a large sparkling nugget of gold here!
51	Several diamonds
000	There are diamonds here!
52	Bars of silver
000	There are bars of silver here!
53	Precious jewelry
000	There is precious jewelry here!
54	Rare coins
000	There are many coins here!
55	Treasure chest
000	The pirate's treasure chest is here!
56	Golden eggs
000	There is a large nest here, full of golden eggs!
100	The nest of golden eggs has vanished!
200	Done!
57	Jeweled trident
000	There is a jewel-encrusted trident here!
58	Ming vase
000	There is a delicate, precious, ming vase here!
100	The vase is now resting, delicately, on a velvet pillow.
200	The floor is littered with worthless shards of pottery.
300	The ming vase drops with a delicate crash.
59	Egg-sized emerald
000	There is an emerald here the size of a plover's egg!
60	Platinum pyramid
000	There is a platinum pyramid here, 8 inches on a side!
61	Glistening pearl
000	Off to one side lies a glistening pearl!
62	Persian rug
000	There is a Persian rug spread out on the floor!
100	The dragon is sprawled out on a Persian rug!!
63	Rare spices
000	There are rare spices here!
64	Golden chain
000	There is a golden chain lying in a heap on the floor!
100	The bear is locked to the wall with a golden chain!
200	There is a golden chain locked to the wall!
=end adventData05

=begin adventData06
1	Somewhere nearby is Colossal Cave, where others have found fortunes in
1	treasure and gold, though it is rumored that some who enter are never
1	seen again.  Magic is said to work in the cave.  I will be your eyes
1	and hands.  Direct me with commands of 1 or 2 words.  I should warn
1	you that I look at only the first five letters of each word, so you'll
1	have to enter "northeast" as "ne" to distinguish it from "north".
1	(Should you get stuck, type "help" for some general hints.  For infor-
1	mation on how to end your adventure, etc., type "info".)
1				      - - -
1	This program was originally developed by Willie Crowther.  Most of the
1	features of the current program were added by Don Woods (don @ su-ai).
1	Contact Don if you have any questions, comments, etc.
2	A little dwarf with a big knife blocks your way.
3	A little dwarf just walked around a corner, saw you, threw a little
3	axe at you which missed, cursed, and ran away.
4	There is a threatening little dwarf in the room with you!
5	One sharp nasty knife is thrown at you!
6	None of them hit you!
7	One of them gets you!
8	A hollow voice says "PLUGH".
9	There is no way to go that direction.
10	I am unsure how you are facing.  Use compass points or nearby objects.
11	I don't know in from out here.  Use compass points or name something
11	in the general direction you want to go.
12	I don't know how to apply that word here.
13	I don't understand that!
14	I'm game.  Would you care to explain how?
15	Sorry, but I am not allowed to give more detail.  I will repeat the
15	long description of your location.
16	It is now pitch dark.  If you proceed you will likely fall into a pit.
17	If you prefer, simply type w rather than west.
18	Are you trying to catch the bird?
19	The bird is frightened right now and you cannot catch it no matter
19	what you try.  Perhaps you might try later.
20	Are you trying to somehow deal with the snake?
21	You can't kill the snake, or drive it away, or avoid it, or anything
21	like that.  There is a way to get by, but you don't have the necessary
21	resources right now.
22	Do you really want to quit now?
23	You fell into a pit and broke every bone in your body!
24	You are already carrying it!
25	You can't be serious!
26	The bird was unafraid when you entered, but as you approach it becomes
26	disturbed and you cannot catch it.
27	You can catch the bird, but you cannot carry it.
28	There is nothing here with a lock!
29	You aren't carrying it!
30	The little bird attacks the green snake, and in an astounding flurry
30	drives the snake away.
31	You have no keys!
32	It has no lock.
33	I don't know how to lock or unlock such a thing.
34	It was already locked.
35	The grate is now locked.
36	The grate is now unlocked.
37	It was already unlocked.
38	You have no source of light.
39	Your lamp is now on.
40	Your lamp is now off.
41	There is no way to get past the bear to unlock the chain, which is
41	probably just as well.
42	Nothing happens.
43	Where?
44	There is nothing here to attack.
45	The little bird is now dead.  Its body disappears.
46	Attacking the snake both doesn't work and is very dangerous.
47	You killed a little dwarf.
48	You attack a little dwarf, but he dodges out of the way.
49	With what?  Your bare hands?
50	Good try, but that is an old worn-out magic word.
51	I know of places, actions, and things.  Most of my vocabulary
51	describes places and is used to move you there.  To move, try words
51	like forest, building, downstream, enter, east, west, north, south,
51	up, or down.  I know about a few special objects, like a black rod
51	hidden in the cave.  These objects can be manipulated using some of
51	the action words that I know.  Usually you will need to give both the
51	object and action words (in either order), but sometimes I can infer
51	the object from the verb alone.  Some objects also imply verbs; in
51	particular, "inventory" implies "take inventory", which causes me to
51	give you a list of what you're carrying.  The objects have side
51	effects; for instance, the rod scares the bird.  Usually people having
51	trouble moving just need to try a few more words.  Usually people
51	trying unsuccessfully to manipulate an object are attempting something
51	beyond their (or my!) capabilities and should try a completely
51	different tack.  To speed the game you can sometimes move long
51	distances with a single word.  For example, "building" usually gets
51	you to the building from anywhere above ground except when lost in the
51	forest.  Also, note that cave passages turn a lot, and that leaving a
51	room to the north does not guarantee entering the next from the south.
51	Good luck!
52	It misses!
53	It gets you!
54	OK
55	You can't unlock the keys.
56	You have crawled around in some little holes and wound up back in the
56	main passage.
57	I don't know where the cave is, but hereabouts no stream can run on
57	the surface for long.  I would try the stream.
58	I need more detailed instructions to do that.
59	I can only tell you what you see as you move about and manipulate
59	things.  I cannot tell you where remote things are.
60	I don't know that word.
61	What?
62	Are you trying to get into the cave?
63	The grate is very solid and has a hardened steel lock.  You cannot
63	enter without a key, and there are no keys nearby.  I would recommend
63	looking elsewhere for the keys.
64	The trees of the forest are large hardwood oak and maple, with an
64	occasional grove of pine or spruce.  There is quite a bit of under-
64	growth, largely birch and ash saplings plus nondescript bushes of
64	various sorts.  This time of year visibility is quite restricted by
64	all the leaves, but travel is quite easy if you detour around the
64	spruce and berry bushes.
65	Welcome to Adventure!!  Would you like instructions?
66	Digging without a shovel is quite impractical.  Even with a shovel
66	progress is unlikely.
67	Blasting requires dynamite.
68	I'm as confused as you are.
69	Mist is a white vapor, usually water, seen from time to time in
69	caverns.  It can be found anywhere but is frequently a sign of a deep
69	pit leading down to water.
70	Your feet are now wet.
71	I think I just lost my appetite.
72	Thank you, it was delicious!
73	You have taken a drink from the stream.  The water tastes strongly of
73	minerals, but is not unpleasant.  It is extremely cold.
74	The bottle of water is now empty.
75	Rubbing the electric lamp is not particularly rewarding.  Anyway,
75	nothing exciting happens.
76	Peculiar.  Nothing unexpected happens.
77	Your bottle is empty and the ground is wet.
78	You can't pour that.
79	Watch it!
80	Which way?
81	Oh dear, you seem to have gotten yourself killed.  I might be able to
81	help you out, but I've never really done this before.  Do you want me
81	to try to reincarnate you?
82	All right.  But don't blame me if something goes wr......
82			    --- POOF!! ---
82	You are engulfed in a cloud of orange smoke.  Coughing and gasping,
82	you emerge from the smoke and find....
83	You clumsy oaf, you've done it again!  I don't know how long I can
83	keep this up.  Do you want me to try reincarnating you again?
84	Okay, now where did I put my orange smoke?....  >POOF!<
84	Everything disappears in a dense cloud of orange smoke.
85	Now you've really done it!  I'm out of orange smoke!  You don't expect
85	me to do a decent reincarnation without any orange smoke, do you?
86	Okay, if you're so smart, do it yourself!  I'm leaving!
90	>>> Messages 81 through 90 are reserved for "obituaries." <<<
91	Sorry, but I no longer seem to remember how it was you got here.
92	You can't carry anything more.  You'll have to drop something first.
93	You can't go through a locked steel grate!
94	I believe what you want is right here with you.
95	You don't fit through a two-inch slit!
96	I respectfully suggest you go across the bridge instead of jumping.
97	There is no way across the fissure.
98	You're not carrying anything.
99	You are currently holding the following:
100	It's not hungry (it's merely pinin' for the fjords).  Besides, you
100	have no bird seed.
101	The snake has now devoured your bird.
102	There's nothing here it wants to eat (except perhaps you).
103	You fool, dwarves eat only coal!  Now you've made him *REALLY* mad!!
104	You have nothing in which to carry it.
105	Your bottle is already full.
106	There is nothing here with which to fill the bottle.
107	Your bottle is now full of water.
108	Your bottle is now full of oil.
109	You can't fill that.
110	Don't be ridiculous!
111	The door is extremely rusty and refuses to open.
112	The plant indignantly shakes the oil off its leaves and asks, "Water?"
113	The hinges are quite thoroughly rusted now and won't budge.
114	The oil has freed up the hinges so that the door will now move,
114	although it requires some effort.
115	The plant has exceptionally deep roots and cannot be pulled free.
116	The dwarves' knives vanish as they strike the walls of the cave.
117	Something you're carrying won't fit through the tunnel with you.
117	You'd best take inventory and drop something.
118	You can't fit this five-foot clam through that little passage!
119	You can't fit this five-foot oyster through that little passage!
120	I advise you to put down the clam before opening it.  >STRAIN!<
121	I advise you to put down the oyster before opening it.  >WRENCH!<
122	You don't have anything strong enough to open the clam.
123	You don't have anything strong enough to open the oyster.
124	A glistening pearl falls out of the clam and rolls away.  Goodness,
124	this must really be an oyster.  (I never was very good at identifying
124	bivalves.)  Whatever it is, it has now snapped shut again.
125	The oyster creaks open, revealing nothing but oyster inside.  It
125	promptly snaps shut again.
126	You have crawled around in some little holes and found your way
126	blocked by a recent cave-in.  You are now back in the main passage.
127	There are faint rustling noises from the darkness behind you.
128	Out from the shadows behind you pounces a bearded pirate!  "Har, har,"
128	he chortles, "I'll just take all this booty and hide it away with me
128	chest deep in the maze!"  He snatches your treasure and vanishes into
128	the gloom.
129	A sepulchral voice reverberating through the cave, says, "Cave closing
129	soon.  All adventurers exit immediately through main office."
130	A mysterious recorded voice groans into life and announces:
130	   "This exit is closed.  Please leave via main office."
131	It looks as though you're dead.  Well, seeing as how it's so close to
131	closing time anyway, I think we'll just call it a day.
132	The sepulchral voice intones, "The cave is now closed."  As the echoes
132	fade, there is a blinding flash of light (and a small puff of orange
132	smoke). . . .    As your eyes refocus, you look around and find...
133	There is a loud explosion, and a twenty-foot hole appears in the far
133	wall, burying the dwarves in the rubble.  You march through the hole
133	and find yourself in the main office, where a cheering band of
133	friendly elves carry the conquering adventurer off into the sunset.
134	There is a loud explosion, and a twenty-foot hole appears in the far
134	wall, burying the snakes in the rubble.  A river of molten lava pours
134	in through the hole, destroying everything in its path, including you!
135	There is a loud explosion, and you are suddenly splashed across the
135	walls of the room.
136	The resulting ruckus has awakened the dwarves.  There are now several
136	threatening little dwarves in the room with you!  Most of them throw
136	knives at you!  All of them get you!
137	Oh, leave the poor unhappy bird alone.
138	I daresay whatever you want is around here somewhere.
139	I don't know the word "stop".  Use "quit" if you want to give up.
140	You can't get there from here.
141	You are being followed by a very large, tame bear.
142	If you want to end your adventure early, say "quit".  To suspend your
142	adventure such that you can continue later, say "suspend" (or "pause"
142	or "save").  To see what hours the cave is normally open, say "hours".
142	To see how well you're doing, say "score".  To get full credit for a
142	treasure, you must have left it safely in the building, though you get
142	partial credit just for locating it.  You lose points for getting
142	killed, or for quitting, though the former costs you more.  There are
142	also points based on how much (if any) of the cave you've managed to
142	explore; in particular, there is a large bonus just for getting in (to
142	distinguish the beginners from the rest of the pack), and there are
142	other ways to determine whether you've been through some of the more
142	harrowing sections.  If you think you've found all the treasures, just
142	keep exploring for a while.  If nothing interesting happens, you
142	haven't found them all yet.  If something interesting *DOES* happen,
142	it means you're getting a bonus and have an opportunity to garner many
142	more points in the Master's section.  I may occasionally offer hints
142	if you seem to be having trouble.  If I do, I'll warn you in advance
142	how much it will affect your score to accept the hints.  Finally, to
142	save paper, you may specify "brief", which tells me never to repeat
142	the full description of a place unless you explicitly ask me to.
143	Do you indeed wish to quit now?
144	There is nothing here with which to fill the vase.
145	The sudden change in temperature has delicately shattered the vase.
146	It is beyond your power to do that.
147	I don't know how.
148	It is too far up for you to reach.
149	You killed a little dwarf.  The body vanishes in a cloud of greasy
149	black smoke.
150	The shell is very strong and is impervious to attack.
151	What's the matter, can't you read?  Now you'd best start over.
152	The axe bounces harmlessly off the dragon's thick scales.
153	The dragon looks rather nasty.  You'd best not try to get by.
154	The little bird attacks the green dragon, and in an astounding flurry
154	gets burnt to a cinder.  The ashes blow away.
155	On what?
156	Okay, from now on I'll only describe a place in full the first time
156	you come to it.  To get the full description, say "look".
157	Trolls are close relatives with the rocks and have skin as tough as
157	that of a rhinoceros.  The troll fends off your blows effortlessly.
158	The troll deftly catches the axe, examines it carefully, and tosses it
158	back, declaring, "Good workmanship, but it's not valuable enough."
159	The troll catches your treasure and scurries away out of sight.
160	The troll refuses to let you cross.
161	There is no longer any way across the chasm.
162	Just as you reach the other side, the bridge buckles beneath the
162	weight of the bear, which was still following you around.  You
162	scrabble desperately for support, but as the bridge collapses you
162	stumble back and fall into the chasm.
163	The bear lumbers toward the troll, who lets out a startled shriek and
163	scurries away.  The bear soon gives up the pursuit and wanders back.
164	The axe misses and lands near the bear where you can't get at it.
165	With what?  Your bare hands?  Against *HIS* bear hands??
166	The bear is confused; he only wants to be your friend.
167	For crying out loud, the poor thing is already dead!
168	The bear eagerly wolfs down your food, after which he seems to calm
168	down considerably and even becomes rather friendly.
169	The bear is still chained to the wall.
170	The chain is still locked.
171	The chain is now unlocked.
172	The chain is now locked.
173	There is nothing here to which the chain can be locked.
174	There is nothing here to eat.
175	Do you want the hint?
176	Do you need help getting out of the maze?
177	You can make the passages look less alike by dropping things.
178	Are you trying to explore beyond the plover room?
179	There is a way to explore that region without having to worry about
179	falling into a pit.  None of the objects available is immediately
179	useful in discovering the secret.
180	Do you need help getting out of here?
181	Don't go west.
182	Gluttony is not one of the troll's vices.  Avarice, however, is.
183	Your lamp is getting dim.  You'd best start wrapping this up, unless
183	you can find some fresh batteries.  I seem to recall there's a vending
183	machine in the maze.  Bring some coins with you.
184	Your lamp has run out of power.
185	There's not much point in wandering around out here, and you can't
185	explore the cave without a lamp.  So let's just call it a day.
186	There are faint rustling noises from the darkness behind you.  As you
186	turn toward them, the beam of your lamp falls across a bearded pirate.
186	He is carrying a large chest.  "Shiver me timbers!" he cries, "I've
186	been spotted!  I'd best hie meself off to the maze to hide me chest!"
186	With that, he vanishes into the gloom.
187	Your lamp is getting dim.  You'd best go back for those batteries.
188	Your lamp is getting dim.  I'm taking the liberty of replacing the
188	batteries.
189	Your lamp is getting dim, and you're out of spare batteries.  You'd
189	best start wrapping this up.
190	I'm afraid the magazine is written in dwarvish.
191	"This is not the maze where the pirate leaves his treasure chest."
192	Hmmm, this looks like a clue, which means it'll cost you 10 points to
192	read it.  Should I go ahead and read it anyway?
193	It says, "There is something strange about this place, such that one
193	of the words I've always known now has a new effect."
194	It says the same thing it did before.
195	I'm afraid I don't understand.
196	"Congratulations on bringing light into the dark-room!"
197	You strike the mirror a resounding blow, whereupon it shatters into a
197	myriad of tiny fragments.
198	You have taken the vase and hurled it delicately to the ground.
199	You prod the nearest dwarf, who wakes up grumpily, takes one look at
199	you, curses, and grabs for his axe.
200	Is this acceptable?
201	There's no point in suspending a demonstration game.
=end adventData06

=begin adventData07
1	3
2	3
3	8	9
4	10
5	11
6	0
7	14	15
8	13
9	94	-1
10	96
11	19	-1
12	17	27
13	101	-1
14	103
15	0
16	106
17	0	-1
18	0
19	3
20	3
21	0
22	0
23	109	-1
24	25	-1
25	23	67
26	111	-1
27	35	110
28	0
29	97	-1
30	0	-1
31	119	121
32	117	122
33	117	122
34	0	0
35	130	-1
36	0	-1
37	126	-1
38	140	-1
39	0
40	96	-1
50	18
51	27
52	28
53	29
54	30
55	0
56	92
57	95
58	97
59	100
60	101
61	0
62	119	121
63	127
64	130	-1
=end adventData07

=begin adventData08
1	24
2	29
3	0
4	33
5	0
6	33
7	38
8	38
9	42
10	14
11	43
12	110
13	29
14	110
15	73
16	75
17	29
18	13
19	59
20	59
21	174
22	109
23	67
24	13
25	147
26	155
27	195
28	146
29	110
30	13
31	13
=end adventData08

=begin adventData09
0	1	2	3	4	5	6	7	8	9	10
0	100	115	116	126
2	1	3	4	7	38	95	113	24
1	24
3	46	47	48	54	56	58	82	85	86
3	122	123	124	125	126	127	128	129	130
4	8
5	13
6	19
7	42	43	44	45	46	47	48	49	50	51
7	52	53	54	55	56	80	81	82	86	87
8	99	100	101
9	108
=end adventData09

=begin adventData10
35	You are obviously a rank amateur.  Better luck next time.
100	Your score qualifies you as a novice class adventurer.
130	You have achieved the rating: "Experienced Adventurer".
200	You may now consider yourself a "Seasoned Adventurer".
250	You have reached "Junior Master" status.
300	Your score puts you in Master Adventurer Class C.
330	Your score puts you in Master Adventurer Class B.
349	Your score puts you in Master Adventurer Class A.
9999	All of Adventuredom gives tribute to you, Adventurer Grandmaster!
=end adventData10

=begin adventData11
2	9999	10	0	0
3	9999	5	0	0
4	4	2	62	63
5	5	2	18	19
6	8	2	20	21
7	75	4	176	177
8	25	5	178	179
9	20	3	180	181
=end adventData11

=begin adventData12
1	A large cloud of green smoke appears in front of you.  It clears away
1	to reveal a tall wizard, clothed in grey.  He fixes you with a steely
1	glare and declares, "This adventure has lasted too long."  With that
1	he makes a single pass over you with his hands, and everything around
1	you fades away into a grey nothingness.
2	Even wizards have to wait longer than that!
3	I'm terribly sorry, but Colossal Cave is closed.  Our hours are:
4	Only wizards are permitted within the cave right now.
5	We do allow visitors to make short explorations during our off hours.
5	Would you like to do that?
6	Colossal Cave is open to regular adventurers at the following hours:
7	Very well.
8	Only a wizard may continue an adventure this soon.
9	I suggest you resume your adventure at a later time.
10	Do you wish to see the hours?
11	Do you wish to change the hours?
12	New magic word (null to leave unchanged):
13	New magic number (null to leave unchanged):
14	Do you wish to change the message of the day?
15	Okay.  You can save this version now.
16	Are you a wizard?
17	Prove it!  Say the magic word!
18	That is not what I thought it was.  Do you know what I thought it was?
19	Oh dear, you really *are* a wizard!  Sorry to have bothered you . . .
20	Foo, you are nothing but a charlatan!
21	New hours specified by defining "prime time".  Give only the hour
21	(e.g. 14, not 14:00 or 2pm).  Enter a negative number after last pair.
22	New hours for Colossal Cave:
23	Limit lines to 70 chars.  End with null line.
24	Line too long, retype:
25	Not enough room for another line.  Ending message here.
26	Do you wish to (re)schedule the next holiday?
27	To begin how many days from today?
28	To last how many days (zero if no holiday)?
29	To be called what (up to 20 characters)?
30	Too small!  Assuming minimum value (45 minutes).
31	Break out of this and save your core-image.
32	Be sure to save your core-image...
=end adventData12
