#define _XOPEN_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>

void usage(void);

int main(int argc, char** argv) {
 int magnm = 11111;
 time_t now = time(NULL);
 struct tm* nower = localtime(&now);
 int hour = nower->tm_hour, minute = nower->tm_min;
 int ch;
 while ((ch = getopt(argc, argv, "m:H:M:")) != -1) {
  switch (ch) {
   case 'm': magnm = strtol(optarg, NULL, 10); break;
   case 'H': hour = strtol(optarg, NULL, 10); break;
   case 'M': minute = strtol(optarg, NULL, 10); break;
   default: usage(); return 2;
  }
 }
 char* word;
 if (optind < argc) word = argv[optind];
 else {usage(); return 2; }
 int val[5];
 for (int i=0; i<5; i++) {
  if (word[i] == 0) {printf("That word is too short!\n"); return 2; }
  val[i] = toupper(word[i]) - 64;
  if (val[i] < 1 || val[i] > 26) {
   printf("The word should contain exactly five ASCII letters.\n");
   return 2;
  }
 }
 if (word[5] != 0) {printf("That word is too long!\n"); return 2; }
 int t = hour * 100 + minute - minute % 10;
 int d = magnm;
 for (int y=0; y<5; y++) {
  putchar((abs(val[y] - val[(y+1) % 5]) * (d % 10) + (t % 10)) % 26 + 65);
  t /= 10;
  d /= 10;
 }
 putchar('\n');
 return 0;
}

void usage(void) {
 fprintf(stderr, "Usage: frawd [-m magic number] [-H hour] [-M minute] word\n"
  "`word' should consist of exactly five ASCII letters.\n");
}
