#!/usr/bin/python
from   __future__  import print_function, unicode_literals
import argparse
from   collections import defaultdict, namedtuple
from   datetime    import datetime
from   errno       import ENOENT
from   itertools   import dropwhile, groupby
import os.path
import pickle
import shlex
import sys
import traceback
from   enum        import IntEnum
from   six.moves   import input, range

# Configuration:
DEFAULT_MAGICFILE = os.path.expanduser('~/.advmagic')
DEFAULT_SAVEFILE = os.path.expanduser('~/.adventure')
DEFAULT_DATAFILE = 'advent.dat'

MAXDIE = 3
CHLOC  = 114
CHLOC2 = 140
TOTING = -1  # location number for the player's inventory
FIXED  = -1  # "fixed" location for an immovable object found in only one place

class Limits(object):
    OBJECTS   =  64  # advent.for: 100
    LOCATIONS = 140  # advent.for: LOCSIZ/150/
    RTEXT     = 201  # advent.for: RTXSIZ/205/
    HINTS     =   9  # advent.for: HNTSIZ/20/
    MTEXT     =  32  # advent.for: MAGSIZ/35/
    ACTSPK    =  31  # advent.for: VRBSIZ/35/

Movement = IntEnum('Movement', '''
    forced HILL ENTER UPSTREAM DOWNSTREAM FOREST CONTINUE BACK VALLEY STAIRS
    EXIT BUILDING GULLY STREAM ROCK BED CRAWL COBBLE IN SURFACE NULL DARK
    PASSAGE LOW CANYON AWKWARD GIANT VIEW UP DOWN PIT OUTDOORS CRACK STEPS DOME
    LEFT RIGHT HALL JUMP BARREN OVER ACROSS EAST WEST NORTH SOUTH NE SE SW NW
    DEBRIS HOLE WALL BROKEN Y2 CLIMB LOOK FLOOR ROOM SLIT SLAB XYZZY DEPRESSION
    ENTRANCE PLUGH SECRET CAVE null68 CROSS BEDQUILT PLOVER ORIENTAL CAVERN
    SHELL RESERVOIR MAIN FORK
''')

Item = IntEnum('Item', '''
    KEYS LAMP GRATE CAGE ROD ROD2 STEPS BIRD DOOR PILLOW SNAKE FISSUR TABLET
    CLAM OYSTER MAGZIN DWARF KNIFE FOOD BOTTLE WATER OIL MIRROR PLANT PLANT2
    STALACTITE SHADOW AXE DRAWING PIRATE DRAGON CHASM TROLL TROLL2 BEAR MESSAG
    VOLCANO VEND BATTER CARPET null41 null42 null43 null44 null45 null46 null47
    null48 null49 NUGGET DIAMONDS SILVER JEWELRY COINS CHEST EGGS TRIDENT VASE
    EMERALD PYRAM PEARL RUG SPICES CHAIN
''')

Action = IntEnum('Action', '''
    TAKE DROP SAY OPEN NOTHING LOCK ON OFF WAVE CALM WALK KILL POUR EAT DRINK
    RUB THROW QUIT FIND INVENT FEED FILL BLAST SCORE FOO BRIEF READ BREAK WAKE
    SUSPEND HOURS RESUME
''')

class MsgWord(IntEnum):
    FEE = 1
    FIE = 2
    FOE = 3
    FOO = 4
    FUM = 5
    HOCUS = 50
    HELP = 51
    TREE = 64
    DIG = 66
    LOST = 68
    MIST = 69
    FUCK = 79
    STOP = 139
    INFO = 142
    SWIM = 147

class Cond(IntEnum):
    LIGHT = 0
    OIL = 1
    LIQUID = 2
    NO_PIRATE = 3

def indexLines(lines, qty):
    data = [None] * (qty+1)
    for i, block in groupby(lines, lambda s: int(s.partition('\t')[0])):
        data[i] = ''.join(s.partition('\t')[2] for s in block)
        if '>$<' in data[i]:
            data[i] = None
    return data

def intTSV(line):
    return list(map(int, line.split('\t')))

def nonemap(f, xs):
    return [f(x) if x is not None else None for x in xs]

sectno = 0
def bysection(line):
    global sectno
    if line.strip() != '-1':
        try:
            sectno = int(line)
        except ValueError:
            pass
    return sectno

def liq2(p):
    return (Item.WATER, 0, Item.OIL)[p]

def ran(n):
    d = 1
    if ran.r == 0:
        (d, ran.r) = datime()
        ran.r = 18 * ran.r + 5
        d = 1000 + d % 1000
    for _ in range(d):
        ran.r = (ran.r * 1021) % 1048576
    return (n * ran.r) // 1048576
ran.r = 0

def pct(x):
    return ran(100) < x

def load(fp, ofType):
    x = pickle.load(fp)
    if not isinstance(x, ofType):
        raise TypeError('Expected %s object in %r; got %s instead'
                        % (ofType.__name__, fp.name, x.__class__.__name__))
    return x

class Travel(namedtuple('Travel', 'dest verbs verb1 uncond chance nodwarf'
                                  ' carry here obj notprop forced')):
    __slots__ = ()

    @classmethod
    def fromEntry(cls, line):
        line = intTSV(line)
        (M,N) = divmod(line[0], 1000)
        return cls(dest = N,
                   verbs = set(line[1:]),
                   verb1 = line[1],
                   uncond = M == 0 or M == 100,
                   chance = M if 0 < M < 100 else None,
                   nodwarf = M == 100,
                   carry = M - 100 if 100 < M <= 200 else None,
                   here = M - 200 if 200 < M <= 300 else None,
                   obj = M % 100 if 300 < M else None,
                   notprop = M // 100 - 3 if 300 < M else None,
                   forced = line[1] == 1)

Hint = namedtuple('Hint', 'turns points question hint')

class Adventure(object):
    def __init__(self, advdat):
        sections = {
            i: list(sect)[1:-1] for i, sect in groupby(advdat, bysection)
        }
        self.longDesc = indexLines(sections[1], Limits.LOCATIONS)
        self.shortDesc = indexLines(sections[2], Limits.LOCATIONS)
        self.travel = nonemap(lambda s: list(map(Travel.fromEntry,
                                                 s.splitlines())),
                              indexLines(sections[3], Limits.LOCATIONS))
        self.vocabulary = defaultdict(list)
        for entry in sections[4]:
            i, word = entry.strip().split('\t')[:2]
            i = int(i)
            iconst = ([Movement, Item, Action, MsgWord][i // 1000])(i % 1000)
            self.vocabulary[word].append(iconst)
        self.itemDesc = [None] * (Limits.OBJECTS + 1)
        obj = 0
        for line in sections[5]:
            num, _, txt = line.partition('\t')
            num = int(num)
            if '>$<' in txt:
                txt = None
            if 1 <= num < 100:
                obj = num
                self.itemDesc[obj] = [txt]
            else:
                state = num // 100
                try:
                    self.itemDesc[obj][state+1] += txt
                except IndexError:
                    self.itemDesc[obj].append(txt)
        self.rmsg = indexLines(sections[6], Limits.RTEXT)
        self.startplace = [0] * (Limits.OBJECTS + 1)
        self.startfixed = [0] * (Limits.OBJECTS + 1)
        for locs in map(intTSV, sections[7]):
            self.startplace[locs[0]] = locs[1]
            try:
                self.startfixed[locs[0]] = locs[2]
            except IndexError:
                self.startfixed[locs[0]] = 0
        self.actspk = indexLines(sections[8], Limits.ACTSPK)
        self.cond = [0] * (Limits.LOCATIONS + 1)
        for cs in map(intTSV, sections[9]):
            for loc in cs[1:]:
                self.cond[loc] |= 1 << cs[0]
        self.classes = []
        for line in sections[10]:
            threshold, _, msg = line.partition('\t')
            self.classes.append((int(threshold), msg))
        self.hints = nonemap(lambda s: Hint(*intTSV(s)),
                             indexLines(sections[11], Limits.HINTS))
        self.magic = indexLines(sections[12], Limits.MTEXT)

    def liqloc(self, loc):
        return liq2(self.bitset(loc, Cond.OIL) if self.bitset(loc, Cond.LIQUID)
                                               else 1)

    def bitset(self, loc, n):
        return self.cond[loc] & (1 << n)

    def forced(self, loc):
        return loc > 0 and self.travel[loc][0].forced

    def vocab(self, word, wordtype=None):
        matches = self.vocabulary[word]
        if wordtype is not None:
            matches = [i for i in matches if isinstance(i, wordtype)]
        if not matches:
            if wordtype is not None:
                bug(5)
            return None
        return matches[0]


class Game(object):
    def __init__(self):
        self.loc = 0
        self.newloc = 1
        self.oldloc = 0
        self.oldloc2 = 0
        self.limit = 330
        self.turns = 0
        self.iwest = 0
        self.knifeloc = 0
        self.detail = 0
        self.numdie = 0
        self.holding = 0
        self.foobar = 0
        self.tally = 15
        self.tally2 = 0
        self.abbnum = 5
        self.clock1 = 30
        self.clock2 = 50
        self.wzdark = False
        self.closing = False
        self.lmwarn = False
        self.panic = False
        self.closed = False
        self.prop = [0] * 50 + [-1] * (Limits.OBJECTS - 49)
        self.abb = [0] * (Limits.LOCATIONS + 1)
        self.hintlc = [0] * (Limits.HINTS + 1)
        self.hinted = [False] * (Limits.HINTS + 1)
        self.dloc = [19, 27, 33, 44, 64, CHLOC]
        self.odloc = [0] * 6
        self.dseen = [False] * 6
        self.dflag = 0
        self.dkill = 0
        self.place = cave.startplace[:]
        self.fixed = cave.startfixed[:]
        self.saved = -1
        self.savet = 0
        self.gaveup = False
        self.atloc = [[]]
        for i in range(1, Limits.LOCATIONS+1):
            here = [k for k in range(1, Limits.OBJECTS+1)
                      if self.place[k] == i and self.fixed[k] <= 0]
            for k in range(1, Limits.OBJECTS+1):
                if self.place[k] == i and self.fixed[k] > 0:
                    here.append(k)
                elif self.fixed[k] == i:
                    here.append(k+100)
            self.atloc.append(here)

    def toting(self, item):
        return self.place[item] == TOTING

    def here(self, item):
        return self.place[item] == self.loc or self.toting(item)

    def at(self, item):
        return self.loc in (self.place[item], self.fixed[item])

    def liq(self):
        return liq2(max(self.prop[Item.BOTTLE], -1-self.prop[Item.BOTTLE]))

    def dark(self):
        return not (cave.bitset(self.loc, Cond.LIGHT) or
                    (self.prop[Item.LAMP] and self.here(Item.LAMP)))

    def carry(self, obj, where):
        if obj <= 100:
            if self.place[obj] == TOTING:
                return
            self.place[obj] = TOTING
            self.holding += 1
        self.atloc[where].remove(obj)

    def drop(self, obj, where):
        if obj > 100:
            self.fixed[obj-100] = where
        else:
            if self.place[obj] == TOTING:
                self.holding -= 1
            self.place[obj] = where
        if where > 0:
            self.atloc[where].insert(0, obj)

    def move(self, obj, where):
        whence = self.fixed[obj-100] if obj > 100 else self.place[obj]
        if 0 < whence <= 300:
            self.carry(obj, whence)
        self.drop(obj, where)

    def put(self, obj, where, pval):
        self.move(obj, where)
        return -1 - pval

    def destroy(self, obj):
        self.move(obj, 0)

    def juggle(self, obj):
        self.move(obj, self.place[obj])
        self.move(obj+100, self.fixed[obj])

    def score(self, scoring, bonus=0):
        score = 0
        for i in range(50, 65):
            if self.prop[i] >= 0:
                score += 2
            if self.place[i] == 3 and self.prop[i] == 0:
                score += 12 if i == Item.CHEST else 14 if i > Item.CHEST else 10
        score += (MAXDIE - self.numdie) * 10
        if not (scoring or self.gaveup):
            score += 4
        if self.dflag != 0:
            score += 25
        if self.closing:
            score += 25
        if self.closed:
            score += {0: 10, 133: 45, 134: 30, 135: 25}.get(bonus, 0)
        if self.place[Item.MAGZIN] == 108:
            score += 1
        score += 2
        for i in range(1, 10):
            if self.hinted[i]:
                score -= cave.hints[i].points
        return score


class Magic(object):
    on = True
    magicfile = DEFAULT_MAGICFILE

    def __init__(self):
        # These arrays hold the times when adventurers are allowed into
        # Colossal Cave; `self.wkday` is for weekdays, `self.wkend` for
        # weekends, and `self.holid` for holidays (days with special hours).
        # If element `n` of an array is true, then the hour `n:00` through
        # `n:59` is considered "prime time," i.e., the cave is closed then.
        self.wkday = [False] * 8 + [True] * 10 + [False] * 6
        self.wkend = [False] * 24
        self.holid = [False] * 24
        self.hbegin = 0       # start of next holiday
        self.hend = -1        # end of next holiday
        self.short = 30       # turns allowed in a short/demonstration game
        self.magnm = 11111    # magic number
        self.latency = 90     # time required to wait after saving
        self.magic = 'DWARF'  # magic word
        self.hname = ''       # name of next holiday
        self.msg = ''         # MOTD, initially null

    def mspeak(self, msg, blklin=True):
        if msg != 0:
            speak(cave.magic[msg], blklin=blklin)

    def yesm(self, x, y, z, blklin=True):
        return yesx(x, y, z, self.mspeak, blklin=blklin)

    def start(self):
        (d,t) = datime()
        if game.saved != -1:
            delay = (d - game.saved) * 1440 + (t - game.savet)
            if delay < self.latency:
                print('This adventure was suspended a mere', delay,
                      'minutes ago.')
                if delay < self.latency // 3:
                    self.mspeak(2)
                else:
                    self.mspeak(8)
                    if self.wizard():
                        game.saved = -1
                        return False
                    self.mspeak(9)
                sys.exit()
        if (self.holid if self.hbegin <= d <= self.hend
                       else self.wkend if d % 7 <= 1
                       else self.wkday)[t // 60]:
            # Prime time (cave closed)
            self.mspeak(3)
            self.hours()
            self.mspeak(4)
            if self.wizard():
                game.saved = -1
                return False
            if game.saved != -1:
                self.mspeak(9)
                sys.exit()
            if self.yesm(5, 7, 7):
                game.saved = -1
                return True
            sys.exit()
        game.saved = -1
        return False

    def maint(self):
        if not self.wizard():
            return
        if self.yesm(10, 0, 0, blklin=False):
            self.hours()
        if self.yesm(11, 0, 0, blklin=False):
            self.newhrs()
        if self.yesm(26, 0, 0, blklin=False):
            self.mspeak(27, blklin=False)
            self.hbegin = getInt()
            self.mspeak(28, blklin=False)
            self.hend = getInt()
            (d,_) = datime()
            self.hbegin += d
            self.hend += self.hbegin - 1
            self.mspeak(29, blklin=False)
            self.hname = input('> ')[:20]
        print('Length of short game (null to leave at %d):' % (self.short,))
        x = getInt()
        if x > 0:
            self.short = x
        self.mspeak(12, blklin=False)
        word = getin().word1
        if word is not None:
            self.magic = word
        self.mspeak(13, blklin=False)
        x = getInt()
        if x > 0:
            self.magnm = x
        print('Latency for restart (null to leave at %d):' % (self.latency,))
        x = getInt()
        if 0 < x < 45:
            self.mspeak(30, blklin=False)
        if x > 0:
            self.latency = max(45, x)
        if self.yesm(14, 0, 0):
            self.motd(True)
        self.mspeak(15, blklin=False)
        with open(self.magicfile, 'wb') as fp:
            pickle.dump(fp, magic)
        self.ciao()

    def wizard(self):
        if not self.yesm(16, 0, 7):
            return False
        self.mspeak(17)
        word = getin().word1
        if word != self.magic:
            self.mspeak(20)
            return False
        (d,t) = datime()
        t = t*2 + 1
        wchrs = [64] * 5
        val = []
        for y in range(5):
            x = 79 + d % 5
            d //= 5
            for _ in range(x):
                t = (t * 1027) % 1048576
            val.append((t*26) // 1048576 + 1)
            wchrs[y] += val[-1]
        if self.yesm(18, 0, 0):
            self.mspeak(20)
            return False
        print('\n' + ''.join(map(chr, wchrs)))
        wchrs = list(map(ord, getin().word1))
        ### What happens if the inputted word is less than five characters?
        (d,t) = datime()
        t = (t // 60) * 40 + (t // 10) * 10
        d = self.magnm
        for y in range(5):
            wchrs[y] -= (abs(val[y] - val[(y+1)%5]) * (d%10) + t%10) % 26 + 1
            t //= 10
            d //= 10
        if all(c == 64 for c in wchrs):
            self.mspeak(19)
            return True
        else:
            self.mspeak(20)
            return False

    def hours(self):
        print()
        self.hoursx(self.wkday, 'Mon - Fri:')
        self.hoursx(self.wkend, 'Sat - Sun:')
        self.hoursx(self.holid, 'Holidays: ')
        (d,_) = datime()
        if self.hend < d or self.hend < self.hbegin:
            return
        if self.hbegin > d:
            d = self.hbegin - d
            print('\nThe next holiday will be in %d day%s, namely %s.'
                  % (d, '' if d == 1 else 's', self.hname))
        else:
            print('\nToday is a holiday, namely ' + self.hname + '.')

    def hoursx(self, horae, day):
        if not any(horae):
            print(' ' * 10, day, '  Open all day', sep='')
        else:
            first = True
            fromH = 0
            while True:
                while fromH < 24 and horae[fromH]:
                    fromH += 1
                if fromH >= 24:
                    if first:
                        print(' ' * 10, day, '  Closed all day', sep='')
                    break
                else:
                    till = fromH + 1
                    while till < 24 and not horae[till]:
                        till += 1
                    if first:
                        print(' ' * 10, day, '%4d:00 to%3d:00' % (fromH, till),
                              sep='')
                    else:
                        print(' ' * 20, '%4d:00 to%3d:00' % (fromH, till),
                              sep='')
                first = False
                fromH = till

    def newhrs(self):
        self.mspeak(21)
        self.wkday = self.newhrx('weekdays:')
        self.wkend = self.newhrx('weekends:')
        self.holid = self.newhrx('holidays:')
        self.mspeak(22)
        self.hours()

    def newhrx(self, day):
        horae = [False] * 24
        print('Prime time on', day)
        while True:
            fromH = getInt('from: ')
            if not (0 <= fromH < 24):
                return horae
            till = getInt('till: ')
            if not (fromH <= till-1 < 24):
                return horae
            horae[fromH:till] = [True] * (till - fromH)

    def motd(self, alter):
        if alter:
            self.mspeak(23)
            self.msg = ''
            # This doesn't exactly match the logic used in the original Fortran,
            # but it's close:
            while len(self.msg) < 430:
                nextline = input('> ')
                if not nextline:
                    return
                if len(nextline) > 70:
                    self.mspeak(24)
                    continue
                self.msg += nextline + '\n'
            self.mspeak(25)
        elif self.msg:
            print(self.msg, end='')

    def ciao(self):
        self.mspeak(32)
        sys.exit()


class NoMagic(object):
    on = False

    def __getattr__(self, _):
        return lambda *p, **a: None


class InputLine(namedtuple('InputLine', 'word1 in1 word2 in2 raw')):
    __slots__ = ()

    def moveup(self):
        return self._replace(word1=self.word2, in1=self.in2,
                             word2=None, in2=None)


demo = False
verb = None
obj = None
lastline = InputLine(None, None, None, None, None)

cave = None
game = None
magic = None

def speak(s, blklin=True):
    if s:
        if blklin:
            print()
        print(s, end='')

def pspeak(item, state, blklin=True):
    speak(cave.itemDesc[item][state+1], blklin=blklin)

def rspeak(msg, blklin=True):
    if msg != 0:
        speak(cave.rmsg[msg], blklin=blklin)

def getin(blklin=True):
    if blklin:
        print()
    while True:
        raw = input('> ')
        inp = (raw.split() + [None]*2)[:2]
        if inp[0] is None and blklin:
            continue
        words = nonemap(lambda s: s[:5].upper(), inp)
        return InputLine(word1=words[0],
                         word2=words[1],
                         in1=inp[0],
                         in2=inp[1],
                         raw=raw)

def getInt(prompt='> '):
    while True:
        raw = input(prompt)
        try:
            return int(raw)
        except ValueError:
            pass

def yes(x, y, z):
    return yesx(x, y, z, rspeak)

def yesx(x, y, z, spk, blklin=True):
    while True:
        spk(x, blklin=blklin)
        reply = getin(blklin=blklin).word1
        if reply in ('YES', 'Y'):
            spk(y, blklin=blklin)
            return True
        elif reply in ('NO', 'N'):
            spk(z, blklin=blklin)
            return False
        else:
            print('\nPlease answer the question.')

def bug(num):
    print('Fatal error, see source code for interpretation.')
    # Given the above message, I suppose I should list the possible bug numbers
    # in the source somewhere, and right here is as good a place as any:
    # 5 - Required vocabulary word not found
    # 20 - Special travel number (500>L>300) is outside of defined range
    # 22 - Vocabulary type (N/1000) not between 0 and 3
    # 23 - Intransitive action verb not defined
    # 24 - Transitive action verb not defined
    # 26 - Location has no travel entries
    print('Probable cause: erroneous info in database.')
    print('Error code =', num)
    print()
    sys.exit(-1)

def domove(motion):
    # Label 8
    game.newloc = game.loc
    if not cave.travel[game.loc]:
        bug(26)
    if motion == Movement.NULL:
        return label2
    elif motion == Movement.BACK:
        k = game.oldloc2 if cave.forced(game.oldloc) else game.oldloc
        (game.oldloc2, game.oldloc) = (game.oldloc, game.loc)
        if k == game.loc:
            rspeak(91)
        else:
            k2 = 0
            for kk, trav in enumerate(cave.travel[game.loc]):
                ll = trav.dest
                if ll == k:
                    dotrav(trav.verb1)
                    return label2
                elif ll <= 300 and cave.forced(ll) and \
                    cave.travel[ll][0].dest == k:
                    k2 = kk
            if k2 != 0:
                dotrav(cave.travel[game.loc][k2].verb1)
            else:
                rspeak(140)
    elif motion == Movement.LOOK:
        game.detail += 1
        if game.detail < 4:
            rspeak(15)
        game.wzdark = False
        game.abb[game.loc] = 0
    elif motion == Movement.CAVE:
        rspeak(57 if game.loc < 8 else 58)
    else:
        (game.oldloc2, game.oldloc) = (game.oldloc, game.loc)
        dotrav(motion)
    return label2

def dotrav(motion):
    # Label 9
    for trav in dropwhile(lambda t: not t.forced and motion not in t.verbs,
                          cave.travel[game.loc]):
        if trav.uncond or\
                trav.chance and pct(trav.chance) or\
                trav.carry and game.toting(trav.carry) or\
                trav.here and (game.toting(trav.here) or game.at(trav.here)) or\
                trav.obj and game.prop[trav.obj] != trav.notprop:
            rdest = trav.dest
            break
    else:
        if motion in {29, 30, 43, 44, 45, 46, 47, 48, 49, 50}:
            rspeak(9)
        elif motion in {7, 36, 37}:
            rspeak(10)
        elif motion in {11, 19}:
            rspeak(11)
        elif motion in {62, 65}:
            rspeak(42)
        elif motion == 17:
            rspeak(80)
        elif verb in {Action.FIND, Action.INVENT}:
            rspeak(59)
        else:
            rspeak(12)
        return
    if 0 <= rdest <= 300:
        game.newloc = rdest
    elif rdest == 301:
        if not game.holding or game.holding == 1 and game.toting(Item.EMERALD):
            game.newloc = 99 + 100 - game.loc
        else:
            game.newloc = game.loc
            rspeak(117)
    elif rdest == 302:
        game.drop(Item.EMERALD, game.loc)
        game.newloc = 100 if game.loc == 33 else 33
    elif rdest == 303:
        if game.prop[Item.TROLL] == 1:
            pspeak(Item.TROLL, 1)
            game.prop[Item.TROLL] = 0
            game.move(Item.TROLL2, 0)
            game.move(Item.TROLL2+100, 0)
            game.move(Item.TROLL, 117)
            game.move(Item.TROLL+100, 122)
            game.juggle(Item.CHASM)
            game.newloc = game.loc
        else:
            game.newloc = 122 if game.loc == 117 else 117
            if game.prop[Item.TROLL] == 0:
                game.prop[Item.TROLL] = 1
            if game.toting(Item.BEAR):
                rspeak(162)
                game.prop[Item.CHASM] = 1
                game.prop[Item.TROLL] = 2
                game.drop(Item.BEAR, game.newloc)
                game.fixed[Item.BEAR] = FIXED
                game.prop[Item.BEAR] = 3
                if game.prop[Item.SPICES] < 0:
                    game.tally2 += 1
                game.oldloc2 = game.newloc
                death()
    elif 500 < rdest:
        rspeak(rdest - 500)
    else:
        bug(20)

def death():
    # Label 99
    if game.closing:
        rspeak(131)
        game.numdie += 1
        normend()
    else:
        yea = yes(81 + game.numdie * 2, 82 + game.numdie * 2, 54)
        game.numdie += 1
        if game.numdie == MAXDIE or not yea:
            normend()
        game.place[Item.WATER] = 0
        game.place[Item.OIL] = 0
        if game.toting(Item.LAMP):
            game.prop[Item.LAMP] = 0
        for i in range(Limits.OBJECTS, 0, -1):
            if game.toting(i):
                game.drop(i, 1 if i == Item.LAMP else game.oldloc2)
        game.loc = game.oldloc = 3
        return label2000

def normend(bonus=0):
    score = game.score(False, bonus)
    print('\n\n\nYou scored %d out of a possible 350 using %d turns.'
          % (score, game.turns))
    ranks = [cls for cls in cave.classes if cls[0] >= score]
    if ranks:
        speak(ranks[0][1])
        if len(ranks) > 1:
            diff = ranks[0][0] - score + 1
            print('\nTo achieve the next higher rating, you need %d more'
                  ' point%s.\n' % (diff, '' if diff == 1 else 's'))
        else:
            print('\nTo achieve the next higher rating would be a neat trick!')
            print('\nCongratulations!!\n')
    else:
        print('\nYou just went off my scale!!\n')
    sys.exit()

def doaction():
    global lastline
    # Label 5010
    if lastline.word2:
        # Label 2800
        lastline = lastline.moveup()
        return label2610
    elif verb:
        return transitive()
    else:
        print('\nWhat do you want to do with the ' + lastline.in1 + '?')
        return label2600

def datime():
    """
    Returns a tuple of the number of days since 1977 Jan 1 and the number
    of minutes past midnight
    """
    delta = datetime.today() - datetime(1977, 1, 1)
    return (delta.days, delta.seconds // 60)

def poof(on, mfile):
    if on:
        if mfile is None:
            try:
                mfile = open(Magic.magicfile, 'rb')
            except IOError as e:
                if e.errno == ENOENT:
                    return Magic()
                else:
                    raise
        else:
            Magic.magicfile = mfile.name
        with mfile:
            return load(mfile, Magic)
    else:
        return NoMagic()

def main():
    global cave, game, magic, demo, ran
    parser = argparse.ArgumentParser()
    parser.add_argument('-D', '--data-file', type=argparse.FileType('r'),
                        default=DEFAULT_DATAFILE)
    parser.add_argument('-m', '--magic', action='store_true')
    parser.add_argument('-M', '--magic-file', type=argparse.FileType('rb'))
    parser.add_argument('-R', '--orig-rng', action='store_true')
    parser.add_argument('savedgame', type=argparse.FileType('rb'), nargs='?')
    args = parser.parse_args()
    ### What should happen if --magic-file is used without --magic?
    goto = label2
    if args.orig_rng:
        ran(1)
    else:
        from random import randrange
        ran = randrange
    with args.data_file:
        cave = Adventure(args.data_file)
    game = Game()
    magic = poof(args.magic, args.magic_file)
    if args.savedgame is not None:
        with args.savedgame:
            goto = resume(args.savedgame.name, args.savedgame)
    else:
        demo = magic.start()
        magic.motd(False)
        game.hinted[3] = yes(65, 1, 0)
        if game.hinted[3]:
            game.limit = 1000
    while True:
        goto = goto()

def label2():
    if 0 < game.newloc < 9 and game.closing:
        rspeak(130)
        game.newloc = game.loc
        if not game.panic:
            game.clock2 = 15
        game.panic = True
    if game.newloc != game.loc and \
            not cave.forced(game.loc) and \
            not cave.bitset(game.loc, Cond.NO_PIRATE) and \
            any(game.odloc[i] == game.newloc and game.dseen[i]
                for i in range(5)):
        game.newloc = game.loc
        rspeak(2)
    game.loc = game.newloc
    # Dwarven logic:
    if game.loc == 0 or cave.forced(game.loc) or \
            cave.bitset(game.newloc, Cond.NO_PIRATE):
        return label2000
    if game.dflag == 0:
        if game.loc >= 15:
            game.dflag = 1
        return label2000
    elif game.dflag == 1:
        if game.loc < 15 or pct(95):
            return label2000
        game.dflag = 2
        if pct(50):
            game.dloc[ran(5)] = 0
        # Yes, this is supposed to be done twice.
        if pct(50):
            game.dloc[ran(5)] = 0
        for i in range(5):
            if game.dloc[i] == game.loc:
                game.dloc[i] = 18
            game.odloc[i] = game.dloc[i]
        rspeak(3)
        game.drop(Item.AXE, game.loc)
        return label2000
    dtotal, attack, stick = 0, 0, 0
    for i in range(6):  # The individual dwarven movement loop
        if game.dloc[i] == 0:
            continue
        kk = 0
        tk = game.odloc[i]
        for t in cave.travel[game.dloc[i]]:
            if not t.nodwarf:
                newloc = t.dest
                if 15 <= newloc <= 300 and \
                        newloc not in (game.odloc[i], game.dloc[i]) and \
                        not cave.forced(newloc) and \
                        not (i == 5 and cave.bitset(newloc, Cond.NO_PIRATE)):
                    kk += 1
                    if ran(kk) == 0:
                        tk = newloc
        game.odloc[i] = game.dloc[i]
        game.dloc[i] = tk
        game.dseen[i] = (game.dseen[i] and game.loc >= 15) or \
            game.loc in (game.dloc[i], game.odloc[i])
        if game.dseen[i]:
            game.dloc[i] = game.loc
            if i == 5:
                # Pirate logic:
                if game.loc == CHLOC or game.prop[Item.CHEST] >= 0:
                    continue
                k = False
                stole = False
                for j in range(50, 65):
                    if j == Item.PYRAM and game.loc in (100, 101):
                        continue
                    if game.toting(j):
                        rspeak(128)
                        if game.place[Item.MESSAG] == 0:
                            game.move(Item.CHEST, CHLOC)
                        game.move(Item.MESSAG, CHLOC2)
                        for j2 in range(50, 65):
                            if j2 == Item.PYRAM and game.loc in (100, 101):
                                continue
                            if game.at(j2) and game.fixed[j2] == 0:
                                game.carry(j2, game.loc)
                            if game.toting(j2):
                                game.drop(j2, CHLOC)
                        game.dloc[5] = game.odloc[5] = CHLOC
                        game.dseen[5] = False
                        stole = True
                        break
                    if game.here(j):
                        k = True
                if not stole:
                    if game.tally == game.tally2 + 1 and \
                            not k and \
                            game.place[Item.CHEST] == 0 and \
                            game.here(Item.LAMP) and \
                            game.prop[Item.LAMP] == 1:
                        rspeak(186)
                        game.move(Item.CHEST, CHLOC)
                        game.move(Item.MESSAG, CHLOC2)
                        game.dloc[5] = game.odloc[5] = CHLOC
                        game.dseen[5] = False
                    elif game.odloc[5] != game.dloc[5] and pct(20):
                        rspeak(127)
            else:  # not a pirate
                dtotal += 1
                if game.odloc[i] == game.dloc[i]:
                    attack += 1
                    if game.knifeloc >= 0:
                        game.knifeloc = game.loc
                    if ran(1000) < 95 * (game.dflag - 2):
                        stick += 1
    # end of individual dwarf loop
    if dtotal == 0:
        return label2000
    elif dtotal == 1:
        rspeak(4)
    else:
        print('\nThere are', dtotal, 'threatening little dwarves in the room'
              ' with you.')
    if attack == 0:
        return label2000
    if game.dflag == 2:
        game.dflag = 3
    if attack == 1:
        rspeak(5)
        k = 52
    else:
        print()
        print(attack, 'of them throw knives at you!')
        k = 6
    if stick <= 1:
        rspeak(k+stick)
        if stick == 0:
            return label2000
    else:
        print()
        print(stick, 'of them get you!')
    game.oldloc2 = game.loc
    return death()

def label2000():
    if game.loc == 0:
        return death()
    kk = cave.shortDesc[game.loc]
    if game.abb[game.loc] % game.abbnum == 0 or kk is None:
        kk = cave.longDesc[game.loc]
    if not cave.forced(game.loc) and game.dark():
        if game.wzdark and pct(35):
            rspeak(23)
            game.oldloc2 = game.loc
            return death()
        kk = cave.rmsg[16]
    if game.toting(Item.BEAR):
        rspeak(141)
    speak(kk)
    if cave.forced(game.loc):
        return domove(1)
    if game.loc == 33 and pct(25) and not game.closing:
        rspeak(8)
    if not game.dark():
        game.abb[game.loc] += 1
        for obj in game.atloc[game.loc]:
            if obj > 100:
                obj -= 100
            if obj == Item.STEPS and game.toting(Item.NUGGET):
                continue
            if game.prop[obj] < 0:
                if game.closed:
                    continue
                game.prop[obj] = obj in (Item.RUG, Item.CHAIN)
                game.tally -= 1
                if game.tally == game.tally2 and game.tally != 0:
                    game.limit = min(35, game.limit)
            pspeak(obj, 1 if obj == Item.STEPS and
                             game.loc == game.fixed[Item.STEPS]
                          else game.prop[obj])
    return label2012

def label2012():
    global verb, obj
    verb = obj = 0
    return label2600

def label2600():
    global lastline
    for hint in range(4, 10):
        if game.hinted[hint]:
            continue
        if not cave.bitset(game.loc, hint):
            game.hintlc[hint] = -1
        game.hintlc[hint] += 1
        if game.hintlc[hint] >= cave.hints[hint].turns:
            if hint == 4 and (game.prop[Item.GRATE] != 0
                              or game.here(Item.KEYS)) or \
                    hint == 5 and (not game.here(Item.BIRD) or
                                   not game.toting(Item.ROD) or
                                   obj != Item.BIRD) or \
                    hint == 6 and (not game.here(Item.SNAKE)
                                   or game.here(Item.BIRD)) or \
                    hint == 7 and (any(game.atloc[l]
                                       for l in (game.loc, game.oldloc,
                                                 game.oldloc2))
                                   or game.holding <= 1) or \
                    hint == 8 and (game.prop[Item.EMERALD] == -1 or
                                   game.prop[Item.PYRAM] != -1):
                if hint != 5:
                    game.hintlc[hint] = 0
                continue
            game.hintlc[hint] = 0
            if not yes(cave.hints[hint].question, 0, 54):
                continue
            print()
            print('I am prepared to give you a hint, but it will cost you',
                  cave.hints[hint].points, 'points.')
            game.hinted[hint] = yes(175, cave.hints[hint].hint, 54)
            if game.hinted[hint] and game.limit > 30:
                game.limit += 30 * cave.hints[hint].points
    if game.closed:
        if game.prop[Item.OYSTER] < 0 and game.toting(Item.OYSTER):
            pspeak(Item.OYSTER, 1)
        for i in range(1, 65):
            if game.toting(i) and game.prop[i] < 0:
                game.prop[i] = -1 - game.prop[i]
    # Label 2605
    game.wzdark = game.dark()
    if 0 < game.knifeloc != game.loc:
        game.knifeloc = 0
    lastline = getin()
    return label2608

def label2608():
    global verb
    game.foobar = min(0, -game.foobar)
    if magic.on and game.turns == 0 and \
            (lastline.word1, lastline.word2) == ('MAGIC', 'MODE'):
        magic.maint()
    game.turns += 1
    if magic.on and demo and game.turns >= magic.short:
        magic.mspeak(1)
        normend()
    if verb == Action.SAY:
        if lastline.word2:
            verb = 0
        else:
            return vsay() or label19999
    if game.tally == 0 and 15 <= game.loc != 33:
        game.clock1 -= 1
    if game.clock1 == 0:
        game.prop[Item.GRATE] = game.prop[Item.FISSUR] = 0
        game.dloc = [0] * 6
        game.dseen = [False] * 6
        game.move(Item.TROLL, 0)
        game.move(Item.TROLL+100, 0)
        game.move(Item.TROLL2, 117)
        game.move(Item.TROLL2+100, 122)
        game.juggle(Item.CHASM)
        if game.prop[Item.BEAR] != 3:
            game.destroy(Item.BEAR)
        game.prop[Item.CHAIN] = game.prop[Item.AXE] = 0
        game.fixed[Item.CHAIN] = game.fixed[Item.AXE] = 0
        rspeak(129)
        game.clock1 = -1
        game.closing = True
        return label19999
    if game.clock1 < 0:
        game.clock2 -= 1
    if game.clock2 == 0:
        for i in (Item.BOTTLE, Item.PLANT, Item.OYSTER, Item.LAMP, Item.ROD,
                  Item.DWARF):
            game.prop[i] = game.put(i, 115, int(i == Item.BOTTLE))
        game.loc, game.oldloc, game.newloc = 115, 115, 115
        game.put(Item.GRATE, 116, 0)
        for i in (Item.SNAKE, Item.BIRD, Item.CAGE, Item.ROD2, Item.PILLOW):
            game.prop[i] = game.put(i, 116, int(i in (Item.SNAKE, Item.BIRD)))
        game.prop[Item.MIRROR] = game.put(Item.MIRROR, 115, 0)
        game.fixed[Item.MIRROR] = 116
        for i in range(1, 65):
            if game.toting(i):
                game.destroy(i)
        rspeak(132)
        game.closed = True
        return label2
    if game.prop[Item.LAMP] == 1:
        game.limit -= 1
    if game.limit <= 30 and game.here(Item.BATTER) and \
            game.prop[Item.BATTER] == 0 and game.here(Item.LAMP):
        rspeak(188)
        game.prop[Item.BATTER] = 1
        if game.toting(Item.BATTER):
            game.drop(Item.BATTER, game.loc)
        game.limit += 2500
        game.lmwarn = False
    elif game.limit == 0:
        game.limit = -1
        game.prop[Item.LAMP] = 0
        if game.here(Item.LAMP):
            rspeak(184)
    elif game.limit < 0 and game.loc <= 8:
        rspeak(185)
        game.gaveup = True
        normend()
    elif game.limit <= 30 and not game.lmwarn and game.here(Item.LAMP):
        game.lmwarn = True
        rspeak(183 if game.place[Item.BATTER] == 0
                   else 189 if game.prop[Item.BATTER] == 1
                   else 187)
    return label19999

def label19999():
    global lastline
    if lastline.word1 == 'ENTER' and lastline.word2 in ('STREA', 'WATER'):
        rspeak(70 if cave.liqloc(game.loc) == Item.WATER else 43)
        return label2012
    elif lastline.word1 == 'ENTER' and lastline.word2:
        lastline = lastline.moveup()
    elif lastline.word1 in ('WATER', 'OIL') and \
            lastline.word2 in ('PLANT', 'DOOR') and \
            game.at(Item[lastline.word2]):
        lastline = lastline._replace(word2='POUR')
    return label2610

def label2610():
    if lastline.word1 == 'WEST':
        game.iwest += 1
        if game.iwest == 10:
            rspeak(17)
    return label2630

def label2630():
    global obj, verb, lastline
    i = cave.vocab(lastline.word1)
    if i is None:
        rspeak(61 if pct(20) else 13 if pct(20) else 60)
        return label2600
    if isinstance(i, Movement):
        return domove(i)
    elif isinstance(i, Item):
        # Label 5000
        obj = i
        if game.fixed[obj] == game.loc or game.here(obj):
            return doaction()
        elif obj == Item.GRATE:
            if game.loc in (1, 4, 7):
                return domove(Movement.DEPRESSION)
            elif 9 < game.loc < 15:
                return domove(Movement.ENTRANCE)
            elif verb in (Action.FIND, Action.INVENT) and not lastline.word2:
                return doaction()
            else:
                print('\nI see no', lastline.in1, 'here.')
                return label2012
        elif (obj == Item.DWARF and game.dflag >= 2 and \
                game.loc in game.dloc[:5]) or \
                (obj == game.liq() and game.here(Item.BOTTLE)) or \
                obj == cave.liqloc(game.loc):
            return doaction()
        elif obj == Item.PLANT and game.at(Item.PLANT2) and \
                game.prop[Item.PLANT2] != 0:
            obj = Item.PLANT2
            return doaction()
        elif obj == Item.KNIFE and game.knifeloc == game.loc:
            game.knifeloc = -1
            rspeak(116)
            return label2012
        elif obj == Item.ROD and game.here(Item.ROD2):
            obj = Item.ROD2
            return doaction()
        elif verb in (Action.FIND, Action.INVENT) and not lastline.word2:
            return doaction()
        else:
            print('\nI see no', lastline.in1, 'here.')
            return label2012
    elif isinstance(i, Action):
        # Label 4000
        verb = i
        if verb in (Action.SAY, Action.SUSPEND, Action.RESUME):
            obj = lastline.word2 is not None
            # This assignment just indicates whether an object was supplied.
        elif lastline.word2:
            lastline = lastline.moveup()
            return label2610
        if obj:
            return transitive()
        else:
            return intransitive()
    elif isinstance(i, MsgWord):
        rspeak(i)
        return label2012
    else:
        bug(22)


# Verb functions:

def what():
    global obj
    print()
    print(lastline.in1, 'what?')
    obj = 0
    return label2600

def actspk():
    rspeak(cave.actspk[verb])

def vscore():
    score = game.score(True)
    print()
    print('If you were to quit now, you would score', score,
          'out of a possible 350.')
    game.gaveup = yes(143, 54, 54)
    if game.gaveup:
        normend()

def vfoo():
    k = cave.vocab(lastline.word1, MsgWord)
    if k is None:
        k = -1
    if game.foobar == 1-k:
        game.foobar = k
        if k != 4:
            rspeak(54)
            return
        game.foobar = 0
        if game.place[Item.EGGS] == 92 or \
                (game.toting(Item.EGGS) and game.loc == 92):
            rspeak(42)
        else:
            if game.place[Item.EGGS] == game.place[Item.TROLL] == \
                    game.prop[Item.TROLL] == 0:
                game.prop[Item.TROLL] = 1
            game.move(Item.EGGS, 92)
            pspeak(Item.EGGS, 0 if game.loc == 92
                                else 1 if game.here(Item.EGGS)
                                else 2)
    else:
        rspeak(151 if game.foobar else 42)

def vbrief():
    game.abbnum = 10000
    game.detail = 3
    rspeak(156)

def vhours():
    if magic.on:
        magic.mspeak(6)
        magic.hours()
    else:
        print()
        print('Colossal Cave is open all day, every day.')

def vquit():
    game.gaveup = yes(22, 54, 54)
    if game.gaveup:
        normend()

def vinvent():
    spk = 98
    for i in range(65):
        if i == Item.BEAR or not game.toting(i):
            continue
        if spk == 98:
            rspeak(99)
        pspeak(i, -1, blklin=False)
        spk = 0
    if game.toting(Item.BEAR):
        spk = 141
    rspeak(spk)

def vwave():
    # Label 9090
    if not game.toting(obj) and \
            not (obj == Item.ROD and game.toting(Item.ROD2)):
        rspeak(29)
    elif obj != Item.ROD or not game.at(Item.FISSUR) or \
            not game.toting(obj) or game.closing:
        actspk()
    else:
        game.prop[Item.FISSUR] = 1 - game.prop[Item.FISSUR]
        pspeak(Item.FISSUR, 2 - game.prop[Item.FISSUR])

def iveat():
    if game.here(Item.FOOD):
        game.destroy(Item.FOOD)
        rspeak(72)
    else:
        return what()

def veat():
    # Label 9140
    if obj == Item.FOOD:
        game.destroy(Item.FOOD)
        rspeak(72)
    elif obj in (Item.BIRD, Item.SNAKE, Item.CLAM, Item.OYSTER, Item.DWARF,
                 Item.DRAGON, Item.TROLL, Item.BEAR):
        rspeak(71)
    else:
        actspk()

def vthrow():
    # Label 9170
    global obj
    if game.toting(Item.ROD2) and obj == Item.ROD and not game.toting(Item.ROD):
        obj = Item.ROD2
    if not game.toting(obj):
        actspk()
    elif 50 <= obj < 65 and game.at(Item.TROLL):
        game.drop(obj, 0)
        game.move(Item.TROLL, 0)
        game.move(Item.TROLL+100, 0)
        game.drop(Item.TROLL2, 117)
        game.drop(Item.TROLL2+100, 122)
        game.juggle(Item.CHASM)
        rspeak(159)
    elif obj == Item.FOOD and game.here(Item.BEAR):
        obj = Item.BEAR
        return vfeed()
    elif obj == Item.AXE:
        ixs = [i for i in range(5) if game.dloc[i] == game.loc]
        if ixs:
            if ran(3) == 0:
                rspeak(48)
            else:
                game.dseen[ixs[0]] = False
                game.dloc[ixs[0]] = 0
                game.dkill += 1
                rspeak(149 if game.dkill == 1 else 47)
        elif game.at(Item.DRAGON) and game.prop[Item.DRAGON] == 0:
            rspeak(152)
        elif game.at(Item.TROLL):
            rspeak(158)
        elif game.here(Item.BEAR) and game.prop[Item.BEAR] == 0:
            game.drop(Item.AXE, game.loc)
            game.fixed[Item.AXE] = FIXED
            game.prop[Item.AXE] = 1
            game.juggle(Item.BEAR)  # Don't try this at home, kids.
            rspeak(164)
            return label2012
        else:
            obj = 0
            return vkill()
        game.drop(Item.AXE, game.loc)
        return domove(Movement.NULL)
    else:
        return vdrop()

def vfind():
    # Label 9190
    if game.toting(obj):
        rspeak(24)
    elif game.closed:
        rspeak(138)
    elif (obj == Item.DWARF and game.dflag >= 2 and \
            game.loc in game.dloc[:5]) or \
            game.at(obj) or \
            (game.liq() == obj and game.at(Item.BOTTLE)) or \
            obj == cave.liqloc(game.loc):
        rspeak(94)
    else:
        actspk()

def vbreak():
    # Label 9280
    if obj == Item.VASE and game.prop[Item.VASE] == 0:
        if game.toting(Item.VASE):
            game.drop(Item.VASE, game.loc)
        game.prop[Item.VASE] = 2
        game.fixed[Item.VASE] = FIXED
        rspeak(198)
    elif obj != Item.MIRROR:
        actspk()
    elif not game.closed:
        rspeak(148)
    else:
        rspeak(197)
        rspeak(136)
        normend()

def vwake():
    # Label 9290
    if obj == Item.DWARF and game.closed:
        rspeak(199)
        rspeak(136)
        normend()
    else:
        actspk()

def vtake():
    global obj
    if not obj:
        if len(game.atloc[game.loc]) != 1 or \
                (game.dflag >= 2 and game.loc in game.dloc[:5]):
            return what()
        else:
            obj = game.atloc[game.loc][0]
    # Label 9010
    if game.toting(obj):
        actspk()
        return
    spk = 25
    if obj == Item.PLANT and game.prop[Item.PLANT] <= 0:
        spk = 115
    if obj == Item.BEAR and game.prop[Item.BEAR] == 1:
        spk = 169
    if obj == Item.CHAIN and game.prop[Item.BEAR] != 0:
        spk = 170
    if game.fixed[obj]:
        rspeak(spk)
        return
    if obj in (Item.WATER, Item.OIL):
        if not game.here(Item.BOTTLE) or game.liq() != obj:
            obj = Item.BOTTLE
            if game.toting(Item.BOTTLE) and game.prop[Item.BOTTLE] == 1:
                return vfill()
            else:
                if game.prop[Item.BOTTLE] != 1:
                    spk = 105
                if not game.toting(Item.BOTTLE):
                    spk = 104
                rspeak(spk)
                return
        obj = Item.BOTTLE
    if game.holding >= 7:
        rspeak(92)
        return
    if obj == Item.BIRD and game.prop[Item.BIRD] == 0:
        if game.toting(Item.ROD):
            rspeak(26)
            return
        if not game.toting(Item.CAGE):
            rspeak(27)
            return
        game.prop[Item.BIRD] = 1
    if obj in (Item.BIRD, Item.CAGE) and game.prop[Item.BIRD] != 0:
        game.carry(Item.BIRD + Item.CAGE - obj, game.loc)
    game.carry(obj, game.loc)
    k = game.liq()
    if obj == Item.BOTTLE and k != 0:
        game.place[k] = TOTING
    rspeak(54)

def vopen():
    global obj
    if not obj:
        if game.here(Item.CLAM):
            obj = Item.CLAM
        elif game.here(Item.OYSTER):
            obj = Item.OYSTER
        if game.at(Item.DOOR):
            obj = Item.DOOR
        elif game.at(Item.GRATE):
            obj = Item.GRATE
        if obj != 0 and game.here(Item.CHAIN):
            return what()
        elif game.here(Item.CHAIN):
            obj = Item.CHAIN
        elif obj == 0:
            rspeak(28)
            return
    # Label 9040
    spk = cave.actspk[verb]
    if obj in (Item.CLAM, Item.OYSTER):
        k = (obj == Item.OYSTER)
        spk = 124 + k
        if game.toting(obj):
            spk = 120 + k
        if not game.toting(Item.TRIDENT):
            spk = 122 + k
        if verb == Action.LOCK:
            spk = 61
        if spk == 124:
            game.destroy(Item.CLAM)
            game.drop(Item.OYSTER, game.loc)
            game.drop(Item.PEARL, 105)
    elif obj == Item.DOOR:
        spk = 54 if game.prop[Item.DOOR] == 1 else 111
    elif obj == Item.CAGE:
        spk = 32
    elif obj == Item.KEYS:
        spk = 55
    elif obj == Item.CHAIN:
        if not game.here(Item.KEYS):
            spk = 31
        elif verb == Action.LOCK:
            spk = 172
            if game.prop[Item.CHAIN] != 0:
                spk = 34
            if game.loc != 130:
                spk = 173
            if spk == 172:
                game.prop[Item.CHAIN] = 2
                if game.toting(Item.CHAIN):
                    game.drop(Item.CHAIN, game.loc)
                game.fixed[Item.CHAIN] = FIXED
        else:
            spk = 171
            if game.prop[Item.BEAR] == 0:
                spk = 41
            if game.prop[Item.CHAIN] == 0:
                spk = 37
            if spk == 171:
                game.prop[Item.CHAIN] = game.fixed[Item.CHAIN] = 0
                if game.prop[Item.BEAR] != 3:
                    game.prop[Item.BEAR] = 2
                game.fixed[Item.BEAR] = 2 - game.prop[Item.BEAR]
    elif obj == Item.GRATE:
        if not game.here(Item.KEYS):
            spk = 31
        elif game.closing:
            spk = 130
            if not game.panic:
                game.clock2 = 15
            game.panic = True
        else:
            spk = 34 + game.prop[Item.GRATE]
            game.prop[Item.GRATE] = (verb != Action.LOCK)
            spk += 2 * game.prop[Item.GRATE]
    rspeak(spk)

def vread():
    global obj
    if not obj:
        if game.here(Item.MAGZIN):
            obj = Item.MAGZIN
        if game.here(Item.TABLET):
            obj = obj * 100 + Item.TABLET
        if game.here(Item.MESSAG):
            obj = obj * 100 + Item.MESSAG
        if game.closed and game.toting(Item.OYSTER):
            obj = Item.OYSTER
        if obj > 100 or obj == 0 or game.dark():
            return what()
    # Label 9270
    if game.dark():
        print()
        print('I see no', lastline.in1, 'here.')
    else:
        spk = {
            Item.MAGZIN: 190,
            Item.TABLET: 196,
            Item.MESSAG: 191
        }.get(obj, cave.actspk[verb])
        if obj == Item.OYSTER and game.hinted[2] and game.toting(Item.OYSTER):
            spk = 194
        if obj != Item.OYSTER or game.hinted[2] or \
                not game.toting(Item.OYSTER) or not game.closed:
            rspeak(spk)
        else:
            game.hinted[2] = yes(192, 193, 54)

def vkill():
    # Label 9120
    global verb, obj, lastline
    if obj == 0:
        if game.dflag >= 2 and game.loc in game.dloc[:5]:
            obj = Item.DWARF
        if game.here(Item.SNAKE):
            obj = obj * 100 + Item.SNAKE
        if game.at(Item.DRAGON) and game.prop[Item.DRAGON] == 0:
            obj = obj * 100 + Item.DRAGON
        if game.at(Item.TROLL):
            obj = obj * 100 + Item.TROLL
        if game.here(Item.BEAR) and game.prop[Item.BEAR] == 0:
            obj = obj * 100 + Item.BEAR
        if obj > 100:
            return what()
        if obj == 0:
            if game.here(Item.BIRD) and verb != Action.THROW:
                obj = Item.BIRD
            if game.here(Item.CLAM) or game.here(Item.OYSTER):
                obj = obj * 100 + Item.CLAM
            if obj > 100:
                return what()
    if obj == Item.BIRD:
        if game.closed:
            rspeak(137)
        else:
            game.destroy(Item.BIRD)
            game.prop[Item.BIRD] = 0
            if game.place[Item.SNAKE] == 19:
                game.tally2 += 1
            rspeak(45)
    elif obj == 0:
        rspeak(44)
    elif obj in (Item.CLAM, Item.OYSTER):
        rspeak(150)
    elif obj == Item.SNAKE:
        rspeak(46)
    elif obj == Item.DWARF:
        if game.closed:
            rspeak(136)
            normend()
        else:
            rspeak(49)
    elif obj == Item.DRAGON:
        if game.prop[Item.DRAGON] != 0:
            rspeak(167)
        else:
            rspeak(49)
            (verb, obj) = (0, 0)
            lastline = getin()
            if lastline.word1 not in ('YES', 'Y'):
                return label2608
            pspeak(Item.DRAGON, 1)
            game.prop[Item.DRAGON] = 2
            game.prop[Item.RUG] = 0
            game.move(Item.DRAGON+100, FIXED)
            game.move(Item.RUG+100, 0)
            game.move(Item.DRAGON, 120)
            game.move(Item.RUG, 120)
            for i in range(65):
                if game.place[i] in (119, 121):
                    game.move(i, 120)
            game.loc = 120
            return domove(Movement.NULL)
    elif obj == Item.TROLL:
        rspeak(157)
    elif obj == Item.BEAR:
        rspeak(165 + (game.prop[Item.BEAR] + 1) // 2)
    else:
        actspk()

def vpour():
    # Label 9130
    global obj
    if obj in (Item.BOTTLE, 0):
        obj = game.liq()
    if obj == 0:
        return what()
    elif not game.toting(obj):
        actspk()
    elif obj not in (Item.OIL, Item.WATER):
        rspeak(78)
    else:
        game.prop[Item.BOTTLE] = 1
        game.place[obj] = 0
        if game.at(Item.DOOR):
            game.prop[Item.DOOR] = (obj == Item.OIL)
            rspeak(113 + game.prop[Item.DOOR])
        elif game.at(Item.PLANT):
            if obj != Item.WATER:
                rspeak(112)
            else:
                pspeak(Item.PLANT, game.prop[Item.PLANT] + 1)
                game.prop[Item.PLANT] = (game.prop[Item.PLANT] + 2) % 6
                game.prop[Item.PLANT2] = game.prop[Item.PLANT] // 2
                return domove(Movement.NULL)
        else:
            rspeak(77)

def vdrink():
    # Label 9150
    if obj == 0 and cave.liqloc(game.loc) != Item.WATER and \
            (game.liq() != Item.WATER or not game.here(Item.BOTTLE)):
        return what()
    elif obj in (0, Item.WATER):
        if game.liq() == Item.WATER and game.here(Item.BOTTLE):
            game.prop[Item.BOTTLE] = 1
            game.place[Item.WATER] = 0
            rspeak(74)
        else:
            actspk()
    else:
        rspeak(110)

def vfill():
    # Label 9220
    if obj == Item.VASE:
        if cave.liqloc(game.loc) == 0:
            rspeak(144)
        elif not game.toting(Item.VASE):
            rspeak(29)
        else:
            rspeak(145)
            game.prop[Item.VASE] = 2
            game.fixed[Item.VASE] = FIXED
            # In the original Fortran, when the vase is filled with water or
            # oil, its property is set so that it breaks into pieces, *but* the
            # code then branches to label 9024 to actually drop the vase.  Once
            # you cut out the unreachable states, it turns out that the vase
            # remains intact if the pillow is present, but even if it survives
            # it is still marked as a fixed object and can't be picked up
            # again.  This is probably a bug in the original code, but who am I
            # to fix it?
            if game.at(Item.PILLOW):
                game.prop[Item.VASE] = 0
            pspeak(Item.VASE, game.prop[Item.VASE] + 1)
            game.drop(obj, game.loc)
    elif obj not in (0, Item.BOTTLE):
        actspk()
    elif obj == 0 and not game.here(Item.BOTTLE):
        return what()
    elif game.liq() != 0:
        rspeak(105)
    elif cave.liqloc(game.loc) == 0:
        rspeak(106)
    else:
        game.prop[Item.BOTTLE] = cave.bitset(game.loc, Cond.OIL)
        if game.toting(Item.BOTTLE):
            game.place[game.liq()] = TOTING
        rspeak(108 if game.liq() == Item.OIL else 107)

def vblast():
    # Label 9230
    if game.prop[Item.ROD2] < 0 or not game.closed:
        actspk()
    else:
        bonus = 133
        if game.loc == 115:
            bonus = 134
        if game.here(Item.ROD2):
            bonus = 135
        rspeak(bonus)
        normend(bonus)
        # Fin

def von():
    # Label 9070
    if not game.here(Item.LAMP):
        actspk()
    elif game.limit < 0:
        rspeak(184)
    else:
        game.prop[Item.LAMP] = 1
        rspeak(39)
    if game.wzdark:
        return label2000

def voff():
    # Label 9080
    if not game.here(Item.LAMP):
        actspk()
    else:
        game.prop[Item.LAMP] = 0
        rspeak(40)
        if game.dark():
            rspeak(16)

def vdrop():
    # Label 9020
    global obj
    if game.toting(Item.ROD2) and obj == Item.ROD and not game.toting(Item.ROD):
        obj = Item.ROD2
    if not game.toting(obj):
        actspk()
        return
    if obj == Item.BIRD and game.here(Item.SNAKE):
        rspeak(30)
        if game.closed:
            rspeak(136)
            normend()
        game.destroy(Item.SNAKE)
        game.prop[Item.SNAKE] = 1
    elif obj == Item.COINS and game.here(Item.VEND):
        game.destroy(Item.COINS)
        game.drop(Item.BATTER, game.loc)
        pspeak(Item.BATTER, 0)
        return
    elif obj == Item.BIRD and game.at(Item.DRAGON) and \
            game.prop[Item.DRAGON] == 0:
        rspeak(154)
        game.destroy(Item.BIRD)
        game.prop[Item.BIRD] = 0
        if game.place[Item.SNAKE] == 19:
            game.tally2 += 1
        return
    elif obj == Item.BEAR and game.at(Item.TROLL):
        rspeak(163)
        game.move(Item.TROLL, 0)
        game.move(Item.TROLL+100, 0)
        game.move(Item.TROLL2, 117)
        game.move(Item.TROLL2+100, 122)
        game.juggle(Item.CHASM)
        game.prop[Item.TROLL] = 2
    elif obj == Item.VASE and game.loc != 96:
        game.prop[Item.VASE] = 0 if game.at(Item.PILLOW) else 2
        pspeak(Item.VASE, game.prop[Item.VASE] + 1)
        if game.prop[Item.VASE] != 0:
            game.fixed[Item.VASE] = FIXED
    else:
        rspeak(54)
    k = game.liq()
    if k == obj:
        obj = Item.BOTTLE
    if obj == Item.BOTTLE and k != 0:
        game.place[k] = 0
    if obj == Item.CAGE and game.prop[Item.BIRD] != 0:
        game.drop(Item.BIRD, game.loc)
    if obj == Item.BIRD:
        game.prop[Item.BIRD] = 0
    game.drop(obj, game.loc)

def vfeed():
    # Label 9210
    if obj == Item.BIRD:
        rspeak(100)
    elif obj == Item.SNAKE:
        if not game.closed and game.here(Item.BIRD):
            game.destroy(Item.BIRD)
            game.prop[Item.BIRD] = 0
            game.tally2 += 1
            rspeak(101)
        else:
            rspeak(102)
    elif obj == Item.TROLL:
        rspeak(182)
    elif obj == Item.DRAGON:
        rspeak(110 if game.prop[Item.DRAGON] != 0 else 102)
    elif obj == Item.DWARF:
        if not game.here(Item.FOOD):
            actspk()
        else:
            game.dflag += 1
            rspeak(103)
    elif obj == Item.BEAR:
        if not game.here(Item.FOOD):
            p = game.prop[Item.BEAR]
            if p in (1,2):
                actspk()
            else:
                rspeak(102 if p == 0 else 110)
        else:
            game.destroy(Item.FOOD)
            game.prop[Item.BEAR] = 1
            game.fixed[Item.AXE] = 0
            game.prop[Item.AXE] = 0
            rspeak(168)
    else:
        rspeak(14)

def vsay():
    # Label 9030
    global lastline, obj
    tk = lastline.in2 if lastline.in2 is not None else lastline.in1
    if lastline.word2 is not None:
        lastline = lastline._replace(word1=lastline.word2)
    word1 = cave.vocab(lastline.word1)
    if word1 is Movement.XYZZY or word1 is Movement.PLUGH or \
            word1 is Movement.PLOVER or word1 is Action.FOO:
        lastline = lastline._replace(word2=None)
        obj = 0
        return label2630
    else:
        print()
        print('Okay, "' + tk + '".')

def vsuspend():
    if obj:
        cmd = shlex.split(lastline.raw)
        if len(cmd) != 2 or not cmd[1]:
            print()
            print('Save to what file?')
            return
        filename = os.path.expanduser(cmd[1])
    else:
        filename = DEFAULT_SAVEFILE
    if magic.on:
        if demo:
            rspeak(201)
            return
        print()
        print('I can suspend your adventure for you so that you can resume'
              ' later, but')
        print('you will have to wait at least', magic.latency,
              'minutes before continuing.')
    else:
        print()
        print('I can suspend your adventure for you so that you can resume'
              ' later.')
    if not yes(200, 54, 54):
        return
    if magic.on:
        (game.saved, game.savet) = datime()
    print()
    print('Saving to', filename, '...')
    try:
        with open(filename, 'wb') as fp:
            pickle.dump(fp, game)
    except Exception:
        traceback.print_exc()
    else:
        if magic.on:
            magic.ciao()
        sys.exit()

def vresume():
    if obj:
        cmd = shlex.split(lastline.raw)
        if len(cmd) != 2 or not cmd[1]:
            print()
            print('Restore from what file?')
            return
        filename = os.path.expanduser(cmd[1])
    else:
        filename = DEFAULT_SAVEFILE
    if magic.on and demo:
        magic.mspeak(9)
        return None
    if game.turns > 1:
        print()
        print('To resume an earlier Adventure, you must abandon the current'
              ' one.')
        # This message is taken from the 430 pt. version of Adventure (v.2.5).
        if not yes(200, 54, 54):
            return None
    with open(filename, 'rb') as fp:
        return resume(filename, fp)

def resume(fname, fp):
    global game
    print()
    print('Restoring from', fname, '...')
    game = load(fp, Game)
    magic.start()
    return domove(Movement.NULL)

iverbs = {
    Action.NOTHING: lambda: rspeak(54),
    Action.WALK: actspk,
    Action.DROP: what,
    Action.SAY: what,
    Action.WAVE: what,
    Action.CALM: what,
    Action.RUB: what,
    Action.THROW: what,
    Action.FIND: what,
    Action.FEED: what,
    Action.BREAK: what,
    Action.WAKE: what,
    Action.TAKE: vtake,
    Action.OPEN: vopen,
    Action.LOCK: vopen,
    Action.EAT: iveat,
    Action.QUIT: vquit,
    Action.INVENT: vinvent,
    Action.SCORE: vscore,
    Action.FOO: vfoo,
    Action.BRIEF: vbrief,
    Action.READ: vread,
    Action.SUSPEND: vsuspend,
    Action.RESUME: vresume,
    Action.HOURS: vhours,
    Action.ON: von,
    Action.OFF: voff,
    Action.KILL: vkill,
    Action.POUR: vpour,
    Action.DRINK: vdrink,
    Action.FILL: vfill,
    Action.BLAST: vblast,
}

def intransitive():
    # Label 4080 (intransitive verb handling)
    return iverbs.get(verb, lambda: bug(23))() or label2012

tverbs = {
    Action.TAKE: vtake,
    Action.DROP: vdrop,
    Action.SAY: vsay,
    Action.OPEN: vopen,
    Action.LOCK: vopen,
    Action.NOTHING: lambda: rspeak(54),
    Action.ON: von,
    Action.OFF: voff,
    Action.WAVE: vwave,
    Action.CALM: actspk,
    Action.WALK: actspk,
    Action.QUIT: actspk,
    Action.SCORE: actspk,
    Action.FOO: actspk,
    Action.BRIEF: actspk,
    Action.HOURS: actspk,
    Action.KILL: vkill,
    Action.POUR: vpour,
    Action.EAT: veat,
    Action.DRINK: vdrink,
    Action.RUB: lambda: actspk() if obj == Item.LAMP else rspeak(76),
    Action.THROW: vthrow,
    Action.FIND: vfind,
    Action.INVENT: vfind,
    Action.FEED: vfeed,
    Action.FILL: vfill,
    Action.BLAST: vblast,
    Action.READ: vread,
    Action.BREAK: vbreak,
    Action.WAKE: vwake,
    Action.SUSPEND: vsuspend,
    Action.RESUME: vresume,
}

def transitive():
    # Label 4090 (transitive verb handling)
    return tverbs.get(verb, lambda: bug(24))() or label2012

if __name__ == '__main__':
    main()
