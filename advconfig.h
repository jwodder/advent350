#ifndef ADVCONFIG_H
#define ADVCONFIG_H

/* #define ADVMAGIC */

#ifdef ADVMAGIC
#define MAGICFILE "/usr/games/lib/advmagic"
/* file in which the current magic values are stored */
#endif

/* #define ORIG_RAND */

#define DEFAULT_SAVE_NAME  ".adventure"
/* name of the default save file */

#define MAX_INPUT_LENGTH  80
/* maximum length of an input line and input word */

#endif
