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
 return place[item] == -1;
}

bool here(int item) {
 return place[item] == loc || toting(item);
}

bool at(int item) {
 return loc == place[item] || loc == fixed[item];
}

int liq2(int p) {
 return p == 0 ? WATER : p == 2 ? OIL : 0;
}

int liq(void) {
 return liq2(prop[BOTTLE] < 0 ? -1-prop[BOTTLE] : prop[BOTTLE]);
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
 return !(cond[loc] & 1 || (prop[LAMP] && here(LAMP)));
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
 move(obj, place[obj]);
 move(obj+100, fixed[obj]);
}

void move(int obj, int where) {
 int from = obj > 100 ? fixed[obj-100] : place[obj];
 if (0 < from && from <= 300) carry(obj, from);
 drop(obj, where);
}

int put(int obj, int where, int pval) {
 move(obj, where);
 return -1 - pval;
}

void carry(int obj, int where) {
 if (obj <= 100) {
  if (place[obj] == -1) return;
  place[obj] = -1;
  holding++;
 }
 if (atloc[where] == obj) atloc[where] = link[obj];
 else {
  int tmp;
  for (tmp = atloc[where]; link[tmp] != obj; tmp = link[tmp]);
  link[tmp] = link[obj];
 }
}

void drop(int obj, int where) {
 if (obj > 100) {
  fixed[obj-100] = where;
 } else {
  if (place[obj] == -1) holding--;
  place[obj] = where;
 }
 if (where > 0) {
  link[obj] = atloc[where];
  atloc[where] = obj;
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

 /* printf("Probable cause: erroneous info in database.\n");
  * (Not in this version) */
 printf("Error code = %d\n\n", num);
 exit(1);
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

void getin(char* w1, char* r1, char* w2, char* r2) {
 static char line[MAX_INPUT_LENGTH+1];
 if (blklin) putchar('\n');
 for (;;) {
  printf("> ");
  fgets(line, MAX_INPUT_LENGTH+1, stdin);
  char* start1 = line;
  while (isspace(*start1) && *start1 != 0) start1++;
  if (*start1 == 0) {
   if (blklin) continue;
   else {
    *w1 = 0;
    if (r1 != NULL) *r1 = 0;
    if (w2 != NULL) *w2 = 0;
    if (r2 != NULL) *r2 = 0;
    return;
   }
  }
  char* end1 = start1;
  while (!isspace(*end1) && *end1 != 0) end1++;
  int i;
  for (i=0; i<5 && start1 + i < end1; i++) w1[i] = toupper(start1[i]);
  w1[i] = 0;
  if (r1 != NULL) {
   strncpy(r1, start1, end1 - start1);
   r1[end1-start1] = 0;
  }
  if (w2 != NULL) {
   char* start2 = end1;
   while (isspace(*start2) && *start2 != 0) start2++;
   if (*start2 == 0) {*w2 = 0; if (r2 != NULL) *r2 = 0; }
   else {
    char* end2 = start2;
    while (!isspace(*end2) && *end2 != 0) end2++;
    for (i=0; i<5 && start2 + i < end2; i++) w2[i] = toupper(start2[i]);
    w2[i] = 0;
    if (r2 != NULL) {
     strncpy(r2, start2, end2 - start2);
     r2[end2-start2] = 0;
    }
   }
  }
  return;
 }
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
 if (saved != -1) {
  int delay = (d - saved) * 1440 + (t - savet);
  if (delay < latency) {
   printf("This adventure was suspended a mere %d minutes ago.\n", delay);
   if (delay < latency/3) {mspeak(2); exit(0); }
   else {
    mspeak(8);
    if (wizard()) {saved = -1; return false; }
    mspeak(9);
    exit(0);
   }
  }
 }
 int32_t primet = hbegin <= d && d <= hend ? holid : d % 7 <= 1 ? wkend : wkday;
 if (primet & 1 << t/60) {
  /* Prime time (cave closed) */
  mspeak(3);
  hours();
  mspeak(4);
  if (wizard()) {saved = -1; return false; }
  if (saved != -1) {mspeak(9); exit(0); }
  if (yesm(5, 7, 7)) {saved = -1; return true; }
  exit(0);
 }
 saved = -1;
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
  hbegin = (int) strtol(in1, NULL, 10);
  mspeak(28);
  getin(word1, in1, NULL, NULL);
  hend = (int) strtol(in1, NULL, 10);
  int d, t;
  datime(&d, &t);
  hbegin += d;
  hend += hbegin - 1;
  mspeak(29);
  getin(word1, in1, NULL, NULL);
  strncpy(hname, in1, 20);
  hname[20] = 0;
 }
 printf("Length of short game (null to leave at %d):\n", shortGame);
 getin(word1, in1, NULL, NULL);
 int x = (int) strtol(in1, NULL, 10);
 if (x > 0) shortGame = x;
 mspeak(12);
 getin(word1, NULL, NULL, NULL);
 if (*word1) {strncpy(magic, word1, 5); magic[5] = 0; }
 mspeak(13);
 getin(word1, in1, NULL, NULL);
 x = (int) strtol(in1, NULL, 10);
 if (x > 0) magnm = x;
 printf("Latency for restart (null to leave at %d):\n", latency);
 getin(word1, in1, NULL, NULL);
 x = (int) strtol(in1, NULL, 10);
 if (0 < x && x < 45) mspeak(30);
 if (x > 0) latency = x < 45 ? 45 : x;
 if (yesm(14, 0, 0)) motd(true);
 mspeak(15);
 blklin = true;

 FILE* abra = fopen(MAGICFILE, "wb");
 if (abra == NULL) {
  perror("\nError: could not write to " MAGICFILE);
  exit(1);
 }
 /* Check all of these function calls for failure! */
 fwrite(&wkday, sizeof wkday, 1, abra);
 fwrite(&wkend, sizeof wkend, 1, abra);
 fwrite(&holid, sizeof holid, 1, abra);
 fwrite(&hbegin, sizeof hbegin, 1, abra);
 fwrite(&hend, sizeof hend, 1, abra);
 fwrite(hname, 1, sizeof hname, abra);
 fwrite(&shortGame, sizeof shortGame, 1, abra);
 fwrite(magic, 1, sizeof magic, abra);
 fwrite(&magnm, sizeof magnm, 1, abra);
 fwrite(&latency, sizeof latency, 1, abra);
 fwrite(msg, 1, sizeof msg, abra);
 fclose(abra);

 ciao();
}

bool wizard(void) {
 if (!yesm(16, 0, 7)) return false;
 mspeak(17);
 char word[6];
 getin(word, NULL, NULL, NULL);
 if (strcmp(word, magic) != 0) {mspeak(20); return false; }
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
 printf("\n %s\n", word);
 getin(word, NULL, NULL, NULL);
 datime(&d, &t);
 t = (t/60) * 40 + (t/10) * 10;
 d = magnm;
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
 hoursx(wkday, "Mon - Fri:");
 hoursx(wkend, "Sat - Sun:");
 hoursx(holid, "Holidays: ");
 int d, t;
 datime(&d, &t);
 if (hend < d || hend < hbegin) return;
 if (hbegin > d) {
  d = hbegin - d;
  printf("\nThe next holiday will be in %d day%s, namely %s.\n",
   d, d == 1 ? "" : "s", hname);
 } else printf("\nToday is a holiday, namely %s.\n", hname);
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
 wkday = newhrx("weekdays:");
 wkend = newhrx("weekends:");
 holid = newhrx("holidays:");
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
  memset(msg, 0, sizeof msg);
  mspeak(23);
  size_t msgLen = 0;
  for (;;) {
   char line[71];
   printf("> ");
   fgets(line, 70, stdin);
   if (*line == '\n') return;
   if (strchr(line, '\n') == NULL) {mspeak(24); fpurge(stdin); continue; }
   msgLen += strlen(line);
   strncat(msg, line, 70);
  /* This doesn't exactly match the logic used in the original Fortran, but
   * it's close: */
   if (msgLen + 70 >= 500) {mspeak(25); return; }
  }
 } else if (*msg) fputs(msg, stdout);
}

void poof(void) {
 FILE* abra = fopen(MAGICFILE, "rb");
 if (abra == NULL) return;
 /* If MAGICFILE cannot be opened, assume it does not exist and quietly leave
  * the default magic values in place. */

 /* Check all of these function calls for failure! */
 fread(&wkday, sizeof wkday, 1, abra);
 fread(&wkend, sizeof wkend, 1, abra);
 fread(&holid, sizeof holid, 1, abra);
 fread(&hbegin, sizeof hbegin, 1, abra);
 fread(&hend, sizeof hend, 1, abra);
 fread(hname, 1, sizeof hname, abra);
 fread(&shortGame, sizeof shortGame, 1, abra);
 fread(magic, 1, sizeof magic, abra);
 fread(&magnm, sizeof magnm, 1, abra);
 fread(&latency, sizeof latency, 1, abra);
 fread(msg, 1, sizeof msg, abra);
 fclose(abra);
}
#endif  /* #ifdef ADVMAGIC */

#if defined(ADVMAGIC) || defined(ORIG_RNG)
void datime(int* d, int* t) {

/* This function is supposed to set *d to the number of days since 1 Jan 1977
 * and *t to the number of minutes since midnight.  Implementing this by
 * performing basic arithmetic on the return value of time() (assuming POSIX
 * compliance) doesn't work, as Unix time is measured according to UTC, so in
 * any other timezone the cave's hours won't match the local time.  Thus, the
 * time needs to be adjusted for the current timezone, and standard C provides
 * only one way to do this. */

 time_t now = time(NULL);
 struct tm* nower = localtime(&now);  /* nower - like now, but more so */
 int year = nower->tm_year - 77;
 *d = year*365 + nower->tm_yday + year/4 - (nower->tm_year-1)/100
  + (nower->tm_year+299)/400;
 *t = nower->tm_hour * 60 + nower->tm_min;
}
#endif
