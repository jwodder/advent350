This project began as an attempt to run some form of the original 350 point
Colossal Cave Adventure on my MacBook.  The best candidate I could find for
this purpose was the BDS C port[1]; however, even after getting it to compile
(Whoever edited it last seemed to believe that QNX was the only system with
decent header files), there were still a number of problems with it.  The hints
system was gone; the score rankings (e.g., "Experienced Adventurer") were gone
in lieu of listing scores for each section of gameplay (treasures, survival,
and the like); the odds of successfully killing a dwarf were 33% instead of 2
out of 3; dwarves jumped around the rooms at random instead of moving between
adjacent rooms; dwarves got a chance to attack you every turn, even if you were
just picking up the axe in the middle of battle; "WATER PLANT" was considered
bad grammar; and so on.  Fixing these bugs became more & more difficult and/or
time-consuming, and I eventually gave up and decided simply to translate the
original PDP-10 Fortran[2] directly into something that worked.  I had recently
begun learning Perl 6, a developing language which lacks examples of real
working code, and it seemed like as good a choice as any.  Some time later,
here we are.

I have tried to make this translation be as accurate to the original as
possible (in end behavior, not internal logic, that is).  The biggest change is
that there is now a "> " prompt for input (and, of course, the letters aren't
all uppercase).  Games are saved to ordinary files (~/.adventure by default,
though you can supply a filename to the "save" command) and can be resumed
either with the "resume" (or "restore" or "restart" or "load") command during
play or by specifying the name of the save file on the command line.
Additionally, as the time-sharing mechanics are unnecessary and likely to get
in the way on modern computers, there are two different versions of the
program: one with all of the "magic" features of the original (delays before
restarting a saved game, only allowing games during specific hours, and being
able to bypass or configure this behavior through a maintenance/wizard mode)
and one without.

The current plan is for the "master" branch to contain the "ideal" Perl 6
translation of Adventure which works with the language as specified (to the
best of my knowledge of Perl 6, at least), while the "rakudo" branch contains a
simplified form that can run on Rakudo.  Additionally, the "c-adv" branch shall
contain a C translation of the Perl 6 code for the purpose of at least
producing something immediately usable & portable.

Anyway, enjoy!


[1] <http://www.ifarchive.org/if-archive/games/source/advqnx.tar.gz>
[2] <http://www.ifarchive.org/if-archive/games/source/advent-original.tar.gz>
