#ifndef ADVDECL_H
#define ADVDECL_H

#include <stdbool.h>
#include <stdint.h>
#include "advconfig.h"
#include "advconst.h"

extern int togoto;
extern bool blklin, gaveup;
#ifdef ADVMAGIC
extern bool demo;
#endif
extern int bonus;
extern int verb, obj;
extern char in1[MAX_INPUT_LENGTH+1], in2[MAX_INPUT_LENGTH+1];
extern char word1[6], word2[6];

#ifdef ADVMAGIC
/* Magic data: */
struct advmagic {
 int32_t wkday, wkend, holid;
 /* These bitfields hold the times when adventurers are allowed into Colossal
  * Cave: wkday is for weekdays, wkend for weekends, and holid for holidays
  * (days with special hours).  If bit N of one of the above is on, then the
  * hour N:00 through N:59 is considered "prime time," i.e., the cave is closed
  * then. */
 int hbegin, hend;	/* start & end of next holiday */
 int shortGame;		/* turns allowed in a short/demo game */
 int magnm;		/* magic number */
 int latency;		/* minutes required to wait after saving */
 char magic[6];		/* magic word */
 char hname[21]		/* name of next holiday */;
 char msg[500];		/* MOTD */
};

extern struct advmagic mage;
#endif

/* User's game data: */
struct advgame {
 int loc, newloc, oldloc, oldloc2, limit;
 int turns, iwest, knifeloc, detail;
 int numdie, holding, foobar;
 int tally, tally2, abbnum, clock1, clock2;
 bool wzdark : 1, closing : 1, lmwarn : 1, panic : 1, closed : 1;
 signed char prop[65];
 int abb[141];
 int hintlc[10];
 bool hinted[10];
 unsigned char dloc[6];
 unsigned char odloc[6];
 bool dseen[6];
 int dflag, dkill;
 short place[65];
 short fixed[65];
 unsigned char atloc[141];
 unsigned char link[165];
 int saved, savet;
 /* Although `saved' and `savet' are only used when ADVMAGIC is defined, they
  * are declared in both forms of the game in order to make the save files
  * compatible. */
};

extern struct advgame game;

/* Built-in game data: */
extern const char* longdesc[141];
extern const char* shortdesc[141];
extern const char* itemDesc[65][7];
extern const char* rmsg[202];
#ifdef ADVMAGIC
extern const char* magicMsg[33];
#endif
extern const int travel[141][MAXROUTE][MAXTRAV];
extern const struct {char word[6]; int val1, val2; } vocabulary[WORDQTY];
extern const int actspk[32];
extern const int cond[141];
extern const struct {int score; char* rank; } classes[10];
extern const int hints[10][4];

/* Functions defined in util.c: */
bool toting(int item);
bool here(int item);
bool at(int item);
int liq2(int p);
int liq(void);
int liqloc(int loc);
bool bitset(int loc, int n);
bool forced(int loc);
bool dark(void);
bool pct(int x);
void speak(const char* s);
void pspeak(int item, int state);
void rspeak(int msg);
bool yes(int x, int y, int z);
void destroy(int obj);
void juggle(int obj);
void move(int obj, int where);
int put(int obj, int where, int pval);
void carry(int obj, int where);
void drop(int obj, int where);
void bug(int num);
int vocab(const char* word, int type);
void getin(char* w1, char* r1, char* w2, char* r2);
void ftoeol(void);
void readError(void);
void init_ran(int* seed);
int ran(int max);
#ifdef ADVMAGIC
void ciao(void);
void mspeak(int msg);
bool yesm(int x, int y, int z);
bool start(void);
void maint(void);
void poof(void);
bool wizard(void);
void hours(void);
void hoursx(int32_t hours, const char* day);
void newhrs(void);
int32_t newhrx(const char* day);
void motd(bool alter);
#endif
#if defined(ADVMAGIC) || defined(ORIG_RNG)
void datime(int* d, int* t);
#endif

/* Functions defined in motion.c: */
void domove(int motion);
void dotrav(int motion);
void death(void);
int score(bool scoring);
void normend(void);
void doaction(void);
bool dwarfHere(void);

/* Functions defined in verbs.c: */
void intransitive(void);
void transitive(void);
void vtake(void);
void vopen(void);
void vread(void);
void vkill(void);
void vpour(void);
void vdrink(void);
void vfill(void);
void vblast(void);
void von(void);
void voff(void);
void vdrop(void);
void vfeed(void);
void vsay(void);
void vsuspend(char* file);
bool vresume(char* file);
char* defaultSaveFile(void);

#endif
