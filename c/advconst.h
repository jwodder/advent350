#ifndef ADVCONST_H
#define ADVCONST_H

enum item {KEYS = 1, LAMP, GRATE, CAGE, ROD, ROD2, STEPS, BIRD, DOOR, PILLOW,
 SNAKE, FISSUR, TABLET, CLAM, OYSTER, MAGZIN, DWARF, KNIFE, FOOD, BOTTLE,
 WATER, OIL, MIRROR, PLANT, PLANT2, AXE = 28, DRAGON = 31, CHASM, TROLL,
 TROLL2, BEAR, MESSAG, VOLCANO, VEND, BATTER, NUGGET = 50, COINS = 54, CHEST,
 EGGS, TRIDENT, VASE, EMERALD, PYRAM, PEARL, RUG, SPICES, CHAIN};

enum movement {BACK = 8, NULLMOVE = 21, LOOK = 57, DEPRESSION = 63, ENTRANCE =
 64, CAVE = 67};

enum action {TAKE = 1, DROP, SAY, OPEN, NOTHING, LOCK, ON, OFF, WAVE, CALM,
 WALK, KILL, POUR, EAT, DRINK, RUB, THROW, QUIT, FIND, INVENT, FEED, FILL,
 BLAST, SCORE, FOO, BRIEF, READ, BREAK, WAKE, SUSPEND, HOURS, RESUME};

#define MAXDIE  3
#define CHLOC   114
#define CHLOC2  140

#define WORDQTY  289  /* number of words in the vocabulary table */
#define MAXROUTE 13   /* maximum number of travel rows for a location */
#define MAXTRAV  11   /* maximum number of elements in a travel row */

#endif
