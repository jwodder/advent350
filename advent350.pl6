#!/usr/bin/env perl6
use v6;

# Initialize database (lazily):

sub indexLines(Str *@lines --> List of Str) {
 gather for @lines {
  FIRST { take undef }
  state Str $text = '';
  state int $i = 1;
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
 state int $i = 0;
 state int $j = -1;
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
my int @atloc[141;*];
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
 BREAK WAKE SUSPEND HOURS {#< :RESUME >} »;


# Global variables:

constant int MAXDIE, CHLOC, CHLOC2 = 3, 114, 140;
my int $goto = 0;
 #«« my bool $demo; »»
my bool $blklin = True;
my int $verb, $obj;
my Str $in1, $in2, $word1, $word2;

# User's game data:
my int $loc, $newloc, $oldloc, $oldloc2, $limit;
my int $turns, $iwest, $knifeloc, $detail = 0, *;
my int $numdie, $holding, $foobar, $bonus = 0, *;
my bool $wzdark, $closing, $lmwarn, $panic, $closed, $gaveup = False, *;
my int $tally = 15;
my int $tally2 = 0;
my int $abbnum = 5;
my int $clock1 = 30;
my int $clock2 = 50;
my int @prop[65] = 0 xx 50, -1 xx *;
my int @abb[141] = 0, *;
my int @hintlc[10] = 0, *;
my bool @hinted[10] = False, *;
my int @dloc[6] = 19, 27, 33, 44, 64, CHLOC;
my int @odloc[6];
my bool @dseen[6];
my int $dflag, $dkill = 0, *;


# Functions:

# I got sick of constantly flooring quotients (sometimes done because the
# original source required it, oftentimes done just because I don't trust
# Rakudo's typecasting), and I would like to use as many Perl 6 features as
# possible in this program, so I defined an integer division operator:
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

 #«« sub ciao() {mspeak 32; exit 0; } »»

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
 # next bigLoop;
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
    when 200 ^.. 300 { $rdest = $ll % 1000 if toting($robject) || at $robject }
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
 for 50..64 -> $i {
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
 $score -= @hints[$_;1] if @hinted[$_] for 1..9;
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

sub doaction() {
# 5010:
 if $word2 {
# 2800:
  ($word1, $in1) = ($word2, $in2);
  $word2 = $in2 = undef;
  $goto = 2610;
 } elsif $verb { transitive }
 else {
  say "What do you want to do with the $in1?";
  $goto = 2600;
 }
 # next bigLoop;
}


sub MAIN #< Insert command-line stuff here > {

 #«« poof; $demo = start(False); motd(False); »»
 $newloc = 1;
 $limit = (@hinted[3] = yes(65, 1, 0)) ?? 1000 !! 330;

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
 # each part inside a "when" block with a "continue" at the end, and introduce
 # a global variable (named "$goto", of course) to switch on that indicated
 # what part of the loop to start the next iteration at.  (My other ideas were
 # (a) a state machine in which each section of the loop was a function that
 # returned a number representing the next function to call and (b) something
 # involving exceptions.)  This works, but it was not what I had hoped for.
 # Perl 6 seems like it should have a more elegant solution to this problem,
 # but I couldn't find anything better in the Synopses.  If you know of
 # something better, let me know.

 # In summary: I apologize for the code that you are about to see.

 bigLoop: loop {
  given $goto {
   when *..2 {
# 2:
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
    continue if $loc == 0 || forced($loc) || bitset $newloc, 3;
    if $dflag == 0 {
     $dflag = 1 if $loc >= 15;
     continue;
    }
    if $dflag == 1 {
     continue if $loc < 15 || pct 95;
     $dflag = 2;
     @dloc[(^5).pick] = 0 if pct 50 for 1, 2;
     for ^5 -> $i {
      @dloc[$i] = 18 if @dloc[$i] == $loc;
      @odloc[$i] = @dloc[$i];
     }
     rspeak 3;
     drop AXE, $loc;
     continue;
    }
    my int $dtotal, $attack, $stick = 0, *;
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
       next L6030 if $loc == CHLOC || @prop[CHEST] >= 0;
       my Bool $k = False;
       for 50..64 -> $j {
	next if $j == PYRAM && $loc == 100 | 101;
	if toting $j {
	 rspeak 128;
	 move CHEST, CHLOC if @place[MESSAG] == 0;
	 move MESSAG, CHLOC2;
	 for 50..64 -> $j {
	  next if $j == PYRAM && $loc == 100 | 101;
	  carry $j, $loc if at($j) && @fixed[$j] == 0;
	  drop $j, CHLOC if toting $j;
	 }
	 @dloc[5] = @odloc[5] = CHLOC;
	 @dseen[5] = False;
	 next L6030;
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
	next L6030;
       }
       rspeak 127 if @odloc[5] != @dloc[5] && pct 20;
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
    continue if $dtotal == 0;
    if $dtotal != 1 {
     say "There are $dtotal threatening little dwarves in the room with you."
    } else { rspeak 4 }
    continue if $attack == 0;
    $dflag = 3 if $dflag == 2;
    my int $k;
    if $attack != 1 {
     say "$attack of them throw knives at you!";
     $k = 6;
    } else {rspeak 5; $k = 52; }
    if $stick <= 1 {
     rspeak $k + $stick;
     continue if $stick == 0;
    } else { say "$stick of them get you!" }
    $oldloc2 = $loc;
    death;
    # If the player is reincarnated after being killed by a dwarf, they GOTO
    # label 2000 using fallthrough rather than with any special flow control.
    continue;
   }

   when *..2000 {
# 2000:
    if $loc == 0 {death; next bigLoop; }
    my Str $kk = @shortdesc[$loc];
    $kk = @longdesc[$loc] if @abb[$loc] % $abbnum == 0 || !$kk.defined;
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
       @prop[$obj] = $obj == RUG | CHAIN ?? 1 !! 0;
       $tally--;
       $limit = 35 min $limit if $tally == $tally2 && $tally != 0;
      }
      pspeak $obj, $obj == STEPS && $loc == @fixed[STEPS] ?? 1 !! @prop[$obj];
     }
    }
    continue;
   }

   when *..2012 {
# 2012:
    ($verb, $obj) = 0, 0;
    continue;
   }

   when *..2600 {
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
    continue;
   }

   when *..2608 {
# 2608:
    $foobar = 0 min -$foobar;
    #«« maint if $turns == 0 && $word1 eq 'MAGIC' && $word2 eq 'MODE'; »»
    $turns++;
    #«« if $demo && $turns >= $short {mspeak 1; normend; } »»
    $verb = 0 if $verb == SAY && $word2;
    if $verb == SAY {vsay; next bigLoop; }
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
     continue;  # GOTO 19999, a.k.a. 2609
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
    continue;
   }

   when *..2609 {
# This label is 19999 in the original Fortran, but it is being treated here as
# 2609 so that fall-through will work correctly.
    if $word1 eq 'ENTER' && $word2 eq 'STREA' | 'WATER' {
     rspeak(liqloc($loc) == WATER ?? 70 !! 43);
     $goto = 2012;
     next bigLoop;
    }
    if $word1 eq 'ENTER' && $word2 { ($word1, $word2) = ($word2, undef) }
    elsif $word1 eq 'WATER' | 'OIL' && $word2 eq 'PLANT' | 'DOOR' {
     $word2 = 'POUR' if at vocab($word2, 1)
    }
    continue;
   }

   when *..2610 {
# 2610:
    rspeak 17 if $word1 eq 'WEST' && ++$iwest == 10;
    continue;
   }

   when *..2630 {
# 2630:
    my int $i = vocab $word1, -1;
    if $i == -1 {
     rspeak(pct(20) ?? 61 !! pct(20) ?? 13 !! 60);
     $goto = 2600;
     next bigLoop;
    }
    my int $k = $i % 1000;
    given $i idiv 1000 {
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
	else {say "I see no $in1 here."; $goto = 2012; }
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
       else {say "I see no $in1 here."; $goto = 2012; }
      }
     }
     when 2 {
# 4000:
      $verb = $k;
      if $word2 && $verb != SAY {
       ($word1, $in1) = ($word2, $in2);
       $word2 = $in2 = undef;
       $goto = 2610;
       next bigLoop;
      }
      $obj = $word2.defined if $verb == SAY;
      # This assignment just indicates whether an object was supplied to the
      # "SAY" verb.
      $obj == 0 ?? intransitive !! transitive;
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
   say "$in1 what?";
   $obj = 0;
   $goto = 2600;
  }
  when TAKE {
   if @atloc[$loc] != 1 || $dflag >= 2 && @dloc[^5].any == $loc {
    say "$in1 what?";
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
    say "$in1 what?";
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
    say "$in1 what?";
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
   say "If you were to quit now, you would score $score of a possible 350.";
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
    say "$in1 what?"; $obj = 0; $goto = 2600;
   } else { vread }
  }
  when SUSPEND {
  #««
   if $demo {rspeak 201; return; }
   say "I can suspend your adventure for you so that you can resume later, but";
   say "you will have to wait at least $latency minutes before continuing.";
   if yes(200, 54, 54) {
    ($saved, $savet) = datime;
    #< Actually save the game data somewhere in here. >
    ciao;
   }
  »»
   
   # See label 8305 for what to (possibly) do on restoration.

  }
  when HOURS {
   #«« mspeak 6; hours; »»
   # Possible non-magic version:
   say "Colossal Cave is open all day, every day."
  }
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
  when CALM | WALK | QUIT | SCORE | FOO | BRIEF | SUSPEND | HOURS {
   rspeak @actspk[$verb];
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
 if fixed $obj {rspeak $spk; return; }
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
 $k = liq;
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
   $spk = 120 + $k if toting OBJ;
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
 if dark() { say "I see no $in1 here." }
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
  if $obj > 100 {say "$in1 what?"; $obj = 0; $goto = 2600; return; }
  if $obj == 0 {
   $obj = BIRD if here(BIRD) && $verb != THROW;
   $obj = $obj * 100 + CLAM if here(CLAM | OYSTER);
   if $obj > 100 {say "$in1 what?"; $obj = 0; $goto = 2600; return; }
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
    print "\n> ";
    my Str $reply = $*IN.get;
    if $reply !~~ m:i/^^\h*y/ {
     ($in1, $in2) = $reply.words.[0,1];
     ($word1, $word2) = ($in1, $in2).map:
      { .defined ?? .substr(0, 5).uc !! undef };
     $goto = 2608;
     return;
    }
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
  when BEAR { rspeak(165 + (@prop[BEAR]+1) idiv 2) }
 }
}

sub vpour() {
# 9130:
 $obj = liq if $obj == BOTTLE | 0;
 if $obj == 0 {say "$in1 what?"; $obj = 0; $goto = 2600; }
 elsif !toting $obj { rspeak @actspk[$verb] }
 elsif !($obj == OIL | WATER) { rspeak 78 }
 else {
  @prop[BOTTLE] = 1;
  @place[OBJ] = 0;
  if at DOOR {
   @prop[DOOR] = ($obj == OIL);
   rspeak 113 + @prop[DOOR];
  } elsif at PLANT {
   if $obj != WATER { rspeak 112 }
   else {
    pspeak PLANT, @prop[PLANT] + 1;
    @prop[PLANT] = (@prop[PLANT] + 2) % 6;
    @prop[PLANT2] = @prop[PLANT] idiv 2;
    domove NULL;
   }
  } else { rspeak 77 }
 }
}

sub vdrink() {
# 9150:
 if $obj == 0 && liqloc($loc) != WATER && (liq != WATER || !here BOTTLE) {
  say "$in1 what?";
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
   say "$in1 what?";
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
  $word2 = undef;
  $obj = 0;
  $goto = 2630;
 } else { say "Okay, \"$tk\"." }
}


#
# Insert data sections here
#
