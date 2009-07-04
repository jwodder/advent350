#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "advconfig.h"
#include "advconst.h"


#ifdef ADVMAGIC
const char* magicMsg[] = { ... };
#endif


/* Global variables: */

int togoto = 2;
bool blklin = true;
#ifdef ADVMAGIC
bool demo = false;
#endif
int verb, obj;
char* in1, in2, word1, word2;

#ifdef ADVMAGIC
/* These arrays hold the times when adventurers are allowed into Colossal Cave;
 * @wkday is for weekdays, @wkend for weekends, and @holid for holidays (days
 * with special hours).  If element $n of an array is true, then the hour $n:00
 * through $n:59 is considered "prime time," i.e., the cave is closed then. */
bool wkday[24];
bool wkend[24];
bool holid[24];

int hbegin, hend;  /* start & end of next holiday */
char* hname;  /* name of next holiday */
int shortGame;  /* turns allowed in a short/demonstration game */
char* magic;  /* magic word */
int magnm;  /* magic number */
int latency;  /* time required to wait after saving */
char* msg;  /* MOTD, initially null */
#endif

/* User's game data: */
int loc, newloc, oldloc, oldloc2, limit;
int turns = 0, iwest = 0, knifeloc = 0, detail = 0;
int numdie = 0, holding = 0, foobar = 0, bonus = 0;
int tally = 15;
int tally2 = 0;
int abbnum = 5;
int clock1 = 30;
int clock2 = 50;
bool wzdark = false, closing = false, lmwarn = false, panic = false
bool closed = false, gaveup = false;
int prop[65] = 0 xx 50, -1 xx *;
int abb[141] = 0, *;
int hintlc[10] = 0, *;
bool hinted[10] = False, *;
int dloc[6] = {19, 27, 33, 44, 64, CHLOC};
int odloc[6];
bool dseen[6];
int dflag = 0, dkill = 0;
int place[65];
int fixed[65];
int atloc[65];
int link[65];
#ifdef ADVMAGIC
int saved, savet = -1, 0;
#endif


int main(int argc, char** argv) {
#ifdef ADVMAGIC
 poof();
#endif
 if (argc > 1) {
  /* Load a saved game */
  vresume(argv[1]);
  /* Check for failure? */
 } else {
#ifdef ADVMAGIC
  demo = start();
  motd(false);
#endif
  newloc = 1;
  limit = (hinted[3] = yes(65, 1, 0)) ? 1000 : 330;
 }

 /* ...and begin! */

/* A note on the flow control used in this program:
 *
 * Although the large function below (cleverly named "turn") contains the logic
 * for a single turn, not all of it is evaluated every turn; for example, after
 * most non-movement verbs, control passes to the original Fortran's label 2012
 * rather than to label 2 (the start of the function).  In the original
 * Fortran, this was all handled by a twisty little maze of GOTO statements,
 * all different, but since GOTOs are heavily frowned upon nowadays, and
 * because this port of Adventure is intended to be an exercise in modern
 * programming techniques rather than in ancient ones, I had to come up with a
 * better way.
 *
 * (Side note: In the BDS C port of Adventure, all of the turn code is
 * evaluated every turn, and you are very likely to get killed by a dwarf when
 * picking up the axe in the middle of battle.)
 *
 * My best idea was to divide the function up at the necessary GOTO labels, put
 * them all in a "switch" block with "case" labels corresponding to the
 * original labels, and introduce a global variable (named "togoto") to switch
 * on that indicated what part of the function to start at next.  (My other
 * ideas were (a) a state machine in which each section of the loop was a
 * function that returned a number representing the next function to call and
 * (b) something involving exceptions.)  This works, but it was not what I had
 * hoped for.  If you know of  something better, let me know.
 *
 * In summary: I apologize for the code that you are about to see.
 */

 for (;;) turn();
 return 1;  /* This should never be reached (hence the error value). */
}

void turn(void) {
 switch (togoto) {
  case 2:

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
   return if $loc == 0 || forced($loc) || bitset $newloc, 3;
   if $dflag == 0 {
    $dflag = 1 if $loc >= 15;
    return;
   }
   if $dflag == 1 {
    return if $loc < 15 || pct 95;
    $dflag = 2;
    @dloc[(^5).pick] = 0 if pct 50 for 1, 2;
    for ^5 -> $i {
     @dloc[$i] = 18 if @dloc[$i] == $loc;
     @odloc[$i] = @dloc[$i];
    }
    rspeak 3;
    drop AXE, $loc;
    return;
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

  case 2000:

   if $loc == 0 {death; return; }
   my Str $kk = @shortdesc[$loc];
   $kk = @longdesc[$loc] if @abb[$loc] % $abbnum == 0 || !$kk.defined;
   if !forced($loc) && dark() {
    if $wzdark && pct 35 {
     rspeak 23;
     $oldloc2 = $loc;
     death;
     return;
    }
    $kk = @rmsg[16];
   }
   rspeak 141 if toting BEAR;
   speak $kk;
   if forced $loc {domove 1; return; }
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

  case 2012:
   verb = obj = 0;

  case 2600:

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
   $wzdark = dark;
   $knifeloc = 0 if 0 < $knifeloc != $loc;
   ($word1, $in1, $word2, $in2) = getin;

  case 2608:

   $foobar = 0 min -$foobar;
#ifdef ADVMAGIC
   if (turns == 0 && strcmp(word1, "MAGIC") == 0 && strcmp(word2, "MODE") == 0)
    maint();
#endif
   $turns++;
#ifdef ADVMAGIC
   if (demo && turns >= shortGame) {
    mspeak(1);
    normend();
   }
#endif
   $verb = 0 if $verb == SAY && $word2;
   if $verb == SAY {vsay; return; }
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
    return;
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

  case 2609:

# This label is 19999 in the original Fortran, but it is being treated here as
# 2609 so that fall-through will work correctly.
   if $word1 eq 'ENTER' && $word2 eq 'STREA' | 'WATER' {
    rspeak(liqloc($loc) == WATER ?? 70 !! 43);
    $goto = 2012;
    return;
   }
   if $word1 eq 'ENTER' && $word2 { ($word1, $word2) = ($word2, undef) }
   elsif $word1 eq 'WATER' | 'OIL' && $word2 eq 'PLANT' | 'DOOR' {
    $word2 = 'POUR' if at vocab($word2, 1)
   }

  case 2610:

   rspeak 17 if $word1 eq 'WEST' && ++$iwest == 10;

  case 2630:

   my int $i = vocab $word1, -1;
   if $i == -1 {
    rspeak(pct(20) ?? 61 !! pct(20) ?? 13 !! 60);
    $goto = 2600;
    return;
   }
   my int $k = $i % 1000;
   given $i idiv 1000 {
    when 0 { domove $k }
    when 1 {
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
     $verb = $k;
     if $word2 && !($verb == SAY | SUSPEND | RESUME) {
      ($word1, $in1) = ($word2, $in2);
      $word2 = $in2 = undef;
      $goto = 2610;
      return;
     }
     $obj = $word2.defined if $verb == SAY | SUSPEND | RESUME;
     # This assignment just indicates whether an object was supplied.
     $obj ?? transitive !! intransitive;
    }
    when 3 {rspeak $k; $goto = 2012; }
    default { bug 22 }
  }
 }
}
