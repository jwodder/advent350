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

void speak(char* s) {
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
  } else {printf("Please answer the question.\n"); }
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
 if (atloc[where] == obj) {atloc[where] = link[obj]; }
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
 printf("Error code = %d\n", num);
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
 putchar('\n');
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
#ifdef ORIG_RAND
 static int r = 0;
 int d = 1;
 if (r == 0) {
  datime(&d, &r);
  r = 18 * r + 5;
  d = 1000 + d % 1000;
 }
 for (int i=0; i<d; i++) r = (r * 1021) % 1048576;
 return (max * r) / 1048576;
#else
 return rand() % max;
#endif
}
