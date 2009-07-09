#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "advconst.h"
#include "advdecl.h"

void domove(int motion) {
 togoto = 2;
 newloc = loc;
 if (travel[loc][0][0] == -1) bug(26);
 switch (motion) {
  case NULLMOVE: return;
  case BACK: {
   int k = forced(oldloc) ? oldloc2 : oldloc;
   oldloc2 = oldloc;
   oldloc = loc;
   if (k == loc) rspeak(91);
   else {
    int k2 = 0;
    for (int kk=0; travel[loc][kk][0] != -1; kk++) {
     int ll = travel[loc][kk][0] % 1000;
     if (ll == k) {
      dotrav(travel[loc][kk][1]);
      return;
     } else if (ll <= 300) {
      if (forced(ll) && travel[ll][0][0] % 1000 == k) k2 = kk;
     }
    }
    if (k2 != 0) dotrav(travel[loc][k2][1]);
    else rspeak(140);
   }
   break;
  }
  case LOOK:
   if (detail++ < 3) rspeak(15);
   wzdark = false;
   abb[loc] = 0;
   break;
  case CAVE:
   rspeak(loc < 8 ? 57 : 58);
   break;
  default:
   oldloc2 = oldloc;
   oldloc = loc;
   dotrav(motion);
 }
}

void dotrav(int motion) {
 int rdest = -1;
 bool matched = false;
 for (int kk=0; travel[loc][kk][0] != -1; kk++) {
  if (!matched) {
   for (int i=1; travel[loc][kk][i] != -1; i++) {
    if (travel[loc][kk][i] == 1 || travel[loc][kk][i] == motion) { 
     matched = true;
     break;
    }
   }
  }
  if (matched) {
   int ll = travel[loc][kk][0];
   int rcond = ll / 1000;
   int robject = rcond % 100;
   if (rcond == 0 || rcond == 100) rdest = ll % 1000;
   else if (rcond < 100) {if (pct(rcond)) rdest = ll % 1000; }
   else if (rcond <= 200) {if (toting(robject)) rdest = ll % 1000; }
   else if (rcond <= 300) {
    if (toting(robject) || at(robject)) rdest = ll % 1000;
   } else if (prop[robject] != rcond / 100 - 3) rdest = ll % 1000;
   if (rdest != -1) break;
  }
 }
 if (rdest == -1) {
  switch (motion) {
   case 29: case 30: case 43: case 44: case 45:
   case 46: case 47: case 48: case 49: case 50:
    rspeak(9);
    break;
   case 7: case 36: case 37: rspeak(10); break;
   case 11: case 19: rspeak(11); break;
   case 62: case 65: rspeak(42); break;
   case 17: rspeak(80); break;
   default:
    rspeak(verb == FIND || verb == INVENT ? 59 : 12);
  }
 } else if (0 <= rdest && rdest <= 300) newloc = rdest;
 else if (rdest == 301) {
  if (!holding || holding == 1 && toting(EMERALD)) newloc = 99 + 100 - loc;
  else {newloc = loc; rspeak(117); }
 } else if (rdest == 302) {
  drop(EMERALD, loc);
  newloc = (loc == 33 ? 100 : 33);
 } else if (rdest == 303) {
  if (prop[TROLL] == 1) {
   pspeak(TROLL, 1);
   prop[TROLL] = 0;
   move(TROLL2, 0);
   move(TROLL2+100, 0);
   move(TROLL, 117);
   move(TROLL+100, 122);
   juggle(CHASM);
   newloc = loc;
  } else {
   newloc = (loc == 117 ? 122 : 117);
   if (prop[TROLL] == 0) prop[TROLL] = 1;
   if (toting(BEAR)) {
    rspeak(162);
    prop[CHASM] = 1;
    prop[TROLL] = 2;
    drop(BEAR, newloc);
    fixed[BEAR] = -1;
    prop[BEAR] = 3;
    if (prop[SPICES] < 0) tally2++;
    oldloc2 = newloc;
    death();
   }
  }
 } else if (rdest > 500) rspeak(rdest-500);
 else bug(20);
}

void death(void) {
 if (closing) {
  rspeak(131);
  numdie++;
  normend();
 } else {
  bool yea = yes(81 + numdie*2, 82 + numdie*2, 54);
  numdie++;
  if (numdie == MAXDIE || !yea) normend();
  place[WATER] = place[OIL] = 0;
  if (toting(LAMP)) prop[LAMP] = 0;
  for (int i=64; i>0; i--) {
   if (toting(i)) drop(i, i == LAMP ? 1 : oldloc2);
  }
  loc = oldloc = 3;
  togoto = 2000;
 }
}

int score(bool scoring) {
 int scr = 0;
 for (int i=50; i<65; i++) {
  if (prop[i] >= 0) scr += 2;
  if (place[i] == 3 && prop[i] == 0)
   scr += (i == CHEST ? 12 : i > CHEST ? 14 : 10);
 }
 scr += (MAXDIE - numdie) * 10;
 if (!scoring && !gaveup) scr += 4;
 if (dflag != 0) scr += 25;
 if (closing) scr += 25;
 if (closed) {
  switch (bonus) {
   case 0: scr += 10; break;
   case 133: scr += 45; break;
   case 134: scr += 30; break;
   case 135: scr += 25; break;
  }
 }
 if (place[MAGZIN] == 108) scr++;
 scr += 2;
 for (int i=1; i<10; i++) if (hinted[i]) scr -= hints[i][1];
 return scr;
}

void normend(void) {
 int scr = score(false);
 printf("You scored %d out of a possible 350 using %d turns.\n", scr, turns);
 int i;
 for (i=0; classes[i].score != 0; i++) if (classes[i].score >= scr) break;
 if (classes[i].score != 0) {
  speak(classes[i].rank);
  if (classes[i+1].score != 0) {
   int diff = classes[i+1].score - scr + 1;
   printf("To achieve the next higher rating, you need %d more point%s.\n",
    diff, diff == 1 ? "" : "s");
  } else {
   puts("To achieve the next higher rating would be a neat trick!");
   puts("Congratulations!!");
  }
 } else puts("You just went off my scale!!");
 exit(0);
}

void doaction(void) {
 if (*word2) {
  strcpy(word1, word2);
  strcpy(in1, in2);
  *word2 = *in2 = 0;
  togoto = 2610;
 } else if (verb) transitive();
 else {
  printf("What do you want to do with the %s?\n", in1);
  togoto = 2600;
 }
}

bool dwarfHere(void) {
 for (int i=0; i<5; i++) if (dloc[i] == loc) return true;
 return false;
}
