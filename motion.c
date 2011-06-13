#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "advconst.h"
#include "advdecl.h"

void domove(int motion) {
 togoto = 2;
 game.newloc = game.loc;
 if (travel[game.loc][0][0] == -1) bug(26);
 switch (motion) {
  case NULLMOVE: return;
  case BACK: {
   int k = forced(game.oldloc) ? game.oldloc2 : game.oldloc;
   game.oldloc2 = game.oldloc;
   game.oldloc = game.loc;
   if (k == game.loc) rspeak(91);
   else {
    int k2 = 0;
    for (int kk = 0; travel[game.loc][kk][0] != -1; kk++) {
     int ll = travel[game.loc][kk][0] % 1000;
     if (ll == k) {
      dotrav(travel[game.loc][kk][1]);
      return;
     } else if (ll <= 300) {
      if (forced(ll) && travel[ll][0][0] % 1000 == k) k2 = kk;
     }
    }
    if (k2 != 0) dotrav(travel[game.loc][k2][1]);
    else rspeak(140);
   }
   break;
  }
  case LOOK:
   if (game.detail++ < 3) rspeak(15);
   game.wzdark = false;
   game.abb[game.loc] = 0;
   break;
  case CAVE:
   rspeak(game.loc < 8 ? 57 : 58);
   break;
  default:
   game.oldloc2 = game.oldloc;
   game.oldloc = game.loc;
   dotrav(motion);
 }
}

void dotrav(int motion) {
 int rdest = -1;
 bool matched = false;
 for (int kk=0; travel[game.loc][kk][0] != -1; kk++) {
  if (!matched) {
   for (int i=1; travel[game.loc][kk][i] != -1; i++) {
    if (travel[game.loc][kk][i] == 1 || travel[game.loc][kk][i] == motion) {
     matched = true;
     break;
    }
   }
  }
  if (matched) {
   int ll = travel[game.loc][kk][0];
   int rcond = ll / 1000;
   int robject = rcond % 100;
   if (rcond == 0 || rcond == 100) rdest = ll % 1000;
   else if (rcond < 100) {if (pct(rcond)) rdest = ll % 1000; }
   else if (rcond <= 200) {if (toting(robject)) rdest = ll % 1000; }
   else if (rcond <= 300) {
    if (toting(robject) || at(robject)) rdest = ll % 1000;
   } else if (game.prop[robject] != rcond / 100 - 3) rdest = ll % 1000;
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
 } else if (0 <= rdest && rdest <= 300) game.newloc = rdest;
 else if (rdest == 301) {
  if (!game.holding || game.holding == 1 && toting(EMERALD))
   game.newloc = 99 + 100 - game.loc;
  else {game.newloc = game.loc; rspeak(117); }
 } else if (rdest == 302) {
  drop(EMERALD, game.loc);
  game.newloc = (game.loc == 33 ? 100 : 33);
 } else if (rdest == 303) {
  if (game.prop[TROLL] == 1) {
   pspeak(TROLL, 1);
   game.prop[TROLL] = 0;
   move(TROLL2, 0);
   move(TROLL2+100, 0);
   move(TROLL, 117);
   move(TROLL+100, 122);
   juggle(CHASM);
   game.newloc = game.loc;
  } else {
   game.newloc = (game.loc == 117 ? 122 : 117);
   if (game.prop[TROLL] == 0) game.prop[TROLL] = 1;
   if (toting(BEAR)) {
    rspeak(162);
    game.prop[CHASM] = 1;
    game.prop[TROLL] = 2;
    drop(BEAR, game.newloc);
    game.fixed[BEAR] = -1;
    game.prop[BEAR] = 3;
    if (game.prop[SPICES] < 0) game.tally2++;
    game.oldloc2 = game.newloc;
    death();
   }
  }
 } else if (rdest > 500) rspeak(rdest-500);
 else bug(20);
}

void death(void) {
 if (game.closing) {
  rspeak(131);
  game.numdie++;
  normend();
 } else {
  bool yea = yes(81 + game.numdie*2, 82 + game.numdie*2, 54);
  game.numdie++;
  if (game.numdie == MAXDIE || !yea) normend();
  game.place[WATER] = game.place[OIL] = 0;
  if (toting(LAMP)) game.prop[LAMP] = 0;
  for (int i=64; i>0; i--) {
   if (toting(i)) drop(i, i == LAMP ? 1 : game.oldloc2);
  }
  game.loc = game.oldloc = 3;
  togoto = 2000;
 }
}

int score(bool scoring) {
 int scr = 0;
 for (int i=50; i<65; i++) {
  if (game.prop[i] >= 0) scr += 2;
  if (game.place[i] == 3 && game.prop[i] == 0)
   scr += (i == CHEST ? 12 : i > CHEST ? 14 : 10);
 }
 scr += (MAXDIE - game.numdie) * 10;
 if (!scoring && !gaveup) scr += 4;
 if (game.dflag != 0) scr += 25;
 if (game.closing) scr += 25;
 if (game.closed) {
  switch (bonus) {
   case 0: scr += 10; break;
   case 133: scr += 45; break;
   case 134: scr += 30; break;
   case 135: scr += 25; break;
  }
 }
 if (game.place[MAGZIN] == 108) scr++;
 scr += 2;
 for (int i=1; i<10; i++) if (game.hinted[i]) scr -= hints[i][1];
 return scr;
}

void normend(void) {
 int scr = score(false);
 printf("\n\n\nYou scored %d out of a possible 350 using %d turn%s.\n",
  scr, game.turns, game.turns == 1 ? "" : "s");
 int i;
 for (i=0; classes[i].score != 0; i++) if (classes[i].score >= scr) break;
 if (classes[i].score != 0) {
  speak(classes[i].rank);
  if (classes[i+1].score != 0) {
   int diff = classes[i].score - scr + 1;
   printf("\nTo achieve the next higher rating, you need %d more point%s.\n\n",
    diff, diff == 1 ? "" : "s");
  } else {
   printf("\nTo achieve the next higher rating would be a neat trick!\n\n");
   printf("Congratulations!!\n\n");
  }
 } else printf("\nYou just went off my scale!!\n\n");
 exit(0);
}

void doaction(void) {
 if (*word2) {
  strncpy(word1, word2, 5);
  strncpy(in1, in2, MAX_INPUT_LENGTH);
  *word2 = *in2 = 0;
  togoto = 2610;
 } else if (verb) transitive();
 else {
  printf("\nWhat do you want to do with the %s?\n", in1);
  togoto = 2600;
 }
}

bool dwarfHere(void) {
 for (int i=0; i<5; i++) if (game.dloc[i] == game.loc) return true;
 return false;
}
