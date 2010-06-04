#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "advconfig.h"
#include "advconst.h"
#include "advdecl.h"

bool toting(int item) {
 return game.place[item] == -1;
}

bool here(int item) {
 return game.place[item] == game.loc || toting(item);
}

bool at(int item) {
 return game.loc == game.place[item] || game.loc == game.fixed[item];
}

int liq2(int p) {
 return p == 0 ? WATER : p == 2 ? OIL : 0;
}

int liq(void) {
 return liq2(game.prop[BOTTLE] < 0 ? -1-game.prop[BOTTLE] : game.prop[BOTTLE]);
}

int liqloc(int loc) {
 return liq2(cond[loc] & 4 ? cond[loc] & 2 : 1);
}

bool bitset(int loc, int n) {
 return cond[loc] & 1 << n;
}

bool forced(int loc) {
 return travel[loc][0][1] == 1;
}

bool dark(void) {
 return !(cond[game.loc] & 1 || (game.prop[LAMP] && here(LAMP)));
}

bool pct(int x) {
 return ran(100) < x;
}

void speak(const char* s) {
 if (s == NULL) return;
 if (blklin) putchar('\n');
 fputs(s, stdout);
}

void pspeak(int item, int state) {
 speak(itemDesc[item][state+1]);
}

void rspeak(int msg) {
 if (msg != 0) speak(rmsg[msg]);
}

bool yes(int x, int y, int z) {
 for (;;) {
  rspeak(x);
  char reply[6];
  getin(reply, NULL, NULL, NULL); /* Ignore everything after the first word. */
  if (strcmp(reply, "YES") == 0 || strcmp(reply, "Y") == 0) {
   rspeak(y);
   return true;
  } else if (strcmp(reply, "NO") == 0 || strcmp(reply, "N") == 0) {
   rspeak(z);
   return false;
  } else printf("\nPlease answer the question.\n");
 }
}

void destroy(int obj) {move(obj, 0); }

void juggle(int obj) {
 move(obj, game.place[obj]);
 move(obj+100, game.fixed[obj]);
}

void move(int obj, int where) {
 int from = obj > 100 ? game.fixed[obj-100] : game.place[obj];
 if (0 < from && from <= 300) carry(obj, from);
 drop(obj, where);
}

int put(int obj, int where, int pval) {
 move(obj, where);
 return -1 - pval;
}

void carry(int obj, int where) {
 if (obj <= 100) {
  if (game.place[obj] == -1) return;
  game.place[obj] = -1;
  game.holding++;
 }
 if (game.atloc[where] == obj) game.atloc[where] = game.link[obj];
 else {
  int tmp;
  for (tmp = game.atloc[where]; game.link[tmp] != obj; tmp = game.link[tmp]);
  game.link[tmp] = game.link[obj];
 }
}

void drop(int obj, int where) {
 if (obj > 100) {
  game.fixed[obj-100] = where;
 } else {
  if (game.place[obj] == -1) game.holding--;
  game.place[obj] = where;
 }
 if (where > 0) {
  game.link[obj] = game.atloc[where];
  game.atloc[where] = obj;
 }
}

void bug(int num) {
 printf("Fatal error, see source code for interpretation.\n");
/* Given the above message, I suppose I should list the possible bug numbers in
 * the source somewhere, and right here is as good a place as any:
 * 5 - Required vocabulary word not found
 * 20 - Special travel number (500>L>300) is outside of defined range
 * 22 - Vocabulary type (N/1000) not between 0 and 3
 * 23 - Intransitive action verb not defined
 * 24 - Transitive action verb not defined
 * 26 - Location has no travel entries
 */
 printf("Probable cause: erroneous info in database.\n");
 printf("Error code = %d\n\n", num);
 exit(EXIT_FAILURE);
}

int vocab(const char* word, int type) {
 int low = 0, high = WORDQTY-1, i;
 while (low <= high) {
  i = (low+high)/2;
  int cmp = strcmp(word, vocabulary[i].word);
  if (cmp < 0) high = i-1;
  else if (cmp > 0) low = i+1;
  else break;
 }
 if (low > high) {
  if (type >= 0) bug(5);
  return -1;
 }
 return type >= 0 ? (vocabulary[i].val1 / 1000 == type ? vocabulary[i].val1
  : vocabulary[i].val2) % 1000 : vocabulary[i].val1;
}

/* getin(word1, full1, word2, full2) reads a line of text from standard input
 * (maximum length MAX_INPUT_LENGTH characters; if the "line" is nothing but
 * spaces without a terminating newline, another "line" will be read in) and
 * stores the first word (non-whitespace substring) in full1 and the second
 * word (or an empty string if there is none) in full2, both of which must be
 * MAX_INPUT_LENGTH+1 or more characters in size.  The first five letters of
 * each word, converted to uppercase, are stored in word1 and word2, which must
 * each be 6 or more characters in size.  getin() will not attempt to store
 * anything in a NULL argument.  If the user enters a blank line (or a line of
 * spaces) while `blklin' is true, getin() will continue prompting for input; a
 * blank line while `blklin' is false will cause all of getin's arguments to be
 * set to the empty string.  getin() will always read up through a terminating
 * newline, but anything beyond the MAX_INPUT_LENGTH mark will be discarded.
 */

void getin(char* w1, char* r1, char* w2, char* r2) {
 static char line[MAX_INPUT_LENGTH+1];
 char* start;
 if (blklin) putchar('\n');
 printf("> ");
 for (;;) {
  fgets(line, MAX_INPUT_LENGTH+1, stdin);
  start = line;
  while (isspace(*start) && *start != '\n') start++;
  if (*start == '\n') {
   if (blklin) printf("> ");  /* and then loop back around */
   else {
    *w1 = 0;
    if (r1 != NULL) *r1 = 0;
    if (w2 != NULL) *w2 = 0;
    if (r2 != NULL) *r2 = 0;
    return;
   }
  } else if (*start != 0) break;
 }
 char* end = start;
 while (!isspace(*end) && *end != 0) end++;
 if (w1 != NULL) {
  int i;
  for (i=0; i<5 && start+i < end; i++) w1[i] = toupper(start[i]);
  w1[i] = 0;
 }
 if (r1 != NULL) {
  int fullLen = end-start < MAX_INPUT_LENGTH ? end-start : MAX_INPUT_LENGTH;
   /* I don't know how ``end - start'' could be larger than MAX_INPUT_LENGTH,
    * but you can never be too careful. */
  strncpy(r1, start, fullLen);
  r1[fullLen] = 0;
 }
 if (w2 != NULL || r2 != NULL) {
  start = end;
  while (isspace(*start) && *start != '\n') start++;
  if (*start == 0 || *start == '\n') {
   if (w2 != NULL) *w2 = 0;
   if (r2 != NULL) *r2 = 0;
  } else {
   end = start;
   while (!isspace(*end) && *end != 0) end++;
   if (w2 != NULL) {
    int i;
    for (i=0; i<5 && start+i < end; i++) w2[i] = toupper(start[i]);
    w2[i] = 0;
   }
   if (r2 != NULL) {
    int fullLen = end-start < MAX_INPUT_LENGTH ? end-start : MAX_INPUT_LENGTH;
    strncpy(r2, start, fullLen);
    r2[fullLen] = 0;
   }
  }
 }
 if (strchr(end, '\n') == NULL) ftoeol();
}

void ftoeol(void) {
 int ch = 0;
 while (ch != '\n' && ch != EOF) ch = getchar();
}

int ran(int max) {
#ifdef ORIG_RNG
 static int r = 0;
 int d = 1;
 if (r == 0) {
  datime(&d, &r);
  r = 18 * r + 5;
  d = 1000 + d % 1000;
 }
 for (int i=0; i<d; i++) r = (r * 1021) % 1048576;
 return (max * r) / 1048576;
#elif defined(RANDOM_RNG)
 return random() % max;
#else
 return rand() % max;
#endif
}

#ifdef ADVMAGIC
void ciao(void) {
 mspeak(32);
 exit(0);
}

void mspeak(int msg) {
 if (msg != 0) speak(magicMsg[msg]);
}

bool yesm(int x, int y, int z) {
 for (;;) {
  mspeak(x);
  char reply[6];
  getin(reply, NULL, NULL, NULL); /* Ignore everything after the first word. */
  if (strcmp(reply, "YES") == 0 || strcmp(reply, "Y") == 0) {
   mspeak(y);
   return true;
  } else if (strcmp(reply, "NO") == 0 || strcmp(reply, "N") == 0) {
   mspeak(z);
   return false;
  } else printf("\nPlease answer the question.\n");
 }
}

bool start(void) {
 int d, t;
 datime(&d, &t);
 if (game.saved != -1) {
  int delay = (d - game.saved) * 1440 + (t - game.savet);
  if (delay < mage.latency) {
   printf("This adventure was suspended a mere %d minutes ago.\n", delay);
   if (delay < mage.latency/3) {mspeak(2); exit(0); }
   else {
    mspeak(8);
    if (wizard()) {game.saved = -1; return false; }
    mspeak(9);
    exit(0);
   }
  }
 }
 int32_t primet = mage.hbegin <= d && d <= mage.hend ? mage.holid
  : d % 7 <= 1 ? mage.wkend : mage.wkday;
 if (primet & 1 << t/60) {
  /* Prime time (cave closed) */
  mspeak(3);
  hours();
  mspeak(4);
  if (wizard()) {game.saved = -1; return false; }
  if (game.saved != -1) {mspeak(9); exit(0); }
  if (yesm(5, 7, 7)) {game.saved = -1; return true; }
  exit(0);
 }
 game.saved = -1;
 return false;
}

void maint(void) {
 if (!wizard()) return;
 blklin = false;
 if (yesm(10, 0, 0)) hours();
 if (yesm(11, 0, 0)) newhrs();
 if (yesm(26, 0, 0)) {
  mspeak(27);
  getin(word1, in1, NULL, NULL);
  mage.hbegin = (int) strtol(in1, NULL, 10);
  mspeak(28);
  getin(word1, in1, NULL, NULL);
  mage.hend = (int) strtol(in1, NULL, 10);
  int d, t;
  datime(&d, &t);
  mage.hbegin += d;
  mage.hend += mage.hbegin - 1;
  mspeak(29);
  getin(word1, in1, NULL, NULL);
  strncpy(mage.hname, in1, 20);
  mage.hname[20] = 0;
 }
 printf("Length of short game (null to leave at %d):\n", mage.shortGame);
 getin(word1, in1, NULL, NULL);
 int x = (int) strtol(in1, NULL, 10);
 if (x > 0) mage.shortGame = x;
 mspeak(12);
 getin(word1, NULL, NULL, NULL);
 if (*word1) {strncpy(mage.magic, word1, 5); mage.magic[5] = 0; }
 mspeak(13);
 getin(word1, in1, NULL, NULL);
 x = (int) strtol(in1, NULL, 10);
 if (x > 0) mage.magnm = x;
 printf("Latency for restart (null to leave at %d):\n", mage.latency);
 getin(word1, in1, NULL, NULL);
 x = (int) strtol(in1, NULL, 10);
 if (0 < x && x < 45) mspeak(30);
 if (x > 0) mage.latency = x < 45 ? 45 : x;
 if (yesm(14, 0, 0)) motd(true);
 mspeak(15);
 blklin = true;
 FILE* abra = fopen(MAGICFILE, "wb");
 if (abra == NULL) {
  perror("\nError: could not write to " MAGICFILE);
  exit(EXIT_FAILURE);
 }
 if (fwrite(&mage, sizeof mage, 1, abra) != 1) {
  perror("\nError writing to " MAGICFILE);
  exit(EXIT_FAILURE);
 }
 fclose(abra);
 ciao();
}

void poof(void) {
 FILE* abra = fopen(MAGICFILE, "rb");
 if (abra == NULL) return;
 /* If MAGICFILE cannot be opened, assume it does not exist and quietly leave
  * the default magic values in place. */
 struct advmagic backup = mage;
 if (fread(&mage, sizeof mage, 1, abra) != 1) {
  perror("\nWarning: error reading from " MAGICFILE);
  fputs("Using default magic values\n", stderr);
  mage = backup;
 }
 fclose(abra);
}

bool wizard(void) {
 if (!yesm(16, 0, 7)) return false;
 mspeak(17);
 char word[6];
 getin(word, NULL, NULL, NULL);
 if (strncmp(word, mage.magic, 5) != 0) {mspeak(20); return false; }
 int d, t;
 datime(&d, &t);
 t = t * 2 + 1;
 word[0] = word[1] = word[2] = word[3] = word[4] = 64;
 word[5] = 0;
 int val[5];
 for (int y=0; y<5; y++) {
  int x = 79 + d % 5;
  d /= 5;
  for (; x>0; x--) t = (t * 1027) % 1048576;
  word[y] += val[y] = (t*26) / 1048576 + 1;
 }
 if (yesm(18, 0, 0)) {mspeak(20); return false; }
 printf("\n%s\n", word);
 getin(word, NULL, NULL, NULL);
 datime(&d, &t);
 t = (t/60) * 40 + (t/10) * 10;
 d = mage.magnm;
 for (int y=0; y<5; y++) {
  if (word[y] != (abs(val[y] - val[(y+1) % 5]) * (d % 10) + (t % 10)) % 26
   + 65) {
   mspeak(20);
   return false;
  }
  t /= 10;
  d /= 10;
 }
 mspeak(19);
 return true;
}

void hours(void) {
 putchar('\n');
 hoursx(mage.wkday, "Mon - Fri:");
 hoursx(mage.wkend, "Sat - Sun:");
 hoursx(mage.holid, "Holidays: ");
 int d, t;
 datime(&d, &t);
 if (mage.hend < d || mage.hend < mage.hbegin) return;
 if (mage.hbegin > d) {
  d = mage.hbegin - d;
  printf("\nThe next holiday will be in %d day%s, namely %.20s.\n",
   d, d == 1 ? "" : "s", mage.hname);
 } else printf("\nToday is a holiday, namely %.20s.\n", mage.hname);
}

void hoursx(int32_t hours, const char* day) {
 bool first = true;
 int from = -1;
 if (hours == 0) {printf("%10s%s  Open all day\n", "", day); return; }
 for (;;) {
  do {from++; } while ((hours & 1 << from) && from < 24);
  if (from >= 24) {
   if (first) printf("%10s%s  Closed all day\n", "", day);
   return;
  } else {
   int till = from;
   do {till++; } while (!(hours & 1 << till) && till != 24);
   if (first) printf("%10s%s%4d:00 to%3d:00\n", "", day, from, till);
   else printf("%20s%4d:00 to%3d:00\n", "", from, till);
   first = false;
   from = till;
  }
 }
}

void newhrs(void) {
 mspeak(21);
 mage.wkday = newhrx("weekdays:");
 mage.wkend = newhrx("weekends:");
 mage.holid = newhrx("holidays:");
 mspeak(22);
 hours();
}

int32_t newhrx(const char* day) {
 int32_t hours = 0;
 printf("Prime time on %s\n", day);
 for (;;) {
  int from, till;
  printf("from:\n");
  getin(word1, in1, NULL, NULL);
  from = (int) strtol(in1, NULL, 10);
  if (from < 0 || from >= 24) return hours;
  printf("till:\n");
  getin(word1, in1, NULL, NULL);
  till = (int) strtol(in1, NULL, 10);
  if (till <= from || till > 24) return hours;
  for (; from < till; from++) hours |= 1 << from;
 }
}

void motd(bool alter) {
 if (alter) {
  memset(mage.msg, 0, sizeof mage.msg);
  mspeak(23);
  size_t msgLen = 0;
  for (;;) {
   char line[71];
   printf("> ");
   fgets(line, 70, stdin);
   if (*line == '\n') return;
   if (strchr(line, '\n') == NULL) {mspeak(24); ftoeol(); continue; }
   msgLen += strlen(line);
   strncat(mage.msg, line, 70);
  /* This doesn't exactly match the logic used in the original Fortran, but
   * it's close: */
   if (msgLen + 70 >= 500) {mspeak(25); return; }
  }
 } else if (*mage.msg) printf("%.499s", mage.msg);
}
#endif  /* #ifdef ADVMAGIC */

#if defined(ADVMAGIC) || defined(ORIG_RNG)
/* datime(d, t) is supposed to set *d to the number of days since 1 Jan 1977
 * and *t to the number of minutes since midnight according to the user's
 * timezone. */
void datime(int* d, int* t) {
 time_t now = time(NULL);
 struct tm* nower = localtime(&now);  /* nower - like now, but more so */
 int year = nower->tm_year - 77;
 *d = year*365 + nower->tm_yday + year/4 - (nower->tm_year-1)/100
  + (nower->tm_year+299)/400;
 *t = nower->tm_hour * 60 + nower->tm_min;
}
#endif
