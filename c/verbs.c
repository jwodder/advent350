/* Verb functions */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "advconfig.h"
#include "advconst.h"
#include "advdecl.h"

static bool freeable;

void intransitive(void) {
/* Label 4080 (intransitive verb handling): */
/* As this function is only called at a single point in the code, it doesn't
 * actually need to be a separate routine, but I think we can all agree that
 * turn() is complicated enough as it is without stuffing yet another 140-line
 * "switch" block inside of it. */
 togoto = 2012;
 switch (verb) {
  case NOTHING: rspeak(54); break;
  case WALK: rspeak(actspk[verb]); break;

  case DROP: case SAY: case WAVE: case CALM: case RUB:
  case THROW: case FIND: case FEED: case BREAK: case WAKE:
   printf("\n%s what?\n", in1);
   obj = 0;
   togoto = 2600;
   break;

  case TAKE:
   if (!(game.atloc[game.loc] && game.link[game.atloc[game.loc]] == 0)
       || (game.dflag >= 2 && dwarfHere())) {
    printf("\n%s what?\n", in1);
    obj = 0;
    togoto = 2600;
   } else {
    obj = game.atloc[game.loc];
    vtake();
   }
   break;

  case OPEN:
  case LOCK:
   if (here(CLAM)) obj = CLAM;
   if (here(OYSTER)) obj = OYSTER;
   if (at(DOOR)) obj = DOOR;
   if (at(GRATE)) obj = GRATE;
   if (obj != 0 && here(CHAIN)) {
    printf("\n%s what?\n", in1);
    obj = 0;
    togoto = 2600;
   } else {
    if (here(CHAIN)) obj = CHAIN;
    if (obj == 0) rspeak(28);
    else vopen();
   }
   break;

  case EAT:
   if (here(FOOD)) {
    destroy(FOOD);
    rspeak(72);
   } else {
    printf("\n%s what?\n", in1);
    obj = 0;
    togoto = 2600;
   }
   break;

  case QUIT:
   /* Yes, this is supposed to be an assignment: */
   if ((gaveup = yes(22, 54, 54))) normend();
   break;

  case INVENT: {
   int spk = 98;
   for (int i=0; i<65; i++) {
    if (i == BEAR || !toting(i)) continue;
    if (spk == 98) rspeak(99);
    blklin = false;
    pspeak(i, -1);
    blklin = true;
    spk = 0;
   }
   if (toting(BEAR)) spk = 141;
   rspeak(spk);
   break;
  }

  case SCORE: {
   int scr = score(true);
   printf("\nIf you were to quit now, you would score %d out of a possible"
	  " 350.\n", scr);
   /* Yes, this is supposed to be an assignment: */
   if ((gaveup = yes(143, 54, 54))) normend();
   break;
  }

  case FOO: {
   int k = vocab(word1, 3);
   if (game.foobar == 1-k) {
    game.foobar = k;
    if (k != 4) {rspeak(54); return; }
    game.foobar = 0;
    if (game.place[EGGS] == 92 || (toting(EGGS) && game.loc == 92)) rspeak(42);
    else {
     if (game.place[EGGS] == 0 && game.place[TROLL] == 0
      && game.prop[TROLL] == 0) game.prop[TROLL] = 1;
     k = (game.loc == 92 ? 0 : here(EGGS) ? 1 : 2);
     move(EGGS, 92);
     pspeak(EGGS, k);
    }
   } else rspeak(game.foobar ? 151 : 42);
   break;
  }

  case BRIEF:
   game.abbnum = 10000;
   game.detail = 3;
   rspeak(156);
   break;

  case READ:
   if (here(MAGZIN)) obj = MAGZIN;
   if (here(TABLET)) obj = obj * 100 + TABLET;
   if (here(MESSAG)) obj = obj * 100 + MESSAG;
   if (game.closed && toting(OYSTER)) obj = OYSTER;
   if (obj > 100 || obj == 0 || dark()) {
    printf("\n%s what?\n", in1);
    obj = 0;
    togoto = 2600;
   } else vread();
   break;

  case SUSPEND: {
   char* file = defaultSaveFile();
   vsuspend(file);
   if (freeable) free(file);
   break;
  }

  case RESUME: {
   char* file = defaultSaveFile();
   vresume(file);
   if (freeable) free(file);
   break;
  }

  case HOURS:
#ifdef ADVMAGIC
   mspeak(6);
   hours();
#else
   printf("\nColossal Cave is open all day, every day.\n");
#endif
   break;

  case ON: von(); break;
  case OFF: voff(); break;
  case KILL: vkill(); break;
  case POUR: vpour(); break;
  case DRINK: vdrink(); break;
  case FILL: vfill(); break;
  case BLAST: vblast(); break;
  default: bug(23);
 }
}

void transitive(void) {
/* Label 4090 (transitive verb handling): */
 togoto = 2012;
 switch (verb) {
  case TAKE: vtake(); break;
  case DROP: vdrop(); break;
  case SAY: vsay(); break;
  case OPEN: case LOCK: vopen(); break;
  case NOTHING: rspeak(54); break;
  case ON: von(); break;
  case OFF: voff(); break;

  case WAVE:
   if (!toting(obj) && !(obj == ROD && toting(ROD2))) rspeak(29);
   else if (obj != ROD || !at(FISSUR) || !toting(obj) || game.closing)
    rspeak(actspk[verb]);
   else {
    game.prop[FISSUR] = 1 - game.prop[FISSUR];
    pspeak(FISSUR, 2 - game.prop[FISSUR]);
   }
   break;

  case CALM: case WALK: case QUIT: case SCORE:
  case FOO: case BRIEF: case HOURS:
   rspeak(actspk[verb]);
   break;

  case KILL: vkill(); break;
  case POUR: vpour(); break;

  case EAT:
   if (obj == FOOD) {destroy(FOOD); rspeak(72); }
   else if (obj == BIRD || obj == SNAKE || obj == CLAM || obj == OYSTER
    || obj == DWARF || obj == DRAGON || obj == TROLL || obj == BEAR)
    rspeak(71);
   else rspeak(actspk[verb]);
   break;

  case DRINK: vdrink(); break;
  case RUB: rspeak(obj == LAMP ? actspk[verb] : 76); break;

  case THROW:
   if (toting(ROD2) && obj == ROD && !toting(ROD)) obj = ROD2;
   if (!toting(obj)) rspeak(actspk[verb]);
   else if (50 <= obj && obj < 65 && at(TROLL)) {
    drop(obj, 0);
    move(TROLL, 0);
    move(TROLL+100, 0);
    drop(TROLL2, 117);
    drop(TROLL2+100, 122);
    juggle(CHASM);
    rspeak(159);
   } else if (obj == FOOD && here(BEAR)) {obj = BEAR; vfeed(); }
   else if (obj == AXE) {
    int i = -1;
    for (int j=0; j<5; j++) if (game.dloc[j] == game.loc) {i = j; break; }
    if (i != -1) {
     if (ran(3) == 0) rspeak(48);
     else {
      game.dseen[i] = false;
      game.dloc[i] = 0;
      rspeak(++game.dkill == 1 ? 149 : 47);
     }
     drop(AXE, game.loc);
     domove(NULLMOVE);
    } else if (at(DRAGON) && game.prop[DRAGON] == 0) {
     rspeak(152);
     drop(AXE, game.loc);
     domove(NULLMOVE);
    } else if (at(TROLL)) {
     rspeak(158);
     drop(AXE, game.loc);
     domove(NULLMOVE);
    } else if (here(BEAR) && game.prop[BEAR] == 0) {
     drop(AXE, game.loc);
     game.fixed[AXE] = -1;
     game.prop[AXE] = 1;
     juggle(BEAR);  /* Don't try this at home, kids. */
     rspeak(164);
    } else {obj = 0; vkill(); }
   } else vdrop();
   break;

  case FIND:
  case INVENT:
   if (toting(obj)) rspeak(24);
   else if (game.closed) rspeak(138);
   else if (obj == DWARF && game.dflag >= 2 && dwarfHere()) rspeak(94);
   else if (at(obj) || (liq() == obj && at(BOTTLE)) || obj == liqloc(game.loc))
    rspeak(94);
   else rspeak(actspk[verb]);
   break;

  case FEED: vfeed(); break;
  case FILL: vfill(); break;
  case BLAST: vblast(); break;
  case READ: vread(); break;

  case BREAK:
   if (obj == VASE && game.prop[VASE] == 0) {
    if (toting(VASE)) drop(VASE, game.loc);
    game.prop[VASE] = 2;
    game.fixed[VASE] = -1;
    rspeak(198);
   } else if (obj != MIRROR) rspeak(actspk[verb]);
   else if (!game.closed) rspeak(148);
   else {rspeak(197); rspeak(136); normend(); }
   break;

  case WAKE:
   if (obj == DWARF && game.closed) {rspeak(199); rspeak(136); normend(); }
   else rspeak(actspk[verb]);
   break;

  case SUSPEND: vsuspend(in2); break;
  case RESUME: vresume(in2); break;
  default: bug(24);
 }
}

void vtake(void) {
 if (toting(obj)) {rspeak(actspk[verb]); return; }
 int spk = 25;
 if (obj == PLANT && game.prop[PLANT] <= 0) spk = 115;
 if (obj == BEAR && game.prop[BEAR] == 1) spk = 169;
 if (obj == CHAIN && game.prop[BEAR] != 0) spk = 170;
 if (game.fixed[obj]) {rspeak(spk); return; }
 if (obj == WATER || obj == OIL) {
  if (!here(BOTTLE) || liq() != obj) {
   obj = BOTTLE;
   if (toting(BOTTLE) && game.prop[BOTTLE] == 1) vfill();
   else {
    if (game.prop[BOTTLE] != 1) spk = 105;
    if (!toting(BOTTLE)) spk = 104;
    rspeak(spk);
   }
   return;
  }
  obj = BOTTLE;
 }
 if (game.holding >= 7) {rspeak(92); return; }
 if (obj == BIRD && game.prop[BIRD] == 0) {
  if (toting(ROD)) {rspeak(26); return; }
  if (!toting(CAGE)) {rspeak(27); return; }
  game.prop[BIRD] = 1;
 }
 if ((obj == BIRD || obj == CAGE) && game.prop[BIRD] != 0)
  carry(BIRD+CAGE-obj, game.loc);
 carry(obj, game.loc);
 int k = liq();
 if (obj == BOTTLE && k != 0) game.place[k] = -1;
 rspeak(54);
}

void vopen(void) {
 int spk = actspk[verb];
 switch (obj) {
  case CLAM:
  case OYSTER: {
   int k = (obj == OYSTER);
   spk = 124 + k;
   if (toting(obj)) spk = 120 + k;
   if (!toting(TRIDENT)) spk = 122 + k;
   if (verb == LOCK) spk = 61;
   if (spk == 124) {
    destroy(CLAM);
    drop(OYSTER, game.loc);
    drop(PEARL, 105);
   }
   break;
  }
  case DOOR: spk = game.prop[DOOR] == 1 ? 54 : 111; break;
  case CAGE: spk = 32; break;
  case KEYS: spk = 55; break;
  case CHAIN:
   if (!here(KEYS)) spk = 31;
   else if (verb == LOCK) {
    spk = 172;
    if (game.prop[CHAIN] != 0) spk = 34;
    if (game.loc != 130) spk = 173;
    if (spk == 172) {
     game.prop[CHAIN] = 2;
     if (toting(CHAIN)) drop(CHAIN, game.loc);
     game.fixed[CHAIN] = -1;
    }
   } else {
    spk = 171;
    if (game.prop[BEAR] == 0) spk = 41;
    if (game.prop[CHAIN] == 0) spk = 37;
    if (spk == 171) {
     game.prop[CHAIN] = game.fixed[CHAIN] = 0;
     if (game.prop[BEAR] != 3) game.prop[BEAR] = 2;
     game.fixed[BEAR] = 2 - game.prop[BEAR];
    }
   }
   break;
  case GRATE:
   if (!here(KEYS)) spk = 31;
   else if (game.closing) {
    spk = 130;
    if (!game.panic) game.clock2 = 15;
    game.panic = true;
   } else {
    spk = 34 + game.prop[GRATE];
    game.prop[GRATE] = (verb != LOCK);
    spk += 2 * game.prop[GRATE];
   }
   break;
 }
 rspeak(spk);
}

void vread(void) {
 if (dark()) printf("\nI see no %s here.\n", in1);
 else {
  int spk = actspk[verb];
  if (obj == MAGZIN) spk = 190;
  if (obj == TABLET) spk = 196;
  if (obj == MESSAG) spk = 191;
  if (obj == OYSTER && game.hinted[2] && toting(OYSTER)) spk = 194;
  if (obj != OYSTER || game.hinted[2] || !toting(OYSTER) || !game.closed)
   rspeak(spk);
  else game.hinted[2] = yes(192, 193, 54);
 }
}

void vkill(void) {
 if (obj == 0) {
  if (game.dflag >= 2 && dwarfHere()) obj = DWARF;
  if (here(SNAKE)) obj = obj * 100 + SNAKE;
  if (at(DRAGON) && game.prop[DRAGON] == 0) obj = obj * 100 + DRAGON;
  if (at(TROLL)) obj = obj * 100 + TROLL;
  if (here(BEAR) && game.prop[BEAR] == 0) obj = obj * 100 + BEAR;
  if (obj > 100) {printf("\n%s what?\n", in1); obj = 0; togoto = 2600; return; }
  if (obj == 0) {
   if (here(BIRD) && verb != THROW) obj = BIRD;
   if (here(CLAM) || here(OYSTER)) obj = obj * 100 + CLAM;
   if (obj > 100) {
    printf("\n%s what?\n", in1);
    obj = 0;
    togoto = 2600;
    return;
   }
  }
 }
 switch (obj) {
  case BIRD:
   if (game.closed) rspeak(137);
   else {
    destroy(BIRD);
    game.prop[BIRD] = 0;
    if (game.place[SNAKE] == 19) game.tally2++;
    rspeak(45);
   }
   break;
  case 0: rspeak(44); break;
  case CLAM: case OYSTER: rspeak(150); break;
  case SNAKE: rspeak(46); break;
  case DWARF:
   if (game.closed) {rspeak(136); normend(); }
   else rspeak(49);
   break;
  case DRAGON:
   if (game.prop[DRAGON] != 0) rspeak(167);
   else {
    rspeak(49);
    verb = obj = 0;
    getin(word1, in1, word2, in2);
    if (strcmp(word1, "YES") && strcmp(word1, "Y")) {togoto = 2608; return; }
    pspeak(DRAGON, 1);
    game.prop[DRAGON] = 2;
    game.prop[RUG] = 0;
    move(DRAGON+100, -1);
    move(RUG+100, 0);
    move(DRAGON, 120);
    move(RUG, 120);
    for (int i=0; i<65; i++)
     if (game.place[i] == 119 || game.place[i] == 121) move(i, 120);
    game.loc = 120;
    domove(NULLMOVE);
   }
   break;
  case TROLL: rspeak(157); break;
  case BEAR: rspeak(165 + (game.prop[BEAR]+1) / 2); break;
  default: rspeak(actspk[verb]);
 }
}

void vpour(void) {
 if (obj == BOTTLE || obj == 0) obj = liq();
 if (obj == 0) {printf("\n%s what?\n", in1); obj = 0; togoto = 2600; }
 else if (!toting(obj)) rspeak(actspk[verb]);
 else if (obj != OIL && obj != WATER) rspeak(78);
 else {
  game.prop[BOTTLE] = 1;
  game.place[obj] = 0;
  if (at(DOOR)) {
   game.prop[DOOR] = (obj == OIL);
   rspeak(113 + game.prop[DOOR]);
  } else if (at(PLANT)) {
   if (obj != WATER) rspeak(112);
   else {
    pspeak(PLANT, game.prop[PLANT] + 1);
    game.prop[PLANT] = (game.prop[PLANT] + 2) % 6;
    game.prop[PLANT2] = game.prop[PLANT] / 2;
    domove(NULLMOVE);
   }
  } else rspeak(77);
 }
}

void vdrink(void) {
 if (obj == 0 && liqloc(game.loc) != WATER && (liq() != WATER || !here(BOTTLE)))
 {
  printf("\n%s what?\n", in1);
  obj = 0;
  togoto = 2600;
 } else if (obj == 0 || obj == WATER) {
  if (liq() == WATER && here(BOTTLE)) {
   game.prop[BOTTLE] = 1;
   game.place[WATER] = 0;
   rspeak(74);
  } else rspeak(actspk[verb]);
 } else rspeak(110);
}

void vfill(void) {
 if (obj == VASE) {
  if (liqloc(game.loc) == 0) rspeak(144);
  else if (!toting(VASE)) rspeak(29);
  else {
   rspeak(145);
   game.prop[VASE] = 2;
   game.fixed[VASE] = -1;
/* In the original Fortran, when the vase is filled with water or oil, its
 * property is set so that it breaks into pieces, *but* the code then branches
 * to label 9024 to actually drop the vase.  Once you cut out the unreachable
 * states, it turns out that the vase remains intact if the pillow is present,
 * but even if it survives it is still marked as a fixed object and can't be
 * picked up again.  This is probably a bug in the original code, but who am I
 * to fix it? */
   if (at(PILLOW)) game.prop[VASE] = 0;
   pspeak(VASE, game.prop[VASE] + 1);
   drop(obj, game.loc);
  }
 } else {
  if (obj != 0 && obj != BOTTLE) rspeak(actspk[verb]);
  else if (obj == 0 && !here(BOTTLE)) {
   printf("\n%s what?\n", in1);
   obj = 0;
   togoto = 2600;
  } else if (liq() != 0) rspeak(105);
  else if (liqloc(game.loc) == 0) rspeak(106);
  else {
   game.prop[BOTTLE] = cond[game.loc] & 2;
   if (toting(BOTTLE)) game.place[liq()] = -1;
   rspeak(liq() == OIL ? 108 : 107);
  }
 }
}

void vblast(void) {
 if (game.prop[ROD2] < 0 || !game.closed) rspeak(actspk[verb]);
 else {
  bonus = 133;
  if (game.loc == 115) bonus = 134;
  if (here(ROD2)) bonus = 135;
  rspeak(bonus);
  normend();
  /* Fin */
 }
}

void von(void) {
 if (!here(LAMP)) rspeak(actspk[verb]);
 else if (game.limit < 0) rspeak(184);
 else {
  game.prop[LAMP] = 1;
  rspeak(39);
  if (game.wzdark) togoto = 2000;
 }
}

void voff(void) {
 if (!here(LAMP)) rspeak(actspk[verb]);
 else {
  game.prop[LAMP] = 0;
  rspeak(40);
  if (dark()) rspeak(16);
 }
}

void vdrop(void) {
 if (toting(ROD2) && obj == ROD && !toting(ROD)) obj = ROD2;
 if (!toting(obj)) {rspeak(actspk[verb]); return; }
 if (obj == BIRD && here(SNAKE)) {
  rspeak(30);
  if (game.closed) {rspeak(136); normend(); }
  destroy(SNAKE);
  game.prop[SNAKE] = 1;
 } else if (obj == COINS && here(VEND)) {
  destroy(COINS);
  drop(BATTER, game.loc);
  pspeak(BATTER, 0);
  return;
 } else if (obj == BIRD && at(DRAGON) && game.prop[DRAGON] == 0) {
  rspeak(154);
  destroy(BIRD);
  game.prop[BIRD] = 0;
  if (game.place[SNAKE] == 19) game.tally2++;
  return;
 } else if (obj == BEAR && at(TROLL)) {
  rspeak(163);
  move(TROLL, 0);
  move(TROLL+100, 0);
  move(TROLL2, 117);
  move(TROLL2+100, 122);
  juggle(CHASM);
  game.prop[TROLL] = 2;
 } else if (obj == VASE && game.loc != 96) {
  game.prop[VASE] = at(PILLOW) ? 0 : 2;
  pspeak(VASE, game.prop[VASE] + 1);
  if (game.prop[VASE] != 0) game.fixed[VASE] = -1;
 } else rspeak(54);
 int k = liq();
 if (k == obj) obj = BOTTLE;
 if (obj == BOTTLE && k != 0) game.place[k] = 0;
 if (obj == CAGE && game.prop[BIRD] != 0) drop(BIRD, game.loc);
 if (obj == BIRD) game.prop[BIRD] = 0;
 drop(obj, game.loc);
}

void vfeed(void) {
 switch (obj) {
  case BIRD: rspeak(100); break;
  case SNAKE:
   if (!game.closed && here(BIRD)) {
    destroy(BIRD);
    game.prop[BIRD] = 0;
    game.tally2++;
    rspeak(101);
   } else rspeak(102);
   break;
  case TROLL: rspeak(182); break;
  case DRAGON: rspeak(game.prop[DRAGON] != 0 ? 110 : 102); break;
  case DWARF:
   if (!here(FOOD)) rspeak(actspk[verb]);
   else {game.dflag++; rspeak(103); }
   break;
  case BEAR:
   if (!here(FOOD)) rspeak(game.prop[BEAR] == 0 ? 102 : game.prop[BEAR] == 3
    ? 110 : actspk[verb]);
   else {
    destroy(FOOD);
    game.prop[BEAR] = 1;
    game.fixed[AXE] = 0;
    game.prop[AXE] = 0;
    rspeak(168);
   }
   break;
  default: rspeak(14);
 }
}

void vsay(void) {
 char* tk = *in2 ? in2 : in1;
 if (*word2) strncpy(word1, word2, 5);
 int i = vocab(word1, -1);
 if (i == 62 || i == 65 || i == 71 || i == 2025) {
  *word2 = 0;
  obj = 0;
  togoto = 2630;
 } else printf("\nOkay, \"%s\".\n", tk);
}

void vsuspend(char* file) {
#ifdef ADVMAGIC
 if (demo) {rspeak(201); return; }
 datime(&game.saved, &game.savet);
 printf("\nI can suspend your adventure for you so that you can resume later,"
  " but\nyou will have to wait at least %d minutes before continuing.\n",
  mage.latency);
#else
 puts("\nI can suspend your adventure for you so that you can resume later.");
#endif
 if (!yes(200, 54, 54)) return;
 printf("\nSaving to %s ...\n", file);
 FILE* adv = fopen(file, "wb");
 if (adv == NULL) {
  fprintf(stderr, "\nError: could not write to %s: ", file);
  perror(NULL);
  return;
 }
 if (fwrite(&game, sizeof game, 1, adv) != 1) {
  fprintf(stderr, "\nError writing to %s: ", file);
  perror(NULL);
  return;
 }
 fclose(adv);
#ifdef ADVMAGIC
 ciao();
#else
 exit(0);
#endif
}

bool vresume(char* file) {
#ifdef ADVMAGIC
 if (demo) {mspeak(9); return false; }
#endif
 if (game.turns > 1) {
  puts("\nTo resume an earlier Adventure, you must abandon the current one.");
/* This message is taken from the 430 pt. version of Adventure (version 2.5). */
  if (!yes(200, 54, 54)) return false;
 }
 printf("\nRestoring from %s ...\n", file);
 FILE* adv = fopen(file, "rb");
 if (adv == NULL) {
  fprintf(stderr, "\nError: could not read %s: ", file);
  perror(NULL);
  return false;
 }
 struct advgame backup = game;
 if (fread(&game, sizeof game, 1, adv) != 1) {
  fprintf(stderr, "\nError reading %s: ", file);
  perror(NULL);
  fclose(adv);
  game = backup;
  if (game.turns > 1) printf("\nSticking with current game\n");
  return false;
 }
 fclose(adv);
#ifdef ADVMAGIC
 start();
#endif
 domove(NULLMOVE);
 return true;
}

char* defaultSaveFile(void) {
 freeable = false;
 char* home = getenv("HOME");
 if (home != NULL) {
  size_t homeLen = strlen(home);
  size_t baseLen = strlen(DEFAULT_SAVE_NAME);
  char* filename = malloc(homeLen + baseLen + 2);
  if (filename == NULL) return DEFAULT_SAVE_NAME;
  strncpy(filename, home, homeLen);
  filename[homeLen] = '/';
  strncpy(filename + homeLen + 1, DEFAULT_SAVE_NAME, baseLen);
  filename[homeLen+baseLen+1] = 0;
  freeable = true;
  return filename;
 } else return DEFAULT_SAVE_NAME;
}
