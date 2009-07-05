#ifdef ADVMAGIC

void ciao(void) {
 mspeak(32);
 exit(0);
}

void mspeak(int msg) {
 if (msg != 0) speak(magicMsg(msg));
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
  } else {printf("Please answer the question.\n"); }
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
 bool* primet = hbegin <= d && d <= hend ? holid : d % 7 <= 1 ? wkend : wkday;
 if (primet[t/60]) {
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
  printf("> ");
  scanf("%d", &hbegin);
  mspeak(28);
  printf("> ");
  scanf("%d", &hend);
  int d, t;
  datime(&d, &t);
  hbegin += d;
  hend += hbegin - 1;
  mspeak(29);
  printf("> ");
  scanf("%20s", hname);
 }
 printf("Length of short game (null to leave at %d):\n> ", shortGame);
 int x;
 scanf("%d", &x);
 if (x > 0) shortGame = x;
 mspeak(12);
 char shamwow[6];
 getin(shamwow, NULL, NULL, NULL);
 if (*shamwow) strcpy(magic, shamwow);
 mspeak(13);
 printf("> ");
 scanf("%d", &x);
 if (x > 0) magnm = x;
 printf("Latency for restart (null to leave at %d):\n> ", latency);
 scanf("%d", &x);
 if (0 < x && x < 45) mspeak(30);
 if (x > 0) latency = x < 45 ? 45 : x;
 if (yesm(14, 0, 0)) motd(true);
 mspeak(15);  /* Say something else? */
 blklin = true;

 /*****
  # Save values to MAGICFILE
  my IO $abra = open MAGICFILE, :w, :bin;
  writeBool $abra, @wkday;
  writeBool $abra, @wkend;
  writeBool $abra, @holid;
  writeInt $abra, $hbegin;
  writeInt $abra, $hend;
  # write out $hname
  writeInt $abra, $shortGame;
  # write out $magic
  writeInt $abra, $magnm;
  writeInt $abra, $latency;
  # write out $msg
 *****/

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
 printf(" %s\n", word);
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
  printf("The next holiday will be in %d day%s, namely %s.\n",
   d, d == 1 ? "" : "s", hname);
 } else printf("Today is a holiday, namely %s.\n", hname);
}

void hoursx(const bool* hours, const char* day) {
 bool first = true;
 int from = -1;
 for (;;) {
  do from++ while (hours[from] && from < 24);
  if (from >= 24) {
   if (first) printf("%10s%s Closed all day\n", "", day);
   return;
  } else {
   int till = from;
   do till++ while (!hours[till] && till != 24);
   if (from == 0 && till == 24) {
    printf("%10s%s Open all day\n", "", day);
    return;
   } else if (first) printf("%10s%s%4d:00 to%3d:00\n", "", day, from, till);
   else printf("%20s%4d:00 to%3d:00\n", "", from, till);
   first = false;
   from = till;
  }
 }
}

void newhrs(void) {
 mspeak(21);
 newhrx(wkday, "weekdays:");
 newhrx(wkend, "weekends:");
 newhrx(holid, "holidays:");
 mspeak(22);
 hours();
}

void newhrx(bool* hours, const char* day) {
 for (int i=0; i<24; i++) hours[i] = false;
 printf("Prime time on %s\n", day);
 for (;;) {
  int from, till;
  printf("from: ");
  scanf("%d", &from);
  if (from < 0 || from >= 24) return;
  printf("till: ");
  scanf("%d", &till);
  if (till <= from || till > 24) return;
  for (; from < till; from++) hours[from] = true;
 }
}

void motd(bool alter) {
 if (alter) {
  memset(msg, 0, sizeof msg);
  mspeak(23);
  size_t msgLen = 0;
  for (;;) {
   char line[71];
   print("> ");
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

sub poof() {
 #<
  # Read in values from MAGICFILE
  my IO $abra = open MAGICFILE, :r, :bin;
  @wkday = readBool $abra, +@wkday;
  @wkend = readBool $abra, +@wkend;
  @holid = readBool $abra, +@holid;
  $hbegin = readInt $abra;
  $hend = readInt $abra;
  # read in $hname
  $shortGame = readInt $abra;
  # read in $magic
  $magnm = readInt $abra;
  $latency = readInt $abra;
  # read in $msg
 >
}

void datime(int* d, int* t) {
 time_t now = time(NULL);
 *d = (now - 220924800) / 86400;
 *t = (now % 86400) / 60;
}

#endif
