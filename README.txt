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
original PDP-10 Fortran[2] directly into something modern that worked.  I had
recently begun learning Perl 6, a developing language in need of examples of
real working code, and it seemed like as good a choice as any.  Some time
later, here we are.

I have tried to make this translation be as true to the original as possible
(in end behavior, not internal logic, at least).  The biggest changes are that
there is now a "> " prompt for input, there is no longer a space before each
output line, and, of course, the letters aren't all uppercase.  Games are saved
to ordinary files (~/.adventure by default, though you can supply a different
filename to the "save" command) and can be resumed later either with the
"resume" (or "restore" or "restart" or "load") command during play or by
specifying the name of the save file on the command line.  Additionally, as the
time-sharing mechanics are unnecessary and likely to get in the way on modern
computers, two different versions of the program are available: one with all of
the "magic" features of the original (delays before restarting a saved game,
only allowing games during specific hours, and being able to bypass or
reconfigure this behavior through maintenance/wizard mode) and one without.

Currently, the "master" branch contains the "ideal" Perl 6 translation of
Adventure which works with the language as specified (to the best of my
knowledge & understanding, at least), while the "rakudo" branch contains a
simplified form that should run on the latest version of Rakudo.  Additionally,
the "c-adv" branch contains a C99 translation of the Perl 6 code so that I can
at least produce something immediately usable & portable.

Anyway, enjoy!


[1] <http://www.ifarchive.org/if-archive/games/source/advqnx.tar.gz>
[2] <http://www.ifarchive.org/if-archive/games/source/advent-original.tar.gz>
