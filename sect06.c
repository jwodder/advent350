const char* rmsg[] = {
 NULL,

 /* 1 */
 "Somewhere nearby is Colossal Cave, where others have found fortunes in\n"
 "treasure and gold, though it is rumored that some who enter are never\n"
 "seen again.  Magic is said to work in the cave.  I will be your eyes\n"
 "and hands.  Direct me with commands of 1 or 2 words.  I should warn\n"
 "you that I look at only the first five letters of each word, so you'll\n"
 "have to enter \"northeast\" as \"ne\" to distinguish it from \"north\".\n"
 "(Should you get stuck, type \"help\" for some general hints.  For infor-\n"
 "mation on how to end your adventure, etc., type \"info\".)\n"
 "\t\t\t      - - -\n"
 "This program was originally developed by Willie Crowther.  Most of the\n"
 "features of the current program were added by Don Woods (don @ su-ai).\n"
 "Contact Don if you have any questions, comments, etc.\n",

 /* 2 */
 "A little dwarf with a big knife blocks your way.\n",

 /* 3 */
 "A little dwarf just walked around a corner, saw you, threw a little\n"
 "axe at you which missed, cursed, and ran away.\n",

 /* 4 */
 "There is a threatening little dwarf in the room with you!\n",

 /* 5 */
 "One sharp nasty knife is thrown at you!\n",

 /* 6 */
 "None of them hit you!\n",

 /* 7 */
 "One of them gets you!\n",

 /* 8 */
 "A hollow voice says \"PLUGH\".\n",

 /* 9 */
 "There is no way to go that direction.\n",

 /* 10 */
 "I am unsure how you are facing.  Use compass points or nearby objects.\n",

 /* 11 */
 "I don't know in from out here.  Use compass points or name something\n"
 "in the general direction you want to go.\n",

 /* 12 */
 "I don't know how to apply that word here.\n",

 /* 13 */
 "I don't understand that!\n",

 /* 14 */
 "I'm game.  Would you care to explain how?\n",

 /* 15 */
 "Sorry, but I am not allowed to give more detail.  I will repeat the\n"
 "long description of your location.\n",

 /* 16 */
 "It is now pitch dark.  If you proceed you will likely fall into a pit.\n",

 /* 17 */
 "If you prefer, simply type w rather than west.\n",

 /* 18 */
 "Are you trying to catch the bird?\n",

 /* 19 */
 "The bird is frightened right now and you cannot catch it no matter\n"
 "what you try.  Perhaps you might try later.\n",

 /* 20 */
 "Are you trying to somehow deal with the snake?\n",

 /* 21 */
 "You can't kill the snake, or drive it away, or avoid it, or anything\n"
 "like that.  There is a way to get by, but you don't have the necessary\n"
 "resources right now.\n",

 /* 22 */
 "Do you really want to quit now?\n",

 /* 23 */
 "You fell into a pit and broke every bone in your body!\n",

 /* 24 */
 "You are already carrying it!\n",

 /* 25 */
 "You can't be serious!\n",

 /* 26 */
 "The bird was unafraid when you entered, but as you approach it becomes\n"
 "disturbed and you cannot catch it.\n",

 /* 27 */
 "You can catch the bird, but you cannot carry it.\n",

 /* 28 */
 "There is nothing here with a lock!\n",

 /* 29 */
 "You aren't carrying it!\n",

 /* 30 */
 "The little bird attacks the green snake, and in an astounding flurry\n"
 "drives the snake away.\n",

 /* 31 */
 "You have no keys!\n",

 /* 32 */
 "It has no lock.\n",

 /* 33 */
 "I don't know how to lock or unlock such a thing.\n",

 /* 34 */
 "It was already locked.\n",

 /* 35 */
 "The grate is now locked.\n",

 /* 36 */
 "The grate is now unlocked.\n",

 /* 37 */
 "It was already unlocked.\n",

 /* 38 */
 "You have no source of light.\n",

 /* 39 */
 "Your lamp is now on.\n",

 /* 40 */
 "Your lamp is now off.\n",

 /* 41 */
 "There is no way to get past the bear to unlock the chain, which is\n"
 "probably just as well.\n",

 /* 42 */
 "Nothing happens.\n",

 /* 43 */
 "Where?\n",

 /* 44 */
 "There is nothing here to attack.\n",

 /* 45 */
 "The little bird is now dead.  Its body disappears.\n",

 /* 46 */
 "Attacking the snake both doesn't work and is very dangerous.\n",

 /* 47 */
 "You killed a little dwarf.\n",

 /* 48 */
 "You attack a little dwarf, but he dodges out of the way.\n",

 /* 49 */
 "With what?  Your bare hands?\n",

 /* 50 */
 "Good try, but that is an old worn-out magic word.\n",

 /* 51 */
 "I know of places, actions, and things.  Most of my vocabulary\n"
 "describes places and is used to move you there.  To move, try words\n"
 "like forest, building, downstream, enter, east, west, north, south,\n"
 "up, or down.  I know about a few special objects, like a black rod\n"
 "hidden in the cave.  These objects can be manipulated using some of\n"
 "the action words that I know.  Usually you will need to give both the\n"
 "object and action words (in either order), but sometimes I can infer\n"
 "the object from the verb alone.  Some objects also imply verbs; in\n"
 "particular, \"inventory\" implies \"take inventory\", which causes me to\n"
 "give you a list of what you're carrying.  The objects have side\n"
 "effects; for instance, the rod scares the bird.  Usually people having\n"
 "trouble moving just need to try a few more words.  Usually people\n"
 "trying unsuccessfully to manipulate an object are attempting something\n"
 "beyond their (or my!) capabilities and should try a completely\n"
 "different tack.  To speed the game you can sometimes move long\n"
 "distances with a single word.  For example, \"building\" usually gets\n"
 "you to the building from anywhere above ground except when lost in the\n"
 "forest.  Also, note that cave passages turn a lot, and that leaving a\n"
 "room to the north does not guarantee entering the next from the south.\n"
 "Good luck!\n",

 /* 52 */
 "It misses!\n",

 /* 53 */
 "It gets you!\n",

 /* 54 */
 "OK\n",

 /* 55 */
 "You can't unlock the keys.\n",

 /* 56 */
 "You have crawled around in some little holes and wound up back in the\n"
 "main passage.\n",

 /* 57 */
 "I don't know where the cave is, but hereabouts no stream can run on\n"
 "the surface for long.  I would try the stream.\n",

 /* 58 */
 "I need more detailed instructions to do that.\n",

 /* 59 */
 "I can only tell you what you see as you move about and manipulate\n"
 "things.  I cannot tell you where remote things are.\n",

 /* 60 */
 "I don't know that word.\n",

 /* 61 */
 "What?\n",

 /* 62 */
 "Are you trying to get into the cave?\n",

 /* 63 */
 "The grate is very solid and has a hardened steel lock.  You cannot\n"
 "enter without a key, and there are no keys nearby.  I would recommend\n"
 "looking elsewhere for the keys.\n",

 /* 64 */
 "The trees of the forest are large hardwood oak and maple, with an\n"
 "occasional grove of pine or spruce.  There is quite a bit of under-\n"
 "growth, largely birch and ash saplings plus nondescript bushes of\n"
 "various sorts.  This time of year visibility is quite restricted by\n"
 "all the leaves, but travel is quite easy if you detour around the\n"
 "spruce and berry bushes.\n",

 /* 65 */
 "Welcome to Adventure!!  Would you like instructions?\n",

 /* 66 */
 "Digging without a shovel is quite impractical.  Even with a shovel\n"
 "progress is unlikely.\n",

 /* 67 */
 "Blasting requires dynamite.\n",

 /* 68 */
 "I'm as confused as you are.\n",

 /* 69 */
 "Mist is a white vapor, usually water, seen from time to time in\n"
 "caverns.  It can be found anywhere but is frequently a sign of a deep\n"
 "pit leading down to water.\n",

 /* 70 */
 "Your feet are now wet.\n",

 /* 71 */
 "I think I just lost my appetite.\n",

 /* 72 */
 "Thank you, it was delicious!\n",

 /* 73 */
 "You have taken a drink from the stream.  The water tastes strongly of\n"
 "minerals, but is not unpleasant.  It is extremely cold.\n",

 /* 74 */
 "The bottle of water is now empty.\n",

 /* 75 */
 "Rubbing the electric lamp is not particularly rewarding.  Anyway,\n"
 "nothing exciting happens.\n",

 /* 76 */
 "Peculiar.  Nothing unexpected happens.\n",

 /* 77 */
 "Your bottle is empty and the ground is wet.\n",

 /* 78 */
 "You can't pour that.\n",

 /* 79 */
 "Watch it!\n",

 /* 80 */
 "Which way?\n",

 /* 81 */
 "Oh dear, you seem to have gotten yourself killed.  I might be able to\n"
 "help you out, but I've never really done this before.  Do you want me\n"
 "to try to reincarnate you?\n",

 /* 82 */
 "All right.  But don't blame me if something goes wr......\n"
 "\t\t    --- POOF!! ---\n"
 "You are engulfed in a cloud of orange smoke.  Coughing and gasping,\n"
 "you emerge from the smoke and find....\n",

 /* 83 */
 "You clumsy oaf, you've done it again!  I don't know how long I can\n"
 "keep this up.  Do you want me to try reincarnating you again?\n",

 /* 84 */
 "Okay, now where did I put my orange smoke?....  >POOF!<\n"
 "Everything disappears in a dense cloud of orange smoke.\n",

 /* 85 */
 "Now you've really done it!  I'm out of orange smoke!  You don't expect\n"
 "me to do a decent reincarnation without any orange smoke, do you?\n",

 /* 86 */
 "Okay, if you're so smart, do it yourself!  I'm leaving!\n",

 NULL,

 NULL,

 NULL,

 /* 90 */
 ">>> messages 81 thru 90 are reserved for \"obituaries\". <<<\n",

 /* 91 */
 "Sorry, but I no longer seem to remember how it was you got here.\n",

 /* 92 */
 "You can't carry anything more.  You'll have to drop something first.\n",

 /* 93 */
 "You can't go through a locked steel grate!\n",

 /* 94 */
 "I believe what you want is right here with you.\n",

 /* 95 */
 "You don't fit through a two-inch slit!\n",

 /* 96 */
 "I respectfully suggest you go across the bridge instead of jumping.\n",

 /* 97 */
 "There is no way across the fissure.\n",

 /* 98 */
 "You're not carrying anything.\n",

 /* 99 */
 "You are currently holding the following:\n",

 /* 100 */
 "It's not hungry (it's merely pinin' for the fjords).  Besides, you\n"
 "have no bird seed.\n",

 /* 101 */
 "The snake has now devoured your bird.\n",

 /* 102 */
 "There's nothing here it wants to eat (except perhaps you).\n",

 /* 103 */
 "You fool, dwarves eat only coal!  Now you've made him *REALLY* mad!!\n",

 /* 104 */
 "You have nothing in which to carry it.\n",

 /* 105 */
 "Your bottle is already full.\n",

 /* 106 */
 "There is nothing here with which to fill the bottle.\n",

 /* 107 */
 "Your bottle is now full of water.\n",

 /* 108 */
 "Your bottle is now full of oil.\n",

 /* 109 */
 "You can't fill that.\n",

 /* 110 */
 "Don't be ridiculous!\n",

 /* 111 */
 "The door is extremely rusty and refuses to open.\n",

 /* 112 */
 "The plant indignantly shakes the oil off its leaves and asks, \"Water?\"\n",

 /* 113 */
 "The hinges are quite thoroughly rusted now and won't budge.\n",

 /* 114 */
 "The oil has freed up the hinges so that the door will now move,\n"
 "although it requires some effort.\n",

 /* 115 */
 "The plant has exceptionally deep roots and cannot be pulled free.\n",

 /* 116 */
 "The dwarves' knives vanish as they strike the walls of the cave.\n",

 /* 117 */
 "Something you're carrying won't fit through the tunnel with you.\n"
 "You'd best take inventory and drop something.\n",

 /* 118 */
 "You can't fit this five-foot clam through that little passage!\n",

 /* 119 */
 "You can't fit this five-foot oyster through that little passage!\n",

 /* 120 */
 "I advise you to put down the clam before opening it.  >STRAIN!<\n",

 /* 121 */
 "I advise you to put down the oyster before opening it.  >WRENCH!<\n",

 /* 122 */
 "You don't have anything strong enough to open the clam.\n",

 /* 123 */
 "You don't have anything strong enough to open the oyster.\n",

 /* 124 */
 "A glistening pearl falls out of the clam and rolls away.  Goodness,\n"
 "this must really be an oyster.  (I never was very good at identifying\n"
 "bivalves.)  Whatever it is, it has now snapped shut again.\n",

 /* 125 */
 "The oyster creaks open, revealing nothing but oyster inside.  It\n"
 "promptly snaps shut again.\n",

 /* 126 */
 "You have crawled around in some little holes and found your way\n"
 "blocked by a recent cave-in.  You are now back in the main passage.\n",

 /* 127 */
 "There are faint rustling noises from the darkness behind you.\n",

 /* 128 */
 "Out from the shadows behind you pounces a bearded pirate!  \"Har, har,\"\n"
 "he chortles, \"I'll just take all this booty and hide it away with me\n"
 "chest deep in the maze!\"  He snatches your treasure and vanishes into\n"
 "the gloom.\n",

 /* 129 */
 "A sepulchral voice reverberating through the cave, says, \"Cave closing\n"
 "soon.  All adventurers exit immediately through main office.\"\n",

 /* 130 */
 "A mysterious recorded voice groans into life and announces:\n"
 "   \"This exit is closed.  Please leave via main office.\"\n",

 /* 131 */
 "It looks as though you're dead.  Well, seeing as how it's so close to\n"
 "closing time anyway, I think we'll just call it a day.\n",

 /* 132 */
 "The sepulchral voice intones, \"The cave is now closed.\"  As the echoes\n"
 "fade, there is a blinding flash of light (and a small puff of orange\n"
 "smoke). . . .  As your eyes refocus, you look around and find...\n",

 /* 133 */
 "There is a loud explosion, and a twenty-foot hole appears in the far\n"
 "wall, burying the dwarves in the rubble.  You march through the hole\n"
 "and find yourself in the main office, where a cheering band of\n"
 "friendly elves carry the conquering adventurer off into the sunset.\n",

 /* 134 */
 "There is a loud explosion, and a twenty-foot hole appears in the far\n"
 "wall, burying the snakes in the rubble.  A river of molten lava pours\n"
 "in through the hole, destroying everything in its path, including you!\n",

 /* 135 */
 "There is a loud explosion, and you are suddenly splashed across the\n"
 "walls of the room.\n",

 /* 136 */
 "The resulting ruckus has awakened the dwarves.  There are now several\n"
 "threatening little dwarves in the room with you!  Most of them throw\n"
 "knives at you!  All of them get you!\n",

 /* 137 */
 "Oh, leave the poor unhappy bird alone.\n",

 /* 138 */
 "I daresay whatever you want is around here somewhere.\n",

 /* 139 */
 "I don't know the word \"stop\".  Use \"quit\" if you want to give up.\n",

 /* 140 */
 "You can't get there from here.\n",

 /* 141 */
 "You are being followed by a very large, tame bear.\n",

 /* 142 */
 "If you want to end your adventure early, say \"quit\".  To suspend your\n"
 "adventure such that you can continue later, say \"suspend\" (or \"pause\"\n"
 "or \"save\").  To see what hours the cave is normally open, say \"hours\".\n"
 "To see how well you're doing, say \"score\".  To get full credit for a\n"
 "treasure, you must have left it safely in the building, though you get\n"
 "partial credit just for locating it.  You lose points for getting\n"
 "killed, or for quitting, though the former costs you more.  There are\n"
 "also points based on how much (if any) of the cave you've managed to\n"
 "explore; in particular, there is a large bonus just for getting in (to\n"
 "distinguish the beginners from the rest of the pack), and there are\n"
 "other ways to determine whether you've been through some of the more\n"
 "harrowing sections.  If you think you've found all the treasures, just\n"
 "keep exploring for a while.  If nothing interesting happens, you\n"
 "haven't found them all yet.  If something interesting *DOES* happen,\n"
 "it means you're getting a bonus and have an opportunity to garner many\n"
 "more points in the Master's section.  I may occasionally offer hints\n"
 "if you seem to be having trouble.  If I do, I'll warn you in advance\n"
 "how much it will affect your score to accept the hints.  Finally, to\n"
 "save paper, you may specify \"brief\", which tells me never to repeat\n"
 "the full description of a place unless you explicitly ask me to.\n",

 /* 143 */
 "Do you indeed wish to quit now?\n",

 /* 144 */
 "There is nothing here with which to fill the vase.\n",

 /* 145 */
 "The sudden change in temperature has delicately shattered the vase.\n",

 /* 146 */
 "It is beyond your power to do that.\n",

 /* 147 */
 "I don't know how.\n",

 /* 148 */
 "It is too far up for you to reach.\n",

 /* 149 */
 "You killed a little dwarf.  The body vanishes in a cloud of greasy\n"
 "black smoke.\n",

 /* 150 */
 "The shell is very strong and is impervious to attack.\n",

 /* 151 */
 "What's the matter, can't you read?  Now you'd best start over.\n",

 /* 152 */
 "The axe bounces harmlessly off the dragon's thick scales.\n",

 /* 153 */
 "The dragon looks rather nasty.  You'd best not try to get by.\n",

 /* 154 */
 "The little bird attacks the green dragon, and in an astounding flurry\n"
 "gets burnt to a cinder.  The ashes blow away.\n",

 /* 155 */
 "On what?\n",

 /* 156 */
 "Okay, from now on I'll only describe a place in full the first time\n"
 "you come to it.  To get the full description, say \"look\".\n",

 /* 157 */
 "Trolls are close relatives with the rocks and have skin as tough as\n"
 "that of a rhinoceros.  The troll fends off your blows effortlessly.\n",

 /* 158 */
 "The troll deftly catches the axe, examines it carefully, and tosses it\n"
 "back, declaring, \"Good workmanship, but it's not valuable enough.\"\n",

 /* 159 */
 "The troll catches your treasure and scurries away out of sight.\n",

 /* 160 */
 "The troll refuses to let you cross.\n",

 /* 161 */
 "There is no longer any way across the chasm.\n",

 /* 162 */
 "Just as you reach the other side, the bridge buckles beneath the\n"
 "weight of the bear, which was still following you around.  You\n"
 "scrabble desperately for support, but as the bridge collapses you\n"
 "stumble back and fall into the chasm.\n",

 /* 163 */
 "The bear lumbers toward the troll, who lets out a startled shriek and\n"
 "scurries away.  The bear soon gives up the pursuit and wanders back.\n",

 /* 164 */
 "The axe misses and lands near the bear where you can't get at it.\n",

 /* 165 */
 "With what?  Your bare hands?  Against *HIS* bear hands??\n",

 /* 166 */
 "The bear is confused; he only wants to be your friend.\n",

 /* 167 */
 "For crying out loud, the poor thing is already dead!\n",

 /* 168 */
 "The bear eagerly wolfs down your food, after which he seems to calm\n"
 "down considerably and even becomes rather friendly.\n",

 /* 169 */
 "The bear is still chained to the wall.\n",

 /* 170 */
 "The chain is still locked.\n",

 /* 171 */
 "The chain is now unlocked.\n",

 /* 172 */
 "The chain is now locked.\n",

 /* 173 */
 "There is nothing here to which the chain can be locked.\n",

 /* 174 */
 "There is nothing here to eat.\n",

 /* 175 */
 "Do you want the hint?\n",

 /* 176 */
 "Do you need help getting out of the maze?\n",

 /* 177 */
 "You can make the passages look less alike by dropping things.\n",

 /* 178 */
 "Are you trying to explore beyond the plover room?\n",

 /* 179 */
 "There is a way to explore that region without having to worry about\n"
 "falling into a pit.  None of the objects available is immediately\n"
 "useful in discovering the secret.\n",

 /* 180 */
 "Do you need help getting out of here?\n",

 /* 181 */
 "Don't go west.\n",

 /* 182 */
 "Gluttony is not one of the troll's vices.  Avarice, however, is.\n",

 /* 183 */
 "Your lamp is getting dim.  You'd best start wrapping this up, unless\n"
 "you can find some fresh batteries.  I seem to recall there's a vending\n"
 "machine in the maze.  Bring some coins with you.\n",

 /* 184 */
 "Your lamp has run out of power.\n",

 /* 185 */
 "There's not much point in wandering around out here, and you can't\n"
 "explore the cave without a lamp.  So let's just call it a day.\n",

 /* 186 */
 "There are faint rustling noises from the darkness behind you.  As you\n"
 "turn toward them, the beam of your lamp falls across a bearded pirate.\n"
 "He is carrying a large chest.  \"Shiver me timbers!\" he cries, \"I've\n"
 "been spotted!  I'd best hie meself off to the maze to hide me chest!\"\n"
 "With that, he vanishes into the gloom.\n",

 /* 187 */
 "Your lamp is getting dim.  You'd best go back for those batteries.\n",

 /* 188 */
 "Your lamp is getting dim.  I'm taking the liberty of replacing the\n"
 "batteries.\n",

 /* 189 */
 "Your lamp is getting dim, and you're out of spare batteries.  You'd\n"
 "best start wrapping this up.\n",

 /* 190 */
 "I'm afraid the magazine is written in dwarvish.\n",

 /* 191 */
 "\"This is not the maze where the pirate leaves his treasure chest.\"\n",

 /* 192 */
 "Hmmm, this looks like a clue, which means it'll cost you 10 points to\n"
 "read it.  Should I go ahead and read it anyway?\n",

 /* 193 */
 "It says, \"There is something strange about this place, such that one\n"
 "of the words I've always known now has a new effect.\"\n",

 /* 194 */
 "It says the same thing it did before.\n",

 /* 195 */
 "I'm afraid I don't understand.\n",

 /* 196 */
 "\"Congratulations on bringing light into the dark-room!\"\n",

 /* 197 */
 "You strike the mirror a resounding blow, whereupon it shatters into a\n"
 "myriad tiny fragments.\n",

 /* 198 */
 "You have taken the vase and hurled it delicately to the ground.\n",

 /* 199 */
 "You prod the nearest dwarf, who wakes up grumpily, takes one look at\n"
 "you, curses, and grabs for his axe.\n",

 /* 200 */
 "Is this acceptable?\n",

 /* 201 */
 "There's no point in suspending a demonstration game.\n"
};
