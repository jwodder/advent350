/* Verb functions */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "advconfig.h"
#include "advconst.h"
#include "advdecl.h"

static bool freeable;

void intransitive(void) {
/* Label 4080 (intransitive verb handling): */
/* As this function is only called at a single point in the code, it doesn't
 * actually need to be a separate routine, but I think we can all agree that
 * turn() is complicated enough as it is without stuffing yet another 120-line
 * "switch" block inside of it. */
 togoto = 2012;
 switch (verb) {
  case NOTHING: rspeak(54); break;
  case WALK: rspeak(actspk[verb]); break;

  case DROP: case SAY: case WAVE: case CALM: case RUB:
  case THROW: case FIND: case FEED: case BREAK: case WAKE:
   printf("%s what?\n", in1);
   obj = 0;
   togoto = 2600;
   break;

  case TAKE:
   if (!(atloc[loc] && link[atloc[loc]] == 0) || dflag >= 2 && dwarfHere()) {
    printf("%s what?\n", in1);
    obj = 0;
    togoto = 2600;
   } else {
    obj = atloc[loc];
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
    printf("%s what?\n", in1);
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
    printf("%s what?\n", in1);
    obj = 0;
    togoto = 2600;
   }
   break;

  case QUIT:
   /* Yes, this is supposed to be an assignment: */
   if (gaveup = yes(22, 54, 54)) normend();
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
   printf("If you were to quit now, you would score %d out of a possible"
    " 350.\n", scr);
   if (gaveup = yes(143, 54, 54)) normend();
   break;
  }

  case FOO: {
   int k = vocab(word1, 3);
   if (foobar == 1-k) {
    foobar = k;
    if (k != 4) {rspeak(54); return; }
    foobar = 0;
    if (place[EGGS] == 92 || toting(EGGS) && loc == 92) rspeak(42);
    else {
     if (place[EGGS] == 0 && place[TROLL] == 0 && prop[TROLL] == 0)
      prop[TROLL] = 1;
     k = (loc == 92 ? 0 : here(EGGS) ? 1 : 2);
     move(EGGS, 92);
     pspeak(EGGS, k);
    }
   } else rspeak(foobar ? 151 : 42);
   break;
  }

  case BRIEF:
   abbnum = 10000;
   detail = 3;
   rspeak(156);
   break;

  case READ:
   if (here(MAGZIN)) obj = MAGZIN;
   if (here(TABLET)) obj = obj * 100 + TABLET;
   if (here(MESSAG)) obj = obj * 100 + MESSAG;
   if (closed && toting(OYSTER)) obj = OYSTER;
   if (obj > 100 || obj == 0 || dark()) {
    printf("%s what?\n", in1);
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
   puts("Colossal Cave is open all day, every day.");
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
   else if (obj != ROD || !at(FISSUR) || !toting(obj) || closing)
    rspeak(actspk[verb]);
   else {
    prop[FISSUR] = 1 - prop[FISSUR];
    pspeak(FISSUR, 2 - prop[FISSUR]);
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
    for (int j=0; j<5; j++) if (dloc[j] == loc) {i = j; break; }
    if (i != -1) {
     if (ran(3) == 0) rspeak(48);
     else {
      dseen[i] = false;
      dloc[i] = 0;
      rspeak(++dkill == 1 ? 149 : 47);
     }
     drop(AXE, loc);
     domove(NULLMOVE);
    } else if (at(DRAGON) && prop[DRAGON] == 0) {
     rspeak(152);
     drop(AXE, loc);
     domove(NULLMOVE);
    } else if (at(TROLL)) {
     rspeak(158);
     drop(AXE, loc);
     domove(NULLMOVE);
    } else if (here(BEAR) && prop[BEAR] == 0) {
     drop(AXE, loc);
     fixed[AXE] = -1;
     prop[AXE] = 1;
     juggle(BEAR);  /* Don't try this at home, kids. */
     rspeak(164);
    } else {obj = 0; vkill(); }
   } else vdrop();
   break;

  case FIND:
  case INVENT:
   if (toting(obj)) rspeak(24);
   else if (closed) rspeak(138);
   else if (obj == DWARF && dflag >= 2 && dwarfHere()) rspeak(94);
   else if (at(obj) || (liq() == obj && at(BOTTLE)) || obj == liqloc(loc))
    rspeak(94);
   else rspeak(actspk[verb]);
   break;

  case FEED: vfeed(); break;
  case FILL: vfill(); break;
  case BLAST: vblast(); break;
  case READ: vread(); break;

  case BREAK:
   if (obj == VASE && prop[VASE] == 0) {
    if (toting(VASE)) drop(VASE, loc);
    prop[VASE] = 2;
    fixed[VASE] = -1;
    rspeak(198);
   } else if (obj != MIRROR) rspeak(actspk[verb]);
   else if (!closed) rspeak(148);
   else {rspeak(197); rspeak(136); normend(); }
   break;

  case WAKE:
   if (obj == DWARF && closed) {rspeak(199); rspeak(136); normend(); }
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
 if (obj == PLANT && prop[PLANT] <= 0) spk = 115;
 if (obj == BEAR && prop[BEAR] == 1) spk = 169;
 if (obj == CHAIN && prop[BEAR] != 0) spk = 170;
 if (fixed[obj]) {rspeak(spk); return; }
 if (obj == WATER || obj == OIL) {
  if (!here(BOTTLE) || liq() != obj) {
   obj = BOTTLE;
   if (toting(BOTTLE) && prop[BOTTLE] == 1) vfill();
   else {
    if (prop[BOTTLE] != 1) spk = 105;
    if (!toting(BOTTLE)) spk = 104;
    rspeak(spk);
   }
   return;
  }
  obj = BOTTLE;
 }
 if (holding >= 7) {rspeak(92); return; }
 if (obj == BIRD && prop[BIRD] == 0) {
  if (toting(ROD)) {rspeak(26); return; }
  if (!toting(CAGE)) {rspeak(27); return; }
  prop[BIRD] = 1;
 }
 if ((obj == BIRD || obj == CAGE) && prop[BIRD] != 0) carry(BIRD+CAGE-obj, loc);
 carry(obj, loc);
 int k = liq();
 if (obj == BOTTLE && k != 0) place[k] = -1;
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
    drop(OYSTER, loc);
    drop(PEARL, 105);
   }
   break;
  }

  case DOOR: spk = prop[DOOR] == 1 ? 54 : 111; break;
  case CAGE: spk = 32; break;
  case KEYS: spk = 55; break;

  case CHAIN:
   if (!here(KEYS)) spk = 31;
   else if (verb == LOCK) {
    spk = 172;
    if (prop[CHAIN] != 0) spk = 34;
    if (loc != 130) spk = 173;
    if (spk == 172) {
     prop[CHAIN] = 2;
     if (toting(CHAIN)) drop(CHAIN, loc);
     fixed[CHAIN] = -1;
    }
   } else {
    spk = 171;
    if (prop[BEAR] == 0) spk = 41;
    if (prop[CHAIN] == 0) spk = 37;
    if (spk == 171) {
     prop[CHAIN] = fixed[CHAIN] = 0;
     if (prop[BEAR] != 3) prop[BEAR] = 2;
     fixed[BEAR] = 2 - prop[BEAR];
    }
   }
   break;

  case GRATE:
   if (here(KEYS)) spk = 31;
   else if (closing) {
    spk = 130;
    if (!panic) clock2 = 15;
    panic = true;
   } else {
    spk = 34 + prop[GRATE];
    prop[GRATE] = (verb != LOCK);
    spk += 2 * prop[GRATE];
   }
   break;
 }
 rspeak(spk);
}

void vread(void) {
 if (dark()) printf("I see no %s here.\n", in1);
 else {
  int spk = actspk[verb];
  if (obj == MAGZIN) spk = 190;
  if (obj == TABLET) spk = 196;
  if (obj == MESSAG) spk = 191;
  if (obj == OYSTER && hinted[2] && toting(OYSTER)) spk = 194;
  if (obj != OYSTER || hinted[2] || !toting(OYSTER) || !closed) rspeak(spk);
  else hinted[2] = yes(192, 193, 54);
 }
}

void vkill(void) {
 if (obj == 0) {
  if (dflag >= 2 && dwarfHere()) obj = DWARF;
  if (here(SNAKE)) obj = obj * 100 + SNAKE;
  if (at(DRAGON) && prop[DRAGON] == 0) obj = obj * 100 + DRAGON;
  if (at(TROLL)) obj = obj * 100 + TROLL;
  if (here(BEAR) && prop[BEAR] == 0) obj = obj * 100 + BEAR;
  if (obj > 100) {printf("%s what?\n", in1); obj = 0; togoto = 2600; return; }
  if (obj == 0) {
   if (here(BIRD) && verb != THROW) obj = BIRD;
   if (here(CLAM) || here(OYSTER)) obj = obj * 100 + CLAM;
   if (obj > 100) {printf("%s what?\n", in1); obj = 0; togoto = 2600; return; }
  }
 }
 switch (obj) {
  case BIRD:
   if (closed) rspeak(137);
   else {
    destroy(BIRD);
    prop[BIRD] = 0;
    if (place[SNAKE] == 19) tally2++;
    rspeak(45);
   }
   break;

  case 0: rspeak(44); break;
  case CLAM: case OYSTER: rspeak(150); break;
  case SNAKE: rspeak(46); break;

  case DWARF:
   if (closed) {rspeak(136); normend(); }
   else rspeak(49);
   break;

  case DRAGON:
   if (prop[DRAGON] != 0) rspeak(167);
   else {
    rspeak(49);
    verb = obj = 0;
    getin(word1, in1, word2, in2);
    if (strcmp(word1, "YES") && strcmp(word1, "Y")) {togoto = 2608; return; }
    pspeak(DRAGON, 1);
    prop[DRAGON] = 2;
    prop[RUG] = 0;
    move(DRAGON+100, -1);
    move(RUG+100, 0);
    move(DRAGON, 120);
    move(RUG, 120);
    for (int i=0; i<65; i++)
     if (place[i] == 119 || place[i] == 121) move(i, 120);
    loc = 120;
    domove(NULLMOVE);
   }
   break;

  case TROLL: rspeak(157); break;
  case BEAR: rspeak(165 + (prop[BEAR]+1) / 2); break;
  default: rspeak(actspk[verb]);
 }
}

void vpour(void) {
 if (obj == BOTTLE || obj == 0) obj = liq();
 if (obj == 0) {printf("%s what?\n", in1); obj = 0; togoto = 2600; }
 else if (!toting(obj)) rspeak(actspk[verb]);
 else if (obj != OIL && obj != WATER) rspeak(78);
 else {
  prop[BOTTLE] = 1;
  place[obj] = 0;
  if (at(DOOR)) {
   prop[DOOR] = (obj == OIL);
   rspeak(113 + prop[DOOR]);
  } else if (at(PLANT)) {
   if (obj != WATER) rspeak(112);
   else {
    pspeak(PLANT, prop[PLANT] + 1);
    prop[PLANT] = (prop[PLANT] + 2) % 6;
    prop[PLANT2] = prop[PLANT] / 2;
    domove(NULLMOVE);
   }
  } else rspeak(77);
 }
}

void vdrink(void) {
 if (obj == 0 && liqloc(loc) != WATER && (liq() != WATER || !here(BOTTLE))) {
  printf("%s what?\n", in1);
  obj = 0;
  togoto = 2600;
 } else if (obj == 0 || obj == WATER) {
  if (liq() == WATER && here(BOTTLE)) {
   prop[BOTTLE] = 1;
   place[WATER] = 0;
   rspeak(74);
  } else rspeak(actspk[verb]);
 } else rspeak(110);
}

void vfill(void) {
 if (obj == VASE) {
  if (liqloc(loc) == 0) rspeak(144);
  else if (!toting(VASE)) rspeak(29);
  else {
   rspeak(145);
   prop[VASE] = 2;
   fixed[VASE] = -1;
/* In the original Fortran, when the vase is filled with water or oil, its
 * property is set so that it breaks into pieces, *but* the code then branches
 * to label 9024 to actually drop the vase.  Once you cut out the unreachable
 * states, it turns out that the vase remains intact if the pillow is present,
 * but even if it survives it is still marked as a fixed object and can't be
 * picked up again.  This is probably a bug in the original code, but who am I
 * to fix it? */
   if (at(PILLOW)) prop[VASE] = 0;
   pspeak(VASE, prop[VASE] + 1);
   drop(obj, loc);
  }
 } else {
  if (obj != 0 && obj != BOTTLE) rspeak(actspk[verb]);
  else if (obj == 0 && !here(BOTTLE)) {
   printf("%s what?\n", in1);
   obj = 0;
   togoto = 2600;
  } else if (liq() != 0) rspeak(105);
  else if (liqloc(loc) == 0) rspeak(106);
  else {
   prop[BOTTLE] = cond[loc] & 2;
   if (toting(BOTTLE)) place[liq()] = -1;
   rspeak(liq() == OIL ? 108 : 107);
  }
 }
}

void vblast(void) {
 if (prop[ROD2] < 0 || !closed) rspeak(actspk[verb]);
 else {
  bonus = 133;
  if (loc == 115) bonus = 134;
  if (here(ROD2)) bonus = 135;
  rspeak(bonus);
  normend();
  /* Fin */
 }
}

void von(void) {
 if (!here(LAMP)) rspeak(actspk[verb]);
 else if (limit < 0) rspeak(184);
 else {
  prop[LAMP] = 1;
  rspeak(39);
  if (wzdark) togoto = 2000;
 }
}

void voff(void) {
 if (here(LAMP)) rspeak(actspk[verb]);
 else {
  prop[LAMP] = 0;
  rspeak(40);
  if (dark()) rspeak(16);
 }
}

void vdrop(void) {
 if (toting(ROD2) && obj == ROD && !toting(ROD)) obj = ROD2;
 if (!toting(obj)) {rspeak(actspk[verb]); return; }
 if (obj == BIRD && here(SNAKE)) {
  rspeak(30);
  if (closed) {rspeak(136); normend(); }
  destroy(SNAKE);
  prop[SNAKE] = 1;
 } else if (obj == COINS && here(VEND)) {
  destroy(COINS);
  drop(BATTER, loc);
  pspeak(BATTER, 0);
  return;
 } else if (obj == BIRD && at(DRAGON) && prop[DRAGON] == 0) {
  rspeak(154);
  destroy(BIRD);
  prop[BIRD] = 0;
  if (place[SNAKE] == 19) tally2++;
  return;
 } else if (obj == BEAR && at(TROLL)) {
  rspeak(163);
  move(TROLL, 0);
  move(TROLL+100, 0);
  move(TROLL2, 117);
  move(TROLL2+100, 122);
  juggle(CHASM);
  prop[TROLL] = 2;
 } else if (obj == VASE && loc != 96) {
  prop[VASE] = at(PILLOW) ? 0 : 2;
  pspeak(VASE, prop[VASE] + 1);
  if (prop[VASE] != 0) fixed[VASE] = -1;
 } else rspeak(54);
 int k = liq();
 if (k == obj) obj = BOTTLE;
 if (obj == BOTTLE && k != 0) place[k] = 0;
 if (obj == CAGE && prop[BIRD] != 0) drop(BIRD, loc);
 if (obj == BIRD) prop[BIRD] = 0;
 drop(obj, loc);
}

void vfeed(void) {
 switch (obj) {
  case BIRD: rspeak(100); break;
  case SNAKE:
   if (!closed && here(BIRD)) {
    destroy(BIRD);
    prop[BIRD] = 0;
    tally2++;
    rspeak(101);
   } else rspeak(102);
   break;
  case TROLL: rspeak(182); break;
  case DRAGON: rspeak(prop[DRAGON] != 0 ? 110 : 102); break;
  case DWARF:
   if (!here(FOOD)) rspeak(actspk[verb]);
   else {dflag++; rspeak(103); }
   break;
  case BEAR:
   if (!here(FOOD)) rspeak(prop[BEAR] == 0 ? 102 : prop[BEAR] == 3 ? 110
    : actspk[verb]);
   else {
    destroy(FOOD);
    prop[BEAR] = 1;
    fixed[AXE] = 0;
    prop[AXE] = 0;
    rspeak(168);
   }
   break;
  default: rspeak(14);
 }
}

void vsay(void) {
 char* tk = *in2 ? in2 : in1;
 if (*word2) strcpy(word1, word2);
 int i = vocab(word1, -1);
 if (i == 62 || i == 65 || i == 71 || i == 2025) {
  *word2 = 0;
  obj = 0;
  togoto = 2630;
 } else printf("Okay, \"%s\".\n", tk);
}

void vsuspend(char* file) {
#ifdef ADVMAGIC
 if (demo) {rspeak(201); return; }
 datime(&saved, &savet);
 printf("I can suspend your adventure for you so that you can resume later,"
  " but\nyou will have to wait at least %d minutes before continuing.\n",
  latency);
#else
 puts("I can suspend your adventure for you so that you can resume later.");
#endif
 if (!yes(200, 54, 54)) return;
 printf("\nSaving to %s ...\n", file);

 FILE* adv = fopen(file, "wb");
 if (adv == NULL) {
  printf("Error: could not write to %s: ", file);
  perror(NULL);
  return;
 }

 /* Check the return values of all of these calls for failure! */
 fwrite(&loc, sizeof loc, 1, adv);
 fwrite(&newloc, sizeof newloc, 1, adv);
 fwrite(&oldloc, sizeof oldloc, 1, adv);
 fwrite(&oldloc2, sizeof oldloc2, 1, adv);
 fwrite(&limit, sizeof limit, 1, adv);
 fwrite(&turns, sizeof turns, 1, adv);
 fwrite(&iwest, sizeof iwest, 1, adv);
 fwrite(&knifeloc, sizeof knifeloc, 1, adv);
 fwrite(&detail, sizeof detail, 1, adv);
 fwrite(&numdie, sizeof numdie, 1, adv);
 fwrite(&holding, sizeof holding, 1, adv);
 fwrite(&foobar, sizeof foobar, 1, adv);
 fwrite(&tally, sizeof tally, 1, adv);
 fwrite(&tally2, sizeof tally2, 1, adv);
 fwrite(&abbnum, sizeof abbnum, 1, adv);
 fwrite(&clock1, sizeof clock1, 1, adv);
 fwrite(&clock2, sizeof clock2, 1, adv);
 fwrite(&wzdark, sizeof wzdark, 1, adv);
 fwrite(&closing, sizeof closing, 1, adv);
 fwrite(&lmwarn, sizeof lmwarn, 1, adv);
 fwrite(&panic, sizeof panic, 1, adv);
 fwrite(&closed, sizeof closed, 1, adv);
 fwrite(prop, sizeof(prop[0]), sizeof(prop)/sizeof(prop[0]), adv);
 fwrite(abb, sizeof(abb[0]), sizeof(abb)/sizeof(abb[0]), adv);
 fwrite(hintlc, sizeof(hintlc[0]), sizeof(hintlc)/sizeof(hintlc[0]), adv);
 fwrite(hinted, sizeof(hinted[0]), sizeof(hinted)/sizeof(hinted[0]), adv);
 fwrite(dloc, sizeof(dloc[0]), sizeof(dloc)/sizeof(dloc[0]), adv);
 fwrite(odloc, sizeof(odloc[0]), sizeof(odloc)/sizeof(odloc[0]), adv);
 fwrite(dseen, sizeof(dseen[0]), sizeof(dseen)/sizeof(dseen[0]), adv);
 fwrite(&dflag, sizeof dflag, 1, adv);
 fwrite(&dkill, sizeof dkill, 1, adv);
 fwrite(place, sizeof(place[0]), sizeof(place)/sizeof(place[0]), adv);
 fwrite(fixed, sizeof(fixed[0]), sizeof(fixed)/sizeof(fixed[0]), adv);
 fwrite(atloc, sizeof(atloc[0]), sizeof(atloc)/sizeof(atloc[0]), adv);
 fwrite(link, sizeof(link[0]), sizeof(link)/sizeof(link[0]), adv);
 fwrite(&saved, sizeof saved, 1, adv);
 fwrite(&savet, sizeof savet, 1, adv);
 
 fclose(adv);
#ifdef ADVMAGIC
 ciao();
#else
 exit(0);
#endif
}

void vresume(char* file) {
 if (turns != 0) {
  puts("To resume an earlier Adventure, you must abandon the current one.");
/* This message is taken from the 430 pt. version of Adventure (version 2.5). */
  if (!yes(200, 54, 54)) return;
 }
 printf("\nRestoring from %s ...\n", file);

 FILE* adv = fopen(file, "rb");
 if (adv == NULL) {
  printf("Error: could not read %s: ", file);
  perror(NULL);
  return;
 }

 /* Check the return values of all of these calls for failure! */
 fread(&loc, sizeof loc, 1, adv);
 fread(&newloc, sizeof newloc, 1, adv);
 fread(&oldloc, sizeof oldloc, 1, adv);
 fread(&oldloc2, sizeof oldloc2, 1, adv);
 fread(&limit, sizeof limit, 1, adv);
 fread(&turns, sizeof turns, 1, adv);
 fread(&iwest, sizeof iwest, 1, adv);
 fread(&knifeloc, sizeof knifeloc, 1, adv);
 fread(&detail, sizeof detail, 1, adv);
 fread(&numdie, sizeof numdie, 1, adv);
 fread(&holding, sizeof holding, 1, adv);
 fread(&foobar, sizeof foobar, 1, adv);
 fread(&tally, sizeof tally, 1, adv);
 fread(&tally2, sizeof tally2, 1, adv);
 fread(&abbnum, sizeof abbnum, 1, adv);
 fread(&clock1, sizeof clock1, 1, adv);
 fread(&clock2, sizeof clock2, 1, adv);
 fread(&wzdark, sizeof wzdark, 1, adv);
 fread(&closing, sizeof closing, 1, adv);
 fread(&lmwarn, sizeof lmwarn, 1, adv);
 fread(&panic, sizeof panic, 1, adv);
 fread(&closed, sizeof closed, 1, adv);
 fread(prop, sizeof(prop[0]), sizeof(prop)/sizeof(prop[0]), adv);
 fread(abb, sizeof(abb[0]), sizeof(abb)/sizeof(abb[0]), adv);
 fread(hintlc, sizeof(hintlc[0]), sizeof(hintlc)/sizeof(hintlc[0]), adv);
 fread(hinted, sizeof(hinted[0]), sizeof(hinted)/sizeof(hinted[0]), adv);
 fread(dloc, sizeof(dloc[0]), sizeof(dloc)/sizeof(dloc[0]), adv);
 fread(odloc, sizeof(odloc[0]), sizeof(odloc)/sizeof(odloc[0]), adv);
 fread(dseen, sizeof(dseen[0]), sizeof(dseen)/sizeof(dseen[0]), adv);
 fread(&dflag, sizeof dflag, 1, adv);
 fread(&dkill, sizeof dkill, 1, adv);
 fread(place, sizeof(place[0]), sizeof(place)/sizeof(place[0]), adv);
 fread(fixed, sizeof(fixed[0]), sizeof(fixed)/sizeof(fixed[0]), adv);
 fread(atloc, sizeof(atloc[0]), sizeof(atloc)/sizeof(atloc[0]), adv);
 fread(link, sizeof(link[0]), sizeof(link)/sizeof(link[0]), adv);
 fread(&saved, sizeof saved, 1, adv);
 fread(&savet, sizeof savet, 1, adv);
 
 fclose(adv);
#ifdef ADVMAGIC
 start();
#endif
 domove(NULLMOVE);
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
