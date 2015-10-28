This project began as an attempt to run some form of the original 350 point
Colossal Cave Adventure on my MacBook.  The best candidate I could find for
this purpose was [the BDS C port][1]; however, even after getting it to compile
(Whoever edited it last seemed to believe that QNX was the only system with
decent header files), there were still a number of problems with it.  The hints
system was gone; the score rankings (e.g., "Experienced Adventurer") were gone
in lieu of listing scores for each section of gameplay (treasures, survival,
and the like); the odds of successfully killing a dwarf were 33% instead of 2
out of 3; dwarves jumped around the rooms at random instead of moving between
adjacent rooms; dwarves got a chance to attack you every turn, even if you were
just picking up the axe in the middle of battle; "`WATER PLANT`" was considered
bad grammar; and so on.  Fixing these bugs became more & more difficult and/or
time-consuming, and I eventually gave up and decided simply to translate [the
original PDP-10 Fortran][2] directly into something modern that worked.  Some
time later, here we are.

I have tried to make this translation be as true to the original as possible
(in end behavior, not internal logic, at least).  The biggest changes are that
there is now a "`> `" prompt for input, there is no longer a space before each
output line, and, of course, the letters aren't all uppercase.  Games are saved
to ordinary files (`~/.adventure` by default, though you can supply a different
filename to the "`save`" command) and can be resumed later either with the
"`resume`" (or "`restore`" or "`restart`" or "`load`") command during play or
by specifying the name of the save file on the command line.  Additionally, as
the time-sharing mechanics (delays before restarting a saved game, only
allowing games during specific hours, and being able to bypass or reconfigure
this behavior through maintenance/wizard mode) are unnecessary and likely to
get in the way on modern computers, their presence is configurable, defaulting
to "off."  If you opt to use these features (or even if you don't), a program
named 'frawd' is included for passing one of the steps in wizard
authentication; for passing the other steps, see wizard.md.

Anyway, enjoy!

[1]: http://www.ifarchive.org/if-archive/games/source/advqnx.tar.gz
[2]: http://www.ifarchive.org/if-archive/games/source/advent-original.tar.gz
