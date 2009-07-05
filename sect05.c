const char* itemDesc[65][7] = {
 {NULL},

 /* 1 */
 {"Set of keys\n",
  "There are some keys on the ground here.\n"},
 
 /* 2 */
 {"Brass lantern\n",
  "There is a shiny brass lamp nearby.\n",
  "There is a lamp shining nearby.\n"},

 /* 3 */
 {"*grate\n",
  "The grate is locked.\n",
  "The grate is open.\n"},

 /* 4 */
 {"Wicker cage\n",
  "There is a small wicker cage discarded nearby.\n"},

 /* 5 */
 {"Black rod\n",
  "A three foot black rod with a rusty star on an end lies nearby.\n"},

 /* 6 */
 {"Black rod\n",
  "A three foot black rod with a rusty mark on an end lies nearby.\n"},

 /* 7 */
 {"*steps\n",
  "Rough stone steps lead down the pit.\n",
  "Rough stone steps lead up the dome.\n"},

 /* 8 */
 {"Little bird in cage\n",
  "A cheerful little bird is sitting here singing.\n",
  "There is a little bird in the cage.\n"},

 /* 9 */
 {"*rusty door\n",
  "The way north is barred by a massive, rusty, iron door.\n",
  "The way north leads through a massive, rusty, iron door.\n"},

 /* 10 */
 {"Velvet pillow\n",
  "A small velvet pillow lies on the floor.\n"},

 /* 11 */
 {"*snake\n",
  "A huge green fierce snake bars the way!\n",
  NULL /* chased away */ },

 /* 12 */
 {"*fissure\n",
  NULL,
  "A crystal bridge now spans the fissure.\n",
  "The crystal bridge has vanished!\n"},

 /* 13 */
 {"*stone tablet\n",
  "A massive stone tablet embedded in the wall reads:\n"
  "\"Congratulations on bringing light into the dark-room!\"\n"},

 /* 14 */
 {"Giant clam  >GRUNT!<\n",
  "There is an enormous clam here with its shell tightly closed.\n"},

 /* 15 */
 {"Giant oyster  >GROAN!<\n",
  "There is an enormous oyster here with its shell tightly closed.\n",
  "Interesting.  There seems to be something written on the underside of\n"
  "the oyster.\n"},

 /* 16 */
 {"\"Spelunker Today\"\n",
  "There are a few recent issues of \"Spelunker Today\" magazine here.\n"},
 
 {NULL},
 {NULL},

 /* 19 */
 {"Tasty food\n",
  "There is food here.\n"},

 /* 20 */
 {"Small bottle\n",
  "There is a bottle of water here.\n",
  "There is an empty bottle here.\n",
  "There is a bottle of oil here.\n"},

 /* 21 */
 {"Water in the bottle\n"},

 /* 22 */
 {"Oil in the bottle\n"},

 /* 23 */
 {"*mirror\n", NULL},

 /* 24 */
 {"*plant\n",
  "There is a tiny little plant in the pit, murmuring \"water, water, ...\"\n",
  "The plant spurts into furious growth for a few seconds.\n",
  "There is a 12-foot-tall beanstalk stretching up out of the pit,\n"
  "bellowing \"WATER!! WATER!!\"\n",
  "The plant grows explosively, almost filling the bottom of the pit.\n",
  "There is a gigantic beanstalk stretching all the way up to the hole.\n",
  "You've over-watered the plant!  It's shriveling up!  It's, it's...\n"},

 /* 25 */
 {"*phony plant\n", /* seen in Twopit Room only when tall enough */
  NULL,
  "The top of a 12-foot-tall beanstalk is poking out of the west pit.\n",
  "There is a huge beanstalk growing out of the west pit up to the hole.\n"},

 /* 26 */
 {"*stalactite\n", NULL},

 /* 27 */
 {"*shadowy figure\n",
  "The shadowy figure seems to be trying to attract your attention.\n"},

 /* 28 */
 {"Dwarf's axe\n",
  "There is a little axe here.\n",
  "There is a little axe lying beside the bear.\n"},

 /* 29 */
 {"*cave drawings\n", NULL},

 /* 30 */
 {"*pirate\n", NULL},

 /* 31 */
 {"*dragon\n",
  "A huge green fierce dragon bars the way!\n",
  "Congratulations!  You have just vanquished a dragon with your bare\n"
  "hands!  (Unbelievable, isn't it?)\n",
  "The body of a huge green dead dragon is lying off to one side.\n"},

 /* 32 */
 {"*chasm\n",
  "A rickety wooden bridge extends across the chasm, vanishing into the\n"
  "mist.  A sign posted on the bridge reads, \"Stop! Pay troll!\"\n",
  "The wreckage of a bridge (and a dead bear) can be seen at the bottom\n"
  "of the chasm.\n"},

 /* 33 */
 {"*troll\n",
  "A burly troll stands by the bridge and insists you throw him a\n"
  "treasure before you may cross.\n",
  "The troll steps out from beneath the bridge and blocks your way.\n",
  NULL /* chased away */ },

 /* 34 */
 {"*phony troll\n",
  "The troll is nowhere to be seen.\n"},

 /* 35 */
 {NULL, /* bear uses rtext 141 */
  "There is a ferocious cave bear eying you from the far end of the room!\n",
  "There is a gentle cave bear sitting placidly in one corner.\n",
  "There is a contented-looking bear wandering about nearby.\n",
  NULL /* dead */ },

 /* 36 */
 {"*message in second maze\n",
  "There is a message scrawled in the dust in a flowery script, reading:\n"
  "\"This is not the maze where the pirate leaves his treasure chest.\"\n"},

 /* 37 */
 {"*volcano and/or geyser\n", NULL},

 /* 38 */
 {"*vending machine\n",
  "There is a massive vending machine here.  The instructions on it read:\n"
  "\"Drop coins here to receive fresh batteries.\"\n"},

 /* 39 */
 {"Batteries\n",
  "There are fresh batteries here.\n",
  "Some worn-out batteries have been discarded nearby.\n"},

 /* 40 */
 {"*carpet and/or moss\n", NULL},

 {NULL},
 {NULL},
 {NULL},
 {NULL},
 {NULL},
 {NULL},
 {NULL},
 {NULL},
 {NULL},

 /* 50 */
 {"Large gold nugget\n",
  "There is a large sparkling nugget of gold here!\n"},

 /* 51 */
 {"Several diamonds\n",
  "There are diamonds here!\n"},

 /* 52 */
 {"Bars of silver\n",
  "There are bars of silver here!\n"},

 /* 53 */
 {"Precious jewelry\n",
  "There is precious jewelry here!\n"},

 /* 54 */
 {"Rare coins\n",
  "There are many coins here!\n"},

 /* 55 */
 {"Treasure chest\n",
  "The pirate's treasure chest is here!\n"},

 /* 56 */
 {"Golden eggs\n",
  "There is a large nest here, full of golden eggs!\n",
  "The nest of golden eggs has vanished!\n",
  "Done!\n"},

 /* 57 */
 {"Jeweled trident\n",
  "There is a jewel-encrusted trident here!\n"},

 /* 58 */
 {"Ming vase\n",
  "There is a delicate, precious, ming vase here!\n",
  "The vase is now resting, delicately, on a velvet pillow.\n",
  "The floor is littered with worthless shards of pottery.\n",
  "The ming vase drops with a delicate crash.\n"},

 /* 59 */
 {"Egg-sized emerald\n",
  "There is an emerald here the size of a plover's egg!\n"},

 /* 60 */
 {"Platinum pyramid\n",
  "There is a platinum pyramid here, 8 inches on a side!\n"},

 /* 61 */
 {"Glistening pearl\n",
  "Off to one side lies a glistening pearl!\n"},

 /* 62 */
 {"Persian rug\n",
  "There is a Persian rug spread out on the floor!\n",
  "The dragon is sprawled out on a Persian rug!!\n"},

 /* 63 */
 {"Rare spices\n",
  "There are rare spices here!\n"},

 /* 64 */
 {"Golden chain\n",
  "There is a golden chain lying in a heap on the floor!\n",
  "The bear is locked to the wall with a golden chain!\n",
  "There is a golden chain locked to the wall!\n"}
};
