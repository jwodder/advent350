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
 * wkday is for weekdays, wkend for weekends, and holid for holidays (days with
 * special hours).  If element N of an array is true, then the hour N:00
 * through N:59 is considered "prime time," i.e., the cave is closed then. */
bool wkday[24] = {[8] = true, true, true, true, true, true, true, true, true,
 true};  /* The remaining elements are initialized to false. */
bool wkend[24]; /* all false */
bool holid[24]; /* all false */

int hbegin = 0, hend = -1;	/* start & end of next holiday */
char hname[21];			/* name of next holiday */
int shortGame = 30;		/* turns allowed in a short/demo game */
char magic[6] = "DWARF";	/* magic word */
int magnm = 11111;		/* magic number */
int latency = 90;		/* time required to wait after saving */
char msg[500];			/* MOTD, initially null */
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
int prop[65] = {
 /* Elements 0 through 49 are implicitly set to zero. */
 [50] = -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};
int abb[141];
int hintlc[10];
bool hinted[10];
int dloc[6] = {19, 27, 33, 44, 64, CHLOC};
int odloc[6];
bool dseen[6];
int dflag = 0, dkill = 0;
int place[65];
int fixed[65];
int atloc[65];
int link[65];
#ifdef ADVMAGIC
int saved = -1, savet = 0;
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
   if (0 < newloc && newloc < 9 && closing) {
    rspeak(130);
    newloc = loc;
    if (!panic) clock2 = 15;
    panic = true;
   }
   if (newloc != loc && !forced(loc) && !bitset(loc, 3)
    && { @odloc[$^i] == $newloc && @dseen[$^i] }(any ^5) {
    newloc = loc;
    rspeak(2);
   }
   loc = newloc;
   # Dwarven logic:
   togoto = 2000;  /* in preparation for an early `return' */
   if (loc == 0 || forced(loc) || bitset(newloc, 3)) return;
   if (dflag == 0) {
    if (loc >= 15) dflag = 1;
    return;
   }
   if (dflag == 1) {
    if (loc < 15 || pct(95)) return;
    dflag = 2;
    if (pct(50)) dloc[ran(5)] = 0;
    /* Yes, this is supposed to be done twice. */
    if (pct(50)) dloc[ran(5)] = 0;
    for (int i=0; i<5; i++) {
     if (dloc[i] == loc) dloc[i] = 18;
     odloc[i] = dloc[i];
    }
    rspeak(3);
    drop(AXE, loc);
    return;
   }
   int dtotal = 0, attack = 0, stick = 0;
   for (int i=0; i<6; i++) {  /* The individual dwarven movement loop */
    if (dloc[i] == 0) continue;
    int kk=0, tk;
    for (int j=0; j<MAXROUTE; j++) {
     if (travel[dloc[i]][j][0] == -1) break;
     int newloc = travel[dloc[i]][j][0] % 1000;
     if (15 <= newloc && newloc <= 300 && newloc != odloc[i]
      && newloc != dloc[i] && !forced(newloc) && !(i == 5 && bitset(newloc, 3))
      && travel[dloc[i]][j][0] / 1000 != 100 && ran(++kk) == 0) tk = newloc;
    }
    if (ran(kk+1) == 0) tk = odloc[i];
    odloc[i] = dloc[i];
    dloc[i] = tk;
    dseen[i] = (dseen[i] && loc >= 15) || dloc[i] == loc || odloc[i] == loc;
    if (dseen[i]) {
     dloc[i] = loc;
     if (i == 5) {
      /* Pirate logic: */
      if (loc == CHLOC || prop[CHEST] >= 0) continue;
      bool k = false;
      for (int j=50; j<65; j++) {
       if (j == PYRAM && (loc == 100 || loc == 101)) continue;
       if (toting(j)) {
        rspeak(128);
	if (place[MESSAG] == 0) move(CHEST, CHLOC);
	move(MESSAG, CHLOC2);
	for (j=50; j<65; j++) {
	 if (j == PYRAM && (loc == 100 || loc == 101)) continue;
	 if (at(j) && fixed[j] == 0) carry(j, loc);
	 if (toting(j)) drop(j, CHLOC);
	}
	dloc[5] = odloc[5] = CHLOC;
	dseen[5] = false;

	/* GOTO next dwarfLoop */

       }
       if (here(j)) k = true;
      }
      if (tally == tally2 + 1 && !k && place[CHEST] == 0 && here(LAMP)
       && prop[LAMP] == 1) {
       rspeak(186);
       move(CHEST, CHLOC);
       move(MESSAG, CHLOC2);
       dloc[5] = odloc[5] = CHLOC;
       dseen[5] = false;
      } else if (odloc[5] != dloc[5] && pct(20)) {rspeak(127); }
     } else {
      dtotal++;
      if (odloc[i] == dloc[i]) {
       attack++;
       if (knifeloc >= 0) knifeloc = loc;
       if (ran(1000) < 95 * (dflag - 2)) stick++;
      }
     }
    }
   } /* end of individual dwarven movement loop */
   if (dtotal == 0) return;
   if (dtotal == 1) rspeak(4);
   else
    printf("There are %d threatening little dwarves in the room with you.\n",
     dtotal);
   if (attack == 0) return;
   if (dflag == 2) dflag = 3;
   int k;
   if (attack == 1) {rspeak(5); k = 52; }
   else {printf("%d of them throw knives at you!\n", attack); k = 6; }
   if (stick <= 1) {
    rspeak(k + stick);
    if (stick == 0) return;
   } else printf("%d of them get you!\n", stick);
   oldloc2 = loc;
   death();
  /* If the player is reincarnated after being killed by a dwarf, they GOTO
   * label 2000 using fallthrough rather than with any special flow control. */

  case 2000:
   if (loc == 0) {death(); return; }
   const char* kk = shortdesc[loc];
   if (abb[loc] % abbnum == 0 || kk == NULL) kk = longdesc[loc];
   if (!forced(loc) && dark()) {
    if (wzdark && pct(35)) {
     rspeak(23);
     oldloc2 = loc;
     death();
     return;
    }
    kk = rmsg[16];
   }
   if (toting(BEAR)) rspeak(141);
   speak(kk);
   if (forced(loc)) {domove(1); return; }
   if (loc == 33 && pct(25) && !closing) rspeak(8);
   if (!dark()) {
    abb[loc]++;
    for (int i = atloc[loc]; i != 0; i = link[i]) {
     int obj = i > 100 ? i - 100 : i;
     if (obj == STEPS && toting(NUGGET)) continue;
     if (prop[obj] < 0) {
      if (closed) continue;
      prop[obj] = obj == RUG || obj == CHAIN;
      tally--;
      if (tally == tally2 && tally != 0) limit = limit < 35 ? limit : 35;
     }
     pspeak(obj, obj == STEPS && loc == fixed[STEPS] ? 1 : prop[obj]);
    }
   }

  case 2012: verb = obj = 0;

  case 2600:
   for (int hint = 4; hint<10; hint++) {
    if (hinted[hint]) continue;
    if (!bitset(loc, hint)) hintlc[hint] = -1;
    hintlc[hint]++;
    if (hintlc[hint] >= hints[hint][0]) {
     bool hintable = true;
     switch (hint) {
      case 4:
       if (prop[GRATE] != 0 || here(KEYS)) {
        hintlc[hint] = 0;
	hintable = false;
       }
       break;
      case 5:
       if (!here(BIRD) || !toting(ROD) || obj != BIRD) hintable = false;
       break;
      case 6:
       if (!here(SNAKE) || here(BIRD)) {hintlc[hint] = 0; hintable = false; }
       break;
      case 7:
       if (atloc[loc] || atloc[oldloc] || atloc[oldloc2] || holding <= 1) {
        hintlc[hint] = 0;
	hintable = false;
       }
       break;
      case 8:
       if (prop[EMERALD] == -1 || prop[PYRAM] != -1) {
        hintlc[hint] = 0;
	hintable = false;
       }
       break;
     }
     if (hintable) {
      hintlc[hint] = 0;
      if (yes(hints[hint][2], 0, 54)) {
       printf("I am prepared to give you a hint, but it will cost you %d"
        " points.\n", hints[hint][1]);
       hinted[hint] = yes(175, hints[hint][3], 54);
       if (hinted[hint] && limit > 30) limit += 30 * hints[hint][1];
      }
     }
    }
   }
   if (closed) {
    if (prop[OYSTER] < 0 && toting(OYSTER)) pspeak(OYSTER, 1);
    for (int i=1; i<65; i++)
     if (toting(i) && prop[i] < 0) prop[i] = -1 - prop[i];
   }
   wzdark = dark();
   if (0 < knifeloc && knifeloc != loc) knifeloc = 0;
   getin(word1, in1, word2, in2);

  case 2608:
   foobar = foobar > 0 ? -foobar : 0;
#ifdef ADVMAGIC
   if (turns == 0 && strcmp(word1, "MAGIC") == 0 && strcmp(word2, "MODE") == 0)
    maint();
#endif
   turns++;
#ifdef ADVMAGIC
   if (demo && turns >= shortGame) {
    mspeak(1);
    normend();
   }
#endif
   if (verb == SAY) {
    if (*word2) verb = 0;
    else {vsay(); return; }
   }
   if (tally == 0 && 15 <= loc && loc != 33) clock1--;
   if (clock1 == 0) {
    prop[GRATE] = prop[FISSUR] = 0;
    for (int i=0; i<6; i++) dseen[i] = dloc[i] = 0;
    move(TROLL, 0);
    move(TROLL+100, 0);
    move(TROLL2, 117);  /* There are no trolls in _Troll 2_. */
    move(TROLL2+100, 122);
    juggle(CHASM);
    if (prop[BEAR] != 3) destroy(BEAR);
    prop[CHAIN] = prop[AXE] = 0;
    fixed[CHAIN] = fixed[AXE] = 0;
    rspeak(129);
    clock1 = -1;
    closing = true;
    togoto = 19999;
    return;
   }
   if (clock1 < 0) clock2--;
   if (clock2 == 0) {
    prop[BOTTLE] = put(BOTTLE, 115, 1);
    prop[PLANT] = put(PLANT, 115, 0);
    prop[OYSTER] = put(OYSTER, 115, 0);
    prop[LAMP] = put(LAMP, 115, 0);
    prop[ROD] = put(ROD, 115, 0);
    prop[DWARF] = put(DWARF, 115, 0);
    loc = oldloc = newloc = 115;
    put(GRATE, 116, 0);
    prop[SNAKE] = put(SNAKE, 116, 1);
    prop[BIRD] = put(BIRD, 116, 1);
    prop[CAGE] = put(CAGE, 116, 0);
    prop[ROD2] = put(ROD2, 116, 0);
    prop[PILLOW] = put(PILLOW, 116, 0);
    prop[MIRROR] = put(MIRROR, 115, 0);
    fixed[MIRROR] = 116;
    for (int i=1; i<65; i++) if (toting(i)) destroy(i);
    rspeak(132);
    closed = true;
    togoto = 2;
    return;
   }
   if (prop[LAMP] == 1) limit--;
   if (limit <= 30 && here(BATTER) && prop[BATTER] == 0 && here(LAMP)) {
    rspeak(188);
    prop[BATTER] = 1;
    if (toting(BATTER)) drop(BATTER, loc);
    limit += 2500;
    lmwarn = false;
   } else if (limit == 0) {
    limit = -1;
    prop[LAMP] = 0;
    if (here(LAMP)) rspeak(184);
   } else if (limit < 0 && loc <= 8) {
    rspeak(185);
    gaveup = true;
    normend();
   } else if (limit <= 30 && !lmwarn && here(LAMP)) {
    lmwarn = true;
    rspeak(place[BATTER] == 0 ? 183 : prop[BATTER] == 1 ? 189 : 187);
   }

  case 19999:
   if (strcmp(word1, "ENTER") == 0 && (strcmp(word2, "STREA") == 0
    || strcmp(word2, "WATER") == 0)) {
    rspeak(liqloc(loc) == WATER ? 70 : 43);
    togoto = 2012;
    return;
   }
   if (strcmp(word1, "ENTER") == 0 && *word2) {
    strcpy(word1, word2);
    strcpy(in1, in2);
    *word2 = *in2 = 0;
   } else if ((strcmp(word1, "WATER") == 0 || strcmp(word1, "OIL") == 0)
    && (strcmp(word2, "PLANT") == 0 || strcmp(word2, "DOOR") == 0)) {
    if (at(vocab(word2, 1))) strcpy(word2, "POUR");
   }

  case 2610:
   if (strcmp(word1, "WEST") == 0 && ++iwest == 10) rspeak(17);

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
     if (fixed[obj] == loc || here(obj)) doaction();
     else {
     /* You would think that this part would be better expressed as a "switch"
      * block, but that turns out to be far less concise. */
      if (obj == GRATE) {
       if (loc == 1 || loc == 4 || loc == 7) k = DEPRESSION;
       if (9 < loc && loc < 15) k = ENTRANCE;
       if (k != GRATE) domove(k);
       else if ((verb == FIND || verb == INVENT) && !*word2) doaction();
       else {printf("I see no %s here.\n", in1); togoto = 2012; }
      } else if (obj == DWARF && dflag >= 2 && /*** @dloc[^5].any == $loc ***/
       || obj == liq() && here(BOTTLE) || obj == liqloc(loc)) doaction();
      else if (obj == PLANT && at(PLANT2) && prop[PLANT2] != 0) {
       obj = PLANT2;
       doaction();
      } else if (obj == KNIFE && knifeloc == loc) {
       knifeloc = -1;
       rspeak(116);
       togoto = 2012;
      } else if (obj == ROD && here(ROD2)) {obj = ROD2; doaction(); }
      else if ((verb == FIND || verb == INVENT) && !*word2) doaction();
      else {printf("I see no %s here.\n", in1); togoto = 2012; }
     }
     break;
    case 2:
     verb = k;
     if (verb == SAY || verb == SUSPEND || verb == RESUME) obj = *word2;
     /* This assignment just indicates whether an object was supplied. */
     else if (*word2) {
      strcpy(word1, word2);
      strcpy(in1, in2);
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
