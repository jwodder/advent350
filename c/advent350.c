#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include "advconfig.h"
#include "advconst.h"
#include "advdecl.h"

/* Definitions of global variables: */
int togoto = 2;
bool blklin = true, gaveup = false;
int bonus = 0;
int verb, obj;
char in1[MAX_INPUT_LENGTH+1], in2[MAX_INPUT_LENGTH+1];
char word1[6], word2[6];

#ifdef ADVMAGIC
bool demo = false;

struct advmagic mage = {
 .wkday = 000777400, .wkend = 0, .holid = 0,
 .hbegin = 0, .hend = -1, .shortGame = 30, .magnm = 11111, .latency = 90,
 .magic = "DWARF", .hname = "", .msg = ""
};
#endif

struct advgame game = {
 .loc = 0, .newloc = 0, .oldloc = 0, .oldloc2 = 0, .limit = 0, .turns = 0,
 .iwest = 0, .knifeloc = 0, .detail = 0, .numdie = 0, .holding = 0,
 .foobar = 0, .tally = 15, .tally2 = 0, .abbnum = 5, .clock1 = 30, .clock2 = 50,
 .wzdark = false, .closing = false, .lmwarn = false, .panic = false,
 .closed = false,
 .prop = {[50] = -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
  /* Elements 0 through 49 are implicitly set to zero. */
 .abb = {0}, .hintlc = {0}, .hinted = {0}, .dloc = {19, 27, 33, 44, 64, CHLOC},
 .odloc = {0}, .dseen = {0}, .atloc = {0}, .link = {0},
 .dflag = 0, .dkill = 0, .saved = -1, .savet = 0,
 .place = {
  0, 3, 3, 8, 10, 11, 0, 14, 13, 94,
  96, 19, 17, 101, 103, 0, 106, 0, 0, 3,
  3, 0, 0, 109, 25, 23, 111, 35, 0, 97,
  0, 119, 117, 117, 0, 130, 0, 126, 140, 0,
  96, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  18, 27, 28, 29, 30, 0, 92, 95, 97, 100,
  101, 0, 119, 127, 130
 },
 .fixed = {
  0, 0, 0, 9, 0, 0, 0, 15, 0, -1,
  0, -1, 27, -1, 0, 0, 0, -1, 0, 0,
  0, 0, 0, -1, -1, 67, -1, 110, 0, -1,
  -1, 121, 122, 122, 0, -1, -1, -1, -1, 0,
  -1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 121, 0, -1
 }
};

void turn(void);

int main(int argc, char** argv) {
#ifdef ORIG_RNG
 ran(1);
#elif defined(RANDOM_RNG)
 srandom(time(NULL));
#else
 srand(time(NULL));
#endif

#ifdef ADVMAGIC
 poof();
#endif

 if (argc > 1) {if (!vresume(argv[1])) exit(EXIT_FAILURE); }
 else {
#ifdef ADVMAGIC
  demo = start();
  motd(false);
#endif
  for (int k=64; k>0; k--) {
   if (game.fixed[k] > 0) {
    drop(k+100, game.fixed[k]);
    drop(k, game.place[k]);
   }
  }
  for (int k=64; k>0; k--) {
   if (game.place[k] && game.fixed[k] <= 0) drop(k, game.place[k]);
  }
  game.newloc = 1;
  game.limit = (game.hinted[3] = yes(65, 1, 0)) ? 1000 : 330;
 }

 /* ...and begin! */

/* A note on the flow control used in this program:
 *
 * Although the large function below (cleverly named "turn") contains the logic
 * for a single turn, not all of it needs to be evaluated every turn.  For
 * example, after most non-movement verbs, control passes to the original
 * Fortran's label 2012 rather than to label 2 (the start of the function).  In
 * the original Fortran, this was all handled by a twisty little maze of GOTO
 * statements, all different, but since GOTOs are heavily frowned upon
 * nowadays, I had to come up with something better.
 *
 * (Side note: In the BDS C port of Adventure, all of the turn code is
 * evaluated every turn, and you are very likely to get killed by a dwarf when
 * picking up the axe in the middle of battle.)
 *
 * My best idea was to divide the function up at the necessary GOTO labels, put
 * them all in a "switch" block with fallthrough and with "case" labels equal
 * to the original labels, and introduce a global variable (cleverly named
 * "togoto") to switch on that indicated what part of turn() to start at next.
 * This works, but it is still quite bizarre.  If you know of something better,
 * let me know.
 *
 * In summary: I apologize for the code that you are about to see.
 */

 for (;;) turn();
 return EXIT_FAILURE;
  /* This "return" should never be reached (hence the error value). */
}

void turn(void) {
 switch (togoto) {
  case 2:
   if (0 < game.newloc && game.newloc < 9 && game.closing) {
    rspeak(130);
    game.newloc = game.loc;
    if (!game.panic) game.clock2 = 15;
    game.panic = true;
   }
   if (game.newloc != game.loc && !forced(game.loc) && !bitset(game.loc, 3)) {
    for (int i=0; i<5; i++) {
     if (game.odloc[i] == game.newloc && game.dseen[i]) {
      game.newloc = game.loc;
      rspeak(2);
      break;
     }
    }
   }
   game.loc = game.newloc;
   /* Dwarven logic: */
   togoto = 2000;  /* in preparation for an early `return' */
   if (game.loc == 0 || forced(game.loc) || bitset(game.newloc, 3)) return;
   if (game.dflag == 0) {
    if (game.loc >= 15) game.dflag = 1;
    return;
   }
   if (game.dflag == 1) {
    if (game.loc < 15 || pct(95)) return;
    game.dflag = 2;
    if (pct(50)) game.dloc[ran(5)] = 0;
    /* Yes, this is supposed to be done twice. */
    if (pct(50)) game.dloc[ran(5)] = 0;
    for (int i=0; i<5; i++) {
     if (game.dloc[i] == game.loc) game.dloc[i] = 18;
     game.odloc[i] = game.dloc[i];
    }
    rspeak(3);
    drop(AXE, game.loc);
    return;
   }
   int dtotal = 0, attack = 0, stick = 0;
   for (int i=0; i<6; i++) {  /* The individual dwarven movement loop */
    if (game.dloc[i] == 0) continue;
    int kk=0, tk = game.odloc[i];
    for (int j=0; j<MAXROUTE; j++) {
     if (travel[game.dloc[i]][j][0] == -1) break;
     int newloc = travel[game.dloc[i]][j][0] % 1000;
     if (15 <= newloc && newloc <= 300 && newloc != game.odloc[i]
      && newloc != game.dloc[i] && !forced(newloc) && !(i == 5
      && bitset(newloc, 3)) && travel[game.dloc[i]][j][0] / 1000 != 100
      && ran(++kk) == 0) tk = newloc;
    }
    game.odloc[i] = game.dloc[i];
    game.dloc[i] = tk;
    game.dseen[i] = (game.dseen[i] && game.loc >= 15)
     || game.dloc[i] == game.loc || game.odloc[i] == game.loc;
    if (game.dseen[i]) {
     game.dloc[i] = game.loc;
     if (i == 5) {
      /* Pirate logic: */
      if (game.loc == CHLOC || game.prop[CHEST] >= 0) continue;
      bool k = false, stole = false;
      for (int j=50; j<65; j++) {
       if (j == PYRAM && (game.loc == 100 || game.loc == 101)) continue;
       if (toting(j)) {
        rspeak(128);
	if (game.place[MESSAG] == 0) move(CHEST, CHLOC);
	move(MESSAG, CHLOC2);
	for (j=50; j<65; j++) {
	 if (j == PYRAM && (game.loc == 100 || game.loc == 101)) continue;
	 if (at(j) && game.fixed[j] == 0) carry(j, game.loc);
	 if (toting(j)) drop(j, CHLOC);
	}
	game.dloc[5] = game.odloc[5] = CHLOC;
	game.dseen[5] = false;
	stole = true;
	break;
       }
       if (here(j)) k = true;
      }
      if (!stole) {
       if (game.tally == game.tally2 + 1 && !k && game.place[CHEST] == 0
        && here(LAMP) && game.prop[LAMP] == 1) {
	rspeak(186);
	move(CHEST, CHLOC);
	move(MESSAG, CHLOC2);
	game.dloc[5] = game.odloc[5] = CHLOC;
	game.dseen[5] = false;
       } else if (game.odloc[5] != game.dloc[5] && pct(20)) rspeak(127);
      }
     } else {
      dtotal++;
      if (game.odloc[i] == game.dloc[i]) {
       attack++;
       if (game.knifeloc >= 0) game.knifeloc = game.loc;
       if (ran(1000) < 95 * (game.dflag - 2)) stick++;
      }
     }
    }
   } /* end of individual dwarven movement loop */
   if (dtotal == 0) return;
   if (dtotal == 1) rspeak(4);
   else
    printf("\nThere are %d threatening little dwarves in the room with you.\n",
     dtotal);
   if (attack == 0) return;
   if (game.dflag == 2) game.dflag = 3;
   int k;
   if (attack == 1) {rspeak(5); k = 52; }
   else {printf("\n%d of them throw knives at you!\n", attack); k = 6; }
   if (stick <= 1) {
    rspeak(k + stick);
    if (stick == 0) return;
   } else printf("\n%d of them get you!\n", stick);
   game.oldloc2 = game.loc;
   death();
  /* If the player is reincarnated after being killed by a dwarf, they GOTO
   * label 2000 using fallthrough rather than with any special flow control. */

  case 2000:
   if (game.loc == 0) {death(); return; }
   const char* kk = shortdesc[game.loc];
   if (game.abb[game.loc] % game.abbnum == 0 || kk == NULL)
    kk = longdesc[game.loc];
   if (!forced(game.loc) && dark()) {
    if (game.wzdark && pct(35)) {
     rspeak(23);
     game.oldloc2 = game.loc;
     death();
     return;
    }
    kk = rmsg[16];
   }
   if (toting(BEAR)) rspeak(141);
   speak(kk);
   if (forced(game.loc)) {domove(1); return; }
   if (game.loc == 33 && pct(25) && !game.closing) rspeak(8);
   if (!dark()) {
    game.abb[game.loc]++;
    for (int i = game.atloc[game.loc]; i != 0; i = game.link[i]) {
     int obj = i > 100 ? i - 100 : i;
     if (obj == STEPS && toting(NUGGET)) continue;
     if (game.prop[obj] < 0) {
      if (game.closed) continue;
      game.prop[obj] = obj == RUG || obj == CHAIN;
      game.tally--;
      if (game.tally == game.tally2 && game.tally != 0 && game.limit >= 35)
       game.limit = 35;
     }
     pspeak(obj, obj == STEPS && game.loc == game.fixed[STEPS] ? 1
      : game.prop[obj]);
    }
   }

  case 2012: verb = obj = 0;

  case 2600:
   for (int hint = 4; hint<10; hint++) {
    if (game.hinted[hint]) continue;
    if (!bitset(game.loc, hint)) game.hintlc[hint] = -1;
    game.hintlc[hint]++;
    if (game.hintlc[hint] >= hints[hint][0]) {
     bool hintable = true;
     switch (hint) {
      case 4:
       if (game.prop[GRATE] != 0 || here(KEYS)) {
        game.hintlc[hint] = 0;
	hintable = false;
       }
       break;
      case 5:
       if (!here(BIRD) || !toting(ROD) || obj != BIRD) hintable = false;
       break;
      case 6:
       if (!here(SNAKE) || here(BIRD)) {
        game.hintlc[hint] = 0;
	hintable = false;
       }
       break;
      case 7:
       if (game.atloc[game.loc] || game.atloc[game.oldloc]
        || game.atloc[game.oldloc2] || game.holding <= 1) {
        game.hintlc[hint] = 0;
	hintable = false;
       }
       break;
      case 8:
       if (game.prop[EMERALD] == -1 || game.prop[PYRAM] != -1) {
        game.hintlc[hint] = 0;
	hintable = false;
       }
       break;
     }
     if (hintable) {
      game.hintlc[hint] = 0;
      if (yes(hints[hint][2], 0, 54)) {
       printf("\nI am prepared to give you a hint, but it will cost you %d"
        " points.\n", hints[hint][1]);
       game.hinted[hint] = yes(175, hints[hint][3], 54);
       if (game.hinted[hint] && game.limit > 30)
        game.limit += 30 * hints[hint][1];
      }
     }
    }
   }
   if (game.closed) {
    if (game.prop[OYSTER] < 0 && toting(OYSTER)) pspeak(OYSTER, 1);
    for (int i=1; i<65; i++)
     if (toting(i) && game.prop[i] < 0) game.prop[i] = -1 - game.prop[i];
   }
   game.wzdark = dark();
   if (0 < game.knifeloc && game.knifeloc != game.loc) game.knifeloc = 0;
   ran(1);  /* Kick RNG */
   getin(word1, in1, word2, in2);

  case 2608:
   game.foobar = game.foobar > 0 ? -game.foobar : 0;
#ifdef ADVMAGIC
   if (game.turns == 0 && strcmp(word1, "MAGIC") == 0
    && strcmp(word2, "MODE") == 0) maint();
#endif
   game.turns++;
#ifdef ADVMAGIC
   if (demo && game.turns >= mage.shortGame) {
    mspeak(1);
    normend();
   }
#endif
   if (verb == SAY) {
    if (*word2) verb = 0;
    else {vsay(); return; }
   }
   if (game.tally == 0 && 15 <= game.loc && game.loc != 33) game.clock1--;
   if (game.clock1 == 0) {
    game.prop[GRATE] = game.prop[FISSUR] = 0;
    for (int i=0; i<6; i++) {
     game.dloc[i] = 0;
     game.dseen[i] = false;
    }
    move(TROLL, 0);
    move(TROLL+100, 0);
    move(TROLL2, 117);  /* There are no trolls in _Troll 2_. */
    move(TROLL2+100, 122);
    juggle(CHASM);
    if (game.prop[BEAR] != 3) destroy(BEAR);
    game.prop[CHAIN] = game.prop[AXE] = 0;
    game.fixed[CHAIN] = game.fixed[AXE] = 0;
    rspeak(129);
    game.clock1 = -1;
    game.closing = true;
    togoto = 19999;
    return;
   }
   if (game.clock1 < 0) game.clock2--;
   if (game.clock2 == 0) {
    game.prop[BOTTLE] = put(BOTTLE, 115, 1);
    game.prop[PLANT] = put(PLANT, 115, 0);
    game.prop[OYSTER] = put(OYSTER, 115, 0);
    game.prop[LAMP] = put(LAMP, 115, 0);
    game.prop[ROD] = put(ROD, 115, 0);
    game.prop[DWARF] = put(DWARF, 115, 0);
    game.loc = game.oldloc = game.newloc = 115;
    put(GRATE, 116, 0);
    game.prop[SNAKE] = put(SNAKE, 116, 1);
    game.prop[BIRD] = put(BIRD, 116, 1);
    game.prop[CAGE] = put(CAGE, 116, 0);
    game.prop[ROD2] = put(ROD2, 116, 0);
    game.prop[PILLOW] = put(PILLOW, 116, 0);
    game.prop[MIRROR] = put(MIRROR, 115, 0);
    game.fixed[MIRROR] = 116;
    for (int i=1; i<65; i++) if (toting(i)) destroy(i);
    rspeak(132);
    game.closed = true;
    togoto = 2;
    return;
   }
   if (game.prop[LAMP] == 1) game.limit--;
   if (game.limit <= 30 && here(BATTER) && game.prop[BATTER] == 0
    && here(LAMP)) {
    rspeak(188);
    game.prop[BATTER] = 1;
    if (toting(BATTER)) drop(BATTER, game.loc);
    game.limit += 2500;
    game.lmwarn = false;
   } else if (game.limit == 0) {
    game.limit = -1;
    game.prop[LAMP] = 0;
    if (here(LAMP)) rspeak(184);
   } else if (game.limit < 0 && game.loc <= 8) {
    rspeak(185);
    gaveup = true;
    normend();
   } else if (game.limit <= 30 && !game.lmwarn && here(LAMP)) {
    game.lmwarn = true;
    rspeak(game.place[BATTER] == 0 ? 183 : game.prop[BATTER] == 1 ? 189 : 187);
   }

  case 19999:
   if (strcmp(word1, "ENTER") == 0 && (strcmp(word2, "STREA") == 0
    || strcmp(word2, "WATER") == 0)) {
    rspeak(liqloc(game.loc) == WATER ? 70 : 43);
    togoto = 2012;
    return;
   }
   if (strcmp(word1, "ENTER") == 0 && *word2) {
    strncpy(word1, word2, 5);
    strncpy(in1, in2, MAX_INPUT_LENGTH);
    *word2 = *in2 = 0;
   } else if ((strcmp(word1, "WATER") == 0 || strcmp(word1, "OIL") == 0)
    && (strcmp(word2, "PLANT") == 0 || strcmp(word2, "DOOR") == 0)) {
    if (at(vocab(word2, 1))) strncpy(word2, "POUR", 5);
   }

  case 2610:
   if (strcmp(word1, "WEST") == 0 && ++game.iwest == 10) rspeak(17);

  case 2630: {
   int i = vocab(word1, -1);
   if (i == -1) {
    rspeak(pct(20) ? 61 : pct(20) ? 13 : 60);
    togoto = 2600;
    return;
   }
   int k = i % 1000;
   switch (i / 1000) {
    case 0: domove(k); break;
    case 1:
     obj = k;
     if (game.fixed[obj] == game.loc || here(obj)) doaction();
     else {
     /* You would think that this part would be better expressed as a "switch"
      * block, but that turns out to be far less concise. */
      if (obj == GRATE) {
       if (game.loc == 1 || game.loc == 4 || game.loc == 7) k = DEPRESSION;
       if (9 < game.loc && game.loc < 15) k = ENTRANCE;
       if (k != GRATE) domove(k);
       else if ((verb == FIND || verb == INVENT) && !*word2) doaction();
       else {printf("\nI see no %s here.\n", in1); togoto = 2012; }
      } else if ((obj == DWARF && game.dflag >= 2 && dwarfHere())
		 || (obj == liq() && here(BOTTLE))
		 || obj == liqloc(game.loc)) {
       doaction();
      } else if (obj == PLANT && at(PLANT2) && game.prop[PLANT2] != 0) {
       obj = PLANT2;
       doaction();
      } else if (obj == KNIFE && game.knifeloc == game.loc) {
       game.knifeloc = -1;
       rspeak(116);
       togoto = 2012;
      } else if (obj == ROD && here(ROD2)) {obj = ROD2; doaction(); }
      else if ((verb == FIND || verb == INVENT) && !*word2) doaction();
      else {printf("\nI see no %s here.\n", in1); togoto = 2012; }
     }
     break;
    case 2:
     verb = k;
     if (verb == SAY || verb == SUSPEND || verb == RESUME) obj = *word2;
     /* This assignment just indicates whether an object was supplied. */
     else if (*word2) {
      strncpy(word1, word2, 5);
      strncpy(in1, in2, MAX_INPUT_LENGTH);
      *word2 = *in2 = 0;
      togoto = 2610;
      return;
     }
     obj ? transitive() : intransitive();
     break;
    case 3: rspeak(k); togoto = 2012; break;
    default: bug(22);
   }
  }
 }
}
