#!/usr/bin/env perl6
use v6;

sub indexLines(Str *@lines --> List of Str) {
 gather for @lines {
  FIRST { take undef }
  state Str $text = '';
  state Int $i = 1;
  my($num, $t) = .split: "\t", 2;
  if $i == $num { $text ~= $t }
  else {
   take ($text ~~ /\>\$\</ ?? undef !! $text), undef xx $num - $i - 1;
   ($text, $i) = ($t, $num);
  }
  LAST { take $text }
 }
}

my Str @longDesc <== indexLines <== $=adventData01.lines(:!chomp);
my Str @shortDesc <== indexLines <== $=adventData02.lines(:!chomp);

my int @travel[*;*;*]
 <== map { .defined ?? .split("\n").map: *.split("\t") !! undef }
 <== indexLines <== $=adventData03.lines(:!chomp);

(my Array of int %vocab).push: $=adventData04.lines.map: { .split("\t").[1,0] }

my Array of Str @itemDesc = gather for $=adventData05.lines(:!chomp) {
 state Int $i = 0;
 state Int $j = -1;
 state Str @accum = ();
 my($n, $msg) = .split: "\t", 2;
 if 0 < $n < 100 {
  take @accum.map({ /\>\$\</ ?? undef !! $_ }), [] xx $n - $i - 1;
  ($i, $j) = ($n, -1);
  @accum = $msg, ;
 } else {
  @accum.push: '' if $n / 100 != $j;
  @accum[*-1] ~= $msg;
  $j = $n / 100;
 }
 LAST { take @accum.map: { /\>\$\</ ?? undef !! $_ } }
}

my Str @rmsg <== indexLines <== $=adventData06.lines(:!chomp);

my int @place[65];
my int @fixed[65];
my Array of int @atloc;
for $adventData07.lines {
 my($obj, $p, $f) = .split: "\t";
 @place[$obj] = $p;
 @fixed[$obj] = $f // 0;
}
for @fixed.keys({ @fixed[$_] > 0 }).reverse -> $k {
 drop($k + 100, @fixed[$k]);
 drop($k, @place[$k]);
}
drop($_, @place[$_])
 for @fixed.keys({ @place[$_] != 0 && @fixed[$_] <= 0 }).reverse;

my int @actspk[32] <== indexLines <== $adventData08.lines;

my int @cond = 0, *;
for $=adventData09.lines {
 my($bit, @locs) = .split: "\t";
 @cond[$_] +|= 1 +< $bit for @locs;
}

my Pair @classes <== map { [=>] .split("\t") } <== $adventData10.lines(:!chomp);

my int @hints[*;4] <== map { .defined ?? .split("\t") !! undef }
 <== indexLines <== $adventData11.lines;

 #«« my Str @magicMsg <== indexLines <== $=adventData12.lines(!:chomp); »»


# Object & verb numbers:

enum item « :KEYS(1) LAMP GRATE CAGE ROD ROD2 STEPS BIRD DOOR PILLOW SNAKE
 FISSUR TABLET CLAM OYSTER MAGZIN DWARF KNIFE FOOD BOTTLE WATER OIL MIRROR
 PLANT PLANT2 :AXE(28) :DRAGON(31) CHASM TROLL TROLL2 BEAR MESSAG VOLCANO VEND
 BATTER :NUGGET(50) :COINS(54) CHEST EGGS TRIDENT VASE EMERALD PYRAM PEARL RUG
 SPICES CHAIN »;

enum movement « :BACK(8) :NULL(21) :LOOK(57) :DEPRESSION(63) :ENTRANCE(64)
 :CAVE(67) »;

enum action « :TAKE(1) DROP SAY OPEN NOTHING LOCK ON OFF WAVE CALM WALK KILL
 POUR EAT DRINK RUB THROW QUIT FIND INVENT FEED FILL BLAST SCORE FOO BRIEF READ
 BREAK WAKE SUSPEND HOURS {#< :LOAD(33) >} »;


# Global variables:

my int $loc;
my int $newloc;
my int $oldloc;
my int $oldloc2;
my int @prop[65] = 0 xx 50, -1 xx *;
my bool $wzdark = False;
my bool $closing = False;
my bool $lmwarn = False;
my bool $panic = False;
my bool $closed = False;
my bool $gaveup = False;
 #«« my bool $demo; »»
my int $tally = 15;
my int $tally2 = 0;
my int @hintlc[10] = 0, *;
my bool @hinted[10] = False, *;

constant int $chloc = 114;
constant int $chloc2 = 140;

my int @dloc[6] = 19, 27, 33, 44, 64, $chloc;
my int @odloc[6];
my bool @dseen[6];
my int $dflag = 0;

my int $turns = 0;
my int $limit;
my int $iwest = 0;
my int $knifeloc = 0;
my int $detail = 0;
my int @abb = 0, *;
my int $abbnum = 5;
constant int $maxdie = 3;
my int $numdie = 0;
my int $holding = 0;
my int $dkill = 0;
my int $foobar = 0;
my int $bonus = 0;
my int $clock1 = 30;
my int $clock2 = 50;
my bool $blklin = True;

# Global variables used in parsing commands:
my int $verb, $obj;
my Str $in1, $in2, $word1, $word2;


# Functions:

# I got sick of constantly flooring quotients (sometimes done because the
# original source required it, oftentimes done just because I don't trust
# Rakudo's typecasting), and I would like to use as many Perl 6 features as
# possible, so I defined an integer division operator:
multi sub infix:<idiv>(Int | Num | Rat $a, Int | Num | Rat $b --> Int)
 is equiv(&infix:<div>) is assoc('left') {
 ($a div $b).floor
}

sub toting(int $item --> Bool) { @place[$item] == -1 }
sub here(int $item --> Bool) { @place[$item] == $loc || toting $item }
sub at(int $item --> Bool) { $loc == @place[$item] | @fixed[$item] }
sub liq2(int $p --> int) { (WATER, 0, OIL)[$p] }
sub liq( --> int) { liq2(@prop[BOTTLE] max -1-@prop[BOTTLE]) }
sub liqloc(int $loc --> int) { liq2(@cond[$loc] +& 4 ?? @cond[$loc] +& 2 !! 1) }
sub bitset(int $loc, int $n --> Bool) { @cond[$loc] +& 1 +< $n }
sub forced(int $loc --> Bool) { @travel[$loc;0;1] == 1 }
sub dark( --> Bool) { !(@cond[$loc] +& 1 || (@prop[LAMP] && here(LAMP)) }
sub pct(int $x --> Bool) { (^100).pick < $x }

sub speak(Str $s) {
 return if !$s;
 print "\n" if $blklin;
 print $s;
}

sub pspeak(int $item, int $state) { speak @itemDesc[$item;$state+1] }
sub rspeak(int $msg) { speak @rmsg[$msg] if $msg != 0 }

sub yes(int $x, int $y, int $z --> Bool) {
 loop {
  rspeak $x if $x != 0;
  print "\n> ";
  my Str $reply = $*IN.get;
  if $reply ~~ m:i/^^\h*y/ {
   rspeak $y if $y != 0;
   return True;
  } elsif $reply ~~ m:i/^^\h*n/ {
   rspeak $z if $z != 0;
   return False;
  } else { say "Please answer the question." }
 }
}

sub destroy(int $obj) { move $obj, 0 }

sub juggle(int $obj) {
 move $obj, @place[$obj];
 move $obj+100, @fixed[$obj];
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

 #< sub ciao() {mspeak 32; exit 0; } >

sub bug(int $num) {
 say "Fatal error, see source code for interpretation.";

# Given the above message, I suppose I should list the possible bug numbers in
# the source somewhere, and right here is as good a place as any:
# 5 - Required vocabulary word not found
# 20 - Special travel (500>L>300) exceeds GOTO list
# 22 - Vocabulary type (N/1000) not between 0 and 3
# 23 - Intransitive action verb exceeds GOTO list
# 24 - Transitive action verb exceeds GOTO list
# 25 - Conditional travel entry with no alternative
# 26 - Location has no travel entries
# 27 - Hint number exceeds GOTO list
# 28 - Invalid month returned by date function

 #say "Probable cause: erroneous info in database.";  # Not in this version
 say "Error code = $num";
 exit -1;
}

sub vocab(Str $word, int $type --> int) {
 my int @matches = %vocab{$word};
 if $type >= 0 { @matches.=grep: { $_ idiv 1000 == $type } }
 if !@matches {
  if $type >= 0 { bug 5 }
  return -1;
 } else { return $type >= 0 ?? @matches[0] % 1000 !! [min] @matches }
 # When returning values of a specified type, there can be no more than one
 # match; if there is more than one, someone's been messing with the data
 # sections.
}

sub dwarves() {
 return if $loc == 0 || forced $loc || bitset $newloc, 3;
 if $dflag == 0 {
  $dflag = 1 if $loc >= 15;
  return;
 }
 if $dflag == 1 {
  return if $loc < 15 || pct 95;
  $dflag = 2;
  for 1, 2 { @dloc[(^5).pick] = 0 if pct(50) #< && $saved == -1 > }
  for ^5 -> $i {
   @dloc[$i] = 18 if @dloc[$i] == $loc;
   @odloc[$i] = @dloc[$i];
  }
  rspeak 3;
  drop AXE, $loc;
  return;
 }
 my($dtotal, $attack, $stick) = 0, *;
 L6030: for ^6 -> $i {
  # The individual dwarven movement loop, named L6030 because that's the
  # GOTO label at which it ends and TO which numerous statements try to GO.
  next if @dloc[$i] == 0;
  my int @tk = grep {
   my $newloc = $_ % 1000;
   15 <= $newloc <= 300 && $newloc != @odloc[$i] & @dloc[$i]
    && !forced($newloc) && !($i == 5 && bitset($newloc, 3))
    && $_ idiv 1000 != 100;
  } @travel[@dloc[$i];*;0];
  @tk.push: @odloc[$i];
  (@odloc[$i], @dloc[$i]) = @dloc[$i], @tk.pick;
  @dseen[$i] = (@dseen[$i] && $loc >= 15) || @dloc[$i] | @odloc[$i] == $loc;
  if @dseen[$i] {
   @dloc[$i] = $loc;
   if $i == 5 {
    # Pirate logic:
    next L6030 if $loc == $chloc || @prop[CHEST] >= 0;
    my Bool $k = False;
    for 50..64 -> $j {
     next if $j == PYRAM && $loc == 100 | 101;
     if toting $j {
      rspeak 128;
      move CHEST, $chloc if @place[MESSAG] == 0;
      move MESSAG, $chloc2;
      for 50..64 -> $j {
       next if $j == PYRAM && $loc == 100 | 101;
       carry $j, $loc if at($j) && @fixed[$j] == 0;
       drop $j, $chloc if toting $j;
      }
      @dloc[5] = @odloc[5] = $chloc;
      @dseen[5] = False;
      next L6030;
     }
     $k = True if here $j;
    }
    if $tally == $tally2 + 1 && !$k && @place[CHEST] == 0 && here LAMP 
     && @prop[LAMP] == 1 {
     rspeak 186;
     move CHEST, $chloc;
     move MESSAG, $chloc2;
     @dloc[5] = @odloc[5] = $chloc;
     @dseen[5] = False;
     next L6030;
    }
    rspeak 127 if @odloc[6] != @dloc[6] && pct 20;
    next L6030;
   } else {
    $dtotal++;
    next L6030 if @odloc[$i] != @dloc[$i];
    $attack++;
    $knifeloc = $loc if $knifeloc >= 0;
    $stick++ if (^1000).pick < 95 * ($dflag - 2);
   }
  }
 } # end of individual dwarf loop
 return if $dtotal == 0;
 if $dtotal != 1 {
  say "There are $dtotal threatening little dwarves in the room with you."
 } else { rspeak 4 }
 return if $attack == 0;
 $dflag = 3 if $dflag == 2;
 #< $dflag = 20 if $saved != -1; >
 my $k;
 if $attack != 1 {
  say "$attack of them throw knives at you!";
  $k = 6;
 } else {rspeak 5; $k = 52; }
 if $stick <= 1 {
  rspeak $k + $stick;
  return if $stick == 0;
 } else { say "$stick of them get you!" }
 $oldloc2 = $loc;
 death;
}

# Label 8; GOTO 2 (i.e., next bigLoop) on return:
sub domove(int $motion) {
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
     my $ll = @travel[$loc;$kk;0] % 1000;
     if $ll == $k {
      dotrav @travel[$loc;$kk;1];
      return;
     } elsif $ll <= 300 {
      $k2 = $kk if forced $ll && @travel[$ll;0;0] % 1000 == $k
     }
    }
    if $k2 != 0 { dotrav @travel[$loc;$k2;1] }
    else { rspeak 140 }
   }
  }
  when LOOK {
   rspeak 15 if $detail++ < 3;
   $wzdark = False;
   @abb[$loc] = 0;
  }
  when CAVE { rspeak($loc < 8 ?? 57 !! 58) }
  default {($oldloc2, $oldloc) = ($oldloc, $loc); dotrav $motion; }
 }
}

sub dotrav(int $motion) {
# 9:
 my int $rdest = -1;
 for @travel[$loc] -> $kk {
  if $kk[1..*].any == 1 | $motion ff * {
   my int $ll = $kk[0];
   my int $rcond = $ll idiv 1000;
   my int $robject = $rcond % 100;
   given $rcond {
    when 0 | 100 { $rdest = $ll % 1000 }
    when 0 ^..^ 100 { $rdest = $ll % 1000 if pct $_ }
    when 100 ^.. 200 { $rdest = $ll % 1000 if toting $robject }
    when 200 ^.. 300 { $rdest = $ll % 1000 if toting $robject || at $robject }
    default { $rdest = $ll % 1000 if @prop[$robject] != $_ idiv 100 - 3 }
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
  when 500 ^.. * { rspeak $rdest-500 }
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
  normend if $numdie == $maxdie || !$yea;
  @place[WATER, OIL] = 0, 0;
  @prop[LAMP] = 0 if toting LAMP;
  for 64..1:by(-1) -> $i {
   next if !toting $i;
   drop $i, $i == LAMP ?? 1 !! $oldloc2;
  }
  ($loc, $oldloc) = 3, 3;
  #< GOTO 2000 >
 }
}

sub score(Bool $scoring --> int) {
 my int $score = 0;
 for 50..64 -> $i {
  $score += 2 if @prop[$i] >= 0;
  $score += $i == CHEST ?? 12 !! $i > CHEST ?? 14 !! 10
   if @place[$i] == 3 && @prop[$i] == 0;
 }
 $score += ($maxdie - $numdie) * 10;
 $score += 4 if !($scoring || $gaveup);
 $score += 25 if $dflag != 0;
 $score += 25 if $closing;
 if $closed {
  given $bonus {
   when 0 { $score += 10 }
   when 135 { $score += 25 }
   when 134 { $score += 30 }
   when 133 { $score += 45 }
  }
 }
 $score++ if @place[MAGZIN] == 108;
 $score += 2;
 for 1..9 -> $i { $score -= @hints[$i;1] if @hinted[$i] }
 return $score;
}

sub normend() {
 my $score = score(False);
 say "You scored $score out of a possible 350 using $turns turns.";
 my($rank, $next) = @classes.grep({ .key >= $score }).[0,1];
 if $rank {
  speak $rank.value;
  if $next {
   my $diff = $next.key - $score + 1;
   say "To achieve the next higher rating, you need $diff more point",
    $diff == 1 ?? '.' !! 's.';
  } else {
   say "To achieve the next higher rating would be a neat trick!";
   say "Congratulations!!";
  }
 } else { say "You just went off my scale!!" }
 exit 0;
}


sub MAIN #< Insert command-line stuff here > {

 #«« poof; $demo = start; motd(False); »»
 $newloc = 1;
 $limit = (@hinted[3] = yes(65, 1, 0)) ?? 1000 !! 330;
 #< $setup = 3; >  ???

 bigLoop: loop {
# 2:
  if 0 < $newloc < 9 && $closing {
   rspeak 130;
   $newloc = $loc;
   $clock2 = 15 if !$panic;
   $panic = True;
  }
# 71:
  if $newloc != $loc && !forced($loc) && !bitset($loc, 3)
   && { @odloc[$^i] == $newloc && @dseen[$^i] }(any ^5) {
   $newloc = $loc;
   rspeak 2;
  }
# 74:
  $loc = $newloc;
  dwarves;
# 2000:
  death if $loc == 0;
  my Str $kk = @shortdesc[$loc];
  $kk = @longdesc[$loc] if @abb[$loc] % $abbnum == 0 || !$kk.defined;
  if !forced($loc) && dark {
   if $wzdark && pct 35 {
    rspeak 23;
    $oldloc2 = $loc;
    death;
   }
   $kk = @rmsg[16];
  }
  rspeak 141 if toting BEAR;
  speak $kk;
  if forced $loc {domove 1; next bigLoop; }
  rspeak 8 if $loc == 33 && pct(25) && !$closing;
  if !dark {
   @abb[$loc]++;
   for @atloc[$loc] -> $obj {
    $obj -= 100 if $obj > 100;
    next if $obj == STEPS && toting NUGGET;
    if @prop[$obj] < 0 {
     next if $closed;
     @prop[$obj] = $obj == RUG | CHAIN ?? 1 !! 0;
     $tally--;
     $limit = 35 min $limit if $tally == $tally2 && $tally != 0;
    }
    pspeak $obj, $obj == STEPS && $loc == @fixed[STEPS] ?? 1 !! @prop[$obj];
   }
  }
# 2012:
  ($verb, $obj) = 0, 0;
# 2600:
  hintLoop: for 4..9 -> $hint {
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
      # This ^^ is supposed to check whether there is at least one item at any
      # of the given locations; does it work right?
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
    say "I am prepared to give you a hint, but it will cost you ",
     @hints[$hint;1], " points.";
    @hinted[$hint] = yes(175, @hints[$hint;3], 54);
    limit += 30 * @hints[$hint;1] if @hinted[$hint] && $limit > 30;
   }
  }
  if $closed {
   pspeak OYSTER, 1 if @prop[OYSTER] < 0 && toting OYSTER;
   @prop[$_] = -1 - @prop[$_] for grep { toting $_ && @prop[$_] < 0 }, 1..64;
  }
# 2605:
  $wzdark = dark;
  $knifeloc = 0 if 0 < $knifeloc != $loc;
  print "\n> ";
  ($in1, $in2) = $*IN.get.words.[0,1];
  ($word1, $word2) = ($in1, $in2).map:
   { .defined ?? .substr(0, 5).uc !! undef };
# 2608:
  $foobar = 0 min -$foobar;
  #«« maint if $turns == 0 && $word1 eq 'MAGIC' && $word2 eq 'MODE'; »»
  $turns++;
  #«« if $demo && $turns >= $short {mspeak 1; normend; } »»
  $verb = 0 if $verb == SAY && $word2;
  if $verb == SAY { #< GOTO 9030 > }
  $clock1-- if $tally == 0 && 15 <= $loc != 33;
  if $clock1 == 0 {
   @prop[GRATE, FISSUR] = 0, 0;
   (@dseen[$_], @dloc[$_]) = False, 0 for ^6;
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
   #< GOTO 19999 >
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
   destroy $_ for grep { toting $_ }, 1..64;
   # Could this be written as ".destroy for (1..64).grep: *.toting" ?
   rspeak 132;
   $closed = True;
   next bigLoop;
  }
  $limit-- if @prop[LAMP] == 1;
  if $limit <= 30 && here BATTER && @prop[BATTER] == 0 && here LAMP {
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
# 19999:
  $k = 43;
  $k = 70 if liqloc $loc == WATER;
  if $word1 eq 'ENTER' && $word2 eq 'STREA' | 'WATER' {
   rspeak $k;
   #< GOTO 2012 >
  }
  if $word1 eq 'ENTER' && $word2 { ($word1, $word2) = ($word2, undef) }
  elsif $word1 eq 'WATER' | 'OIL' && $word2 eq 'PLANT' | 'DOOR' {
   $word2 = 'POUR' if at vocab($word2, 1)
  }
  for $word1, $word2 -> $wd {
   rspeak 17 if $wd eq 'WEST' && ++$iwest == 10;
# 2630:
   my $i = vocab $wd, -1;
   if $i == -1 {rspeak(pct 20 ?? 61 !! pct 20 ?? 13 !! 60); #< GOTO 2600 > }
   $k = $i % 1000;
   given $i idiv 1000 {
    when 0 {domove $k; next bigLoop; }
    when 1 {
# 5000:
     $obj = $k;
     if @fixed[$k] == $loc || here $k {
# 5010:
      if $word2 { #< GOTO 2800 > }
      if $verb { #< GOTO 4090 > }
      say "What do you want to do with the $in1?";
      #< GOTO 2600 >
     } else {
      if $k == GRATE {
       $k = DEPRESSION if $loc == 1 | 4 | 7;
       $k = ENTRANCE if 9 < $loc < 15;
       if $k != GRATE {domove $k; next bigLoop; }
      } elsif $k == DWARF {
       if $dflag >= 2 && @dloc[^5].any == $loc { #< GOTO 5010 > }
      }
      if liq == $k && here BOTTLE || $k == liqloc $loc { #< GOTO 5010 > }
      if $obj == PLANT && at PLANT2 && @prop[PLANT2] != 0 {
       $obj = PLANT2;
       #< GOTO 5010 >
      }
      if $obj == KNIFE && $knifeloc == $loc {
       $knifeloc = -1;
       rspeak 116;
       #< GOTO 2012 >
      }
      if $obj == ROD && here ROD2 {
       $obj = ROD2;
       #< GOTO 5010 >
      }
# 5190:
      if $verb == FIND | INVENT && !$word2 { #< GOTO 5010 > }
      say "I see no $in1 here.";
      #< GOTO 2012 >
     }
    }
    when 2 {
# 4000:
     $verb = $k;
     #< my $spk = @actspk[$verb]; >
     if $word2 && $verb != SAY { #< GOTO 2800 > }
     $obj = $word2 if $verb == SAY;
     if $obj == 0 {
      ### Execute a function as determined by the map at label 4080

     } else {
      ### Execute a function as determined by the map at label 4090

     }
     ### Now what?

    }
    when 3 {rspeak $k; #< GOTO 2012 > }
    default { bug 22 }
   }
  }



}


#
# Insert data sections here
#
