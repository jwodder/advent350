#ifndef ADVCONFIG_H
#define ADVCONFIG_H

/* #define ADVMAGIC */
/* If ADVMAGIC is defined, the resulting binary will contain the "magic"
 * features of Adventure that are usually omitted from other ports: the game
 * will only be playable during certain hours, a delay will be required before
 * resuming a saved game, and wizards will be able to bypass and reconfigure
 * these restrictions through maintenance/magic mode. */

#ifdef ADVMAGIC
#define MAGICFILE  "/usr/games/lib/advmagic"
/* MAGICFILE is the file in which the current magic settings are stored.  You
 * should make it readable by everyone but writeable only by people that you
 * want making changes to it.
 */
#endif

#define DEFAULT_SAVE_NAME  ".adventure"
/* basename of the default save file in the user's home directory */

#define MAX_INPUT_LENGTH  80
/* maximum length of input lines (counting the newline); longer lines will have
 * their trailing characters discarded */

#define ORIG_RNG
/* If ORIG_RNG is defined, the random number generation algorithm implemented
 * in the original PDP-10 Fortran code will be used instead of C's rand() or
 * random(). */

/* #define RANDOM_RNG */
/* If RANDOM_RNG is defined, the non-standard (but usually higher-quality)
 * function random() will be used instead of rand().  If both RANDOM_RNG and
 * ORIG_RNG are defined, ORIG_RNG takes precedence. */

/* #define SEEDABLE_RNG */
/* If SEEDABLE_RNG is defined, the resulting program will take an optional `-S
 * seed` parameter on the command line for seeding the RNG with a specified
 * value.  This is intended for use in debugging. */

#endif
