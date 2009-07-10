#ifndef ADVCONFIG_H
#define ADVCONFIG_H

#define ADVMAGIC

#ifdef ADVMAGIC
/*#define MAGICFILE  "/usr/games/lib/advmagic"*/
#define MAGICFILE  "magic.dat"
/* file in which the current magic values are stored */
#endif

/* #define ORIG_RNG */
/* Use the same random number generator algorithm as the one used in the
 * original PDP-10 Fortran implementation of the Colossal Cave Adventure */

#define RANDOM_RNG
/* Use the non-standard (but usually higher-quality) function random() in place
 * of rand(); if both are defined, ORIG_RNG takes precedence over RANDOM_RNG */

#define DEFAULT_SAVE_NAME  ".adventure"
/* name of the default save file */

#define MAX_INPUT_LENGTH  80
/* maximum length of an input line and input word */

#endif
