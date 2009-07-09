#include <stddef.h>

const int actspk[32] = {
 0, 24, 29, 0, 33, 0, 33, 38,
 38, 42, 14, 43, 110, 29, 110, 73,
 75, 29, 13, 59, 59, 174, 109, 67,
 13, 147, 155, 195, 146, 110, 13, 13
};

const int cond[141] = {
 0, 5, 1, 5, 5, 1, 1, 5, 17, 1,
 1, 0, 0, 32, 0, 0, 0, 0, 0, 64,
 0, 0, 0, 0, 6, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 4, 0,
 0, 0, 128, 128, 128, 128, 136, 136, 136, 128,
 128, 128, 128, 128, 136, 128, 136, 0, 8, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 128, 128, 136, 0, 0, 8, 136, 128, 0, 0,
 0, 0, 0, 0, 0, 4, 0, 0, 0, 256,
 257, 256, 0, 0, 0, 0, 0, 0, 512, 0,
 0, 0, 0, 4, 0, 1, 1, 0, 0, 0,
 0, 0, 8, 8, 8, 8, 9, 8, 8, 8,
 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

const struct {int score; char* rank; } classes[] = {
 {35, "You are obviously a rank amateur.  Better luck next time.\n"},
 {100, "Your score qualifies you as a novice class adventurer.\n"},
 {130, "You have achieved the rating: \"Experienced Adventurer\".\n"},
 {200, "You may now consider yourself a \"Seasoned Adventurer\".\n"},
 {250, "You have reached \"Junior Master\" status.\n"},
 {300, "Your score puts you in Master Adventurer Class C.\n"},
 {330, "Your score puts you in Master Adventurer Class B.\n"},
 {349, "Your score puts you in Master Adventurer Class A.\n"},
 {9999, "All of Adventuredom gives tribute to you, Adventurer Grandmaster!\n"},
 {0, NULL}
};

const int hints[10][4] = {
 {0, 0, 0, 0},
 {0, 0, 0, 0},
 {9999, 10, 0, 0},
 {9999, 5, 0, 0},
 {4, 2, 62, 63},
 {5, 2, 18, 19},
 {8, 2, 20, 21},
 {75, 4, 176, 177},
 {25, 5, 178, 179},
 {20, 3, 180, 181}
};
