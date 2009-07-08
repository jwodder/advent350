void turn(void);
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
void speak(char* s);
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
int ran(int max);
void domove(int motion);
void dotrav(int motion);
void death(void);
int score(bool scoring);
void normend(void);
void doaction(void);
bool dwarfHere(void);
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
void vresume(char* file);

#ifdef ADVMAGIC
void ciao(void);
void mspeak(int msg);
bool yesm(int x, int y, int z);
bool start(void);
void maint(void);
bool wizard(void);
void hours(void);
void hoursx(const bool* hours, const char* day);
void newhrs(void);
void newhrx(bool* hours, const char* day);
void motd(bool alter);
void poof(void);
#endif

#if defined(ADVMAGIC) || defined(ORIG_RAND)
void datime(int* d, int* t);
#endif
