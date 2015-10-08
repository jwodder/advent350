#!/usr/bin/python
from   __future__  import print_function
import argparse
from   collections import defaultdict, namedtuple
from   datetime    import datetime
from   errno       import ENOENT
import itertools
import os.path
import pickle
import sys
import traceback

# Configuration:
MAGIC = True
magicfile = os.path.expanduser('~/.advmagic')
savefile = os.path.expanduser('~/.adventure')

if sys.version_info[0] >= 3:
    raw_input = input
    xrange = range

def ran(n):
    d = 1
    if ran.r == 0:
        (d, ran.r) = datime()
        ran.r = 18 * ran.r + 5
        d = 1000 + d % 1000
    for _ in xrange(d):
        ran.r = (ran.r * 1021) % 1048576
        return (n * ran.r) // 1048576
ran.r = 0

MAXDIE = 3
CHLOC = 114
CHLOC2 = 140

class Limits(object):
    OBJECTS   = 100
    LOCATIONS = 150  # advent.for: LOCSIZ
    RTEXT     = 205  # advent.for: RTXSIZ
    HINTS     =  20  # advent.for: HNTSIZ
    MTEXT     =  35  # advent.for: MAGSIZ
   #CLASSES   =  12  # advent.for: CLSMAX
    ACTSPK    =  35  # advent.for: VRBSIZ

def mkenum(name, *enums):
    return type(name, (object,), {e: i for (start, words) in enums
                                       for (i,e) in enumerate(words.split(),
                                                              start=start)})

Item = mkenum('Item', (1, 'KEYS LAMP GRATE CAGE ROD ROD2 STEPS BIRD DOOR PILLOW'
                          ' SNAKE FISSUR TABLET CLAM OYSTER MAGZIN DWARF KNIFE'
                          ' FOOD BOTTLE WATER OIL MIRROR PLANT PLANT2'), 
                      (28, 'AXE'),
                      (31, 'DRAGON CHASM TROLL TROLL2 BEAR MESSAG VOLCANO VEND'
                           ' BATTER'),
                      (50, 'NUGGET'),
                      (54, 'COINS CHEST EGGS TRIDENT VASE EMERALD PYRAM PEARL'
                           ' RUG SPICES CHAIN'))

Movement = mkenum('Movement', (8, 'BACK'),
                              (21, 'NULL'),
                              (57, 'LOOK'),
                              (63, 'DEPRESSION ENTRANCE'),
                              (67, 'CAVE'))

Action = mkenum('Action', (1, 'TAKE DROP SAY OPEN NOTHING LOCK ON OFF WAVE CALM'
                              ' WALK KILL POUR EAT DRINK RUB THROW QUIT FIND'
                              ' INVENT FEED FILL BLAST SCORE FOO BRIEF READ'
                              ' BREAK WAKE SUSPEND HOURS RESUME'))

Cond = mkenum('Cond', (0, 'LIGHT OIL LIQUID NO_PIRATE'))

def indexLines(lines, qty):
    data = [None] * (qty+1)
    for i, block in itertools.groupby(lines, lambda s: int(s.split('\t')[0])):
        dict[i] = ''.join(s.split('\t', 1)[1] for s in block)
        if '>$<' in dict[i]:
            dict[i] = None
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

def liq2(self, p):
    return (Item.WATER, 0, Item.OIL)[p]

def pct(x):
    return ran(100) < x

def load(fp, ofType):
    obj = pickle.load(fp)
    if not isinstance(obj, ofType):
        raise TypeError('Expected %s object in file; got %s object'
                        % (ofType.__name__, obj.__class__.__name__))

class Travel(namedtuple('Travel', 'dest verbs verb1 uncond chance nodwarf'
                                  ' carry here obj notprop forced')):
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
                   obj = M % 100,
                   notprop = M // 100 - 3 if 300 < M else None,
                   forced = line[1] == 1)

Hint = namedtuple('Hint', 'turns points question hint')

class Adventure(object):
    def __init__(self, advdat)
        sections = {index: list(sect)[1:-1]
                    for index, sect in itertools.groupby(advdat, bysection)}
        self.longDesc = indexLines(sections[1], Limits.LOCATIONS)
        self.shortDesc = indexLines(sections[2], Limits.LOCATIONS)
        self.travel = nonemap(lambda s: list(map(Travel.fromEntry,
                                                 s.splitlines())),
                              indexLines(sections[3], Limits.LOCATIONS))
        self.vocabulary = defaultdict(list)
        for entry in sections[4]:
            i, word = entry.split('\t')[:2]
            self.vocabulary[word].append(int(i))
        self.itemDesc = [None] * Limits.OBJECTS
        obj = 0
        for line in sections[5]:
            num, txt = line.split('\t', 1)
            num = int(num)
            if '>$<' in txt:
                txt = None
            if 1 <= num < 100:
                obj = num
                itemDesc[obj] = [txt]
            else:
                state = num // 100
                try:
                    itemDesc[obj][state+1] += txt
                except IndexError:
                    itemDesc[obj].append(txt)
        self.rmsg = indexLines(sections[6], Limits.RTEXT)
        self.startplace = [0] * Limits.OBJECTS
        self.startfixed = [0] * Limits.OBJECTS
        for locs in map(intTSV, sections[7]):
            self.startplace[locs[0]] = locs[1]
            try:
                self.startfixed[locs[0]] = locs[2]
            except IndexError:
                self.startfixed[locs[0]] = 0
        self.actspk = indexLines(sections[8], Limits.ACTSPK)
        self.cond = [0] * Limits.LOCATIONS
        for cs in map(intTSV, sections[9]):
            for loc in cs[1:]:
                self.cond[loc] |= 1 << cs[0]
        self.classes = list(map(lambda s: s.strip().split('\t'), sections[10]))
        self.hints = nonemap(lambda s: Hint(*intTSV(s)),
                             indexLines(sections[11], Limits.HINTS))
        self.magic = indexLines(sections[12], Limits.MTEXT)

    def liqloc(self, loc):
        return liq2(self.bitset(loc, Cond.OIL) if self.bitset(loc, Cond.LIQUID)
                                               else 1)

    def bitset(self, loc, n):
        return self.cond[loc] & (1 << n)

    def forced(self, loc):
        return self.travel[loc][0].forced

    def vocab(self, word, wordtype):
        matches = self.vocabulary[word]
        if wordtype >= 0:
            matches = [i for i in matches if i // 1000 == wordtype]
        if not matches:
            if wordtype >= 0:
                bug(5)
            return -1
        return matches[0] % 1000 if wordtype >= 0 else min(matches)
        # When returning values of a specified type, there can be no more than
        # one match; if there is more than one, someone's been messing with the
        # data sections.


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
        self.atloc = [[] for _ in xrange(Limits.LOCATIONS + 1)]
        self.place = game.startplace[:]
        self.fixed = game.startfixed[:]
        self.saved = -1
        self.savet = 0
        self.gaveup = False
        ### TODO: Replace the calls to `drop` with modifications of `atloc`
        for k in xrange(len(self.fixed)-1, -1, -1):
            if self.fixed[k] > 0:
                self.drop(k+100, self.fixed[k])
                self.drop(k, self.place[k])
        for k in xrange(len(self.fixed)-1, -1, -1):
            if self.place[k] != 0 and self.fixed[k] <= 0:
                self.drop(k, self.place[k])

    def toting(self, item):
        return self.place[item] == -1

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
            if self.place[obj] == -1:
                return
            self.place[obj] = -1
            self.holding += 1
        self.atloc[where].remove(obj)

    def drop(self, obj, where):
        if obj > 100:
            self.fixed[obj-100] = where
        else:
            if self.place[obj] == -1:
                self.holding -= 1
            self.place[obj] = where
        if where > 0:
            self.atloc[where].insert(0, obj)

    def move(self, obj, where):
        from = self.fixed[obj-100] if obj > 100 else self.place[obj]
        if 0 < from <= 300:
            self.carry(obj, from)
        self.drop(obj, where)

    def put(self, obj, where, pval):
        self.move(obj, where)
        return -1 - pval

    def destroy(self, obj):
        self.move(obj, 0)

    def juggle(self, obj):
        self.move(obj, self.place[obj])
        self.move(obj+100, self.fixed[obj])

    def score(scoring):
        score = 0
        for i in xrange(50, 65):
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
        for i in xrange(1, 10):
            if self.hinted[i]:
                score -= cave.hints[i].points
        return score


class Magic(object):
    def __init__(self):
        # These arrays hold the times when adventurers are allowed into
        # Colossal Cave; `self.wkday` is for weekdays, `self.wkend` for
        # weekends, and `self.holid` for holidays (days with special hours).
        # If element `n` of an array is true, then the hour `n:00` through
        # `n:59` is considered "prime time," i.e., the cave is closed then.
        self.wkday = [False] * 8 + [True] * 10 + [False] * 6
        self.wkend = [False] * 24
        self.holid = [False] * 24
        self.hbegin = 0  # start of next holiday
        self.hend = -1  # end of next holiday
        self.short = 30  # turns allowed in a short/demonstration game
        self.magnm = 11111  # magic number
        self.latency = 90  # time required to wait after saving
        self.magic = 'DWARF'  # magic word
        self.hname = ''  # name of next holiday
        self.msg = ''  # MOTD, initially null


demo = False
bonus = 0
verb = None
obj = None
word1, in1, word2, in2 = None, None, None, None

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

def rspeak(msg):
    if msg != 0:
        speak(cave.rmsg[msg])

def getin(blklin=True):
    if blklin:
        print()
    while True:
        raw = (raw_input('> ').split() + [None]*2)[:2]
        if raw[0] is None and blklin:
            continue
        words = nonemap(lambda s: s[:5].upper(), raw)
        return (words[0], raw[0], words[1], raw[1])

def getInt(prompt='> '):
    while True:
        raw = raw_input(prompt)
        try:
            return int(raw)
        except ValueError:
            pass

def yes(x, y, z):
    return yesx(x, y, z, rspeak)

def yesx(x, y, z, spk, blklin=True):
    while True:
        spk(x, blklin=blklin)
        reply = getin(blklin=blklin)[0]
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
    for trav in itertools.dropwhile(lambda t: not t.forced and
                                              motion not in t.verbs,
                                    cave.travel[game.loc]):
        if trav.uncond or\
                trav.chance and pct(trav.chance) or\
                trav.carry and game.toting(trav.carry) or\
                trav.here and (game.toting(trav.here) or game.at(trav.here)) or\
                game.prop[trav.obj] != trav.notprop:
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
                game.fixed[Item.BEAR] = -1
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
        for i in xrange(64, 0, -1):
            if game.toting(i):
                game.drop(i, 1 if i == Item.LAMP else game.oldloc2)
        game.loc = game.oldloc = 3
        return label2000

def normend():
    score = game.score(False)
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
    global word1, in1, word2, in2
    # Label 5010
    if word2:
        # Label 2800
        (word1, in1) = (word2, in2)
        word2 = in2 = None
        return label2610
    elif verb:
        return transitive()
    else:
        print('\nWhat do you want to do with the ' + in1 + '?')
        return label2600

def mspeak(msg, blklin=True):  ### MAGIC
    if msg != 0:
        speak(cave.magic[msg], blklin=blklin)

def ciao():  ### MAGIC
    mspeak(32)
    sys.exit()

def yesm(x, y, z, blklin=True):  ### MAGIC
    return yesx(x, y, z, mspeak, blklin=blklin)

def datime():  ### MAGIC
    # This function is supposed to return:
    # - the number of days since 1 Jan 1977
    # - the number of minutes past midnight
    ### TODO: Double-check this
    delta = datetime.today() - datetime(1977, 1, 1)
    return (delta.days, delta.seconds // 60)

def start():  ### MAGIC
    (d,t) = datime()
    if game.saved != -1:
        delay = (d - game.saved) * 1440 + (t - game.savet)
        if delay < magic.latency:
            print('This adventure was suspended a mere', delay, 'minutes ago.')
            if delay < magic.latency // 3:
                mspeak(2)
            else:
                mspeak(8)
                if wizard():
                    game.saved = -1
                    return False
                mspeak(9)
            sys.exit()
    if (magic.holid if magic.hbegin <= d <= magic.hend
                    else magic.wkend if d % 7 <= 1
                    else magic.wkday)[t // 60]:
        # Prime time (cave closed)
        mspeak(3)
        hours()
        mspeak(4)
        if wizard():
            game.saved = -1
            return False
        if game.saved != -1:
            mspeak(9)
            sys.exit()
        if yesm(5, 7, 7):
            game.saved = -1
            return True
        sys.exit()
    game.saved = -1
    return False

def maint():  ### MAGIC
    if not wizard():
        return
    if yesm(10, 0, 0, blklin=False):
        hours()
    if yesm(11, 0, 0, blklin=False):
        newhrs()
    if yesm(26, 0, 0, blklin=False):
        mspeak(27, blklin=False)
        magic.hbegin = getInt()
        mspeak(28, blklin=False)
        magic.hend = getInt()
        (d,t) = datime()
        magic.hbegin += d
        magic.hend += magic.hbegin - 1
        mspeak(29, blklin=False)
        magic.hname = raw_input('> ')[:20]
    print('Length of short game (null to leave at %d):' % (magic.short,))
    x = getInt()
    if x > 0:
        magic.short = x
    mspeak(12, blklin=False)
    word = getin()[0]
    if word is not None:
        magic.magic = word
    mspeak(13, blklin=False)
    x = getInt()
    if x > 0:
        magic.magnm = x
    print('Latency for restart (null to leave at %d):' % (magic.latency,))
    x = getInt()
    if 0 < x < 45:
        mspeak(30, blklin=False)
    if x > 0:
        magic.latency = max(45, x)
    if yesm(14, 0, 0):
        motd(True)
    mspeak(15, blklin=False)
    with open(magicfile, 'w') as fp:
        pickle.dump(fp, magic)
    ciao()

def wizard():  ### MAGIC
    if not yesm(16, 0, 7):
        return False
    mspeak(17)
    word = getin()[0]
    if word != magic.magic:
        mspeak(20)
        return False
    (d,t) = datime
    t = t*2 + 1
    wchrs = [64] * 5
    val = []
    for y in xrange(5):
        x = 79 + d % 5
        d //= 5
        for _ in xrange(x):
            t = (t * 1027) % 1048576
        val.append((t*26) // 1048576 + 1)
        wchrs[y] += val[-1]
    if yesm(18, 0, 0):
        mspeak(20)
        return False
    print('\n' + ''.join(map(chr, wchrs)))
    wchrs = list(map(ord, getin()[0]))
    # What happens if the inputted word is less than five characters?
    (d,t) = datime()
    t = (t // 60) * 40 + (t // 10) * 10
    d = magic.magnm
    for y in xrange(5):
        wchrs[y] -= (abs(val[y] - val[(y+1) % 5]) * (d % 10) + t % 10) % 26 + 1
        t //= 10
        d //= 10
    if all(c == 64 for c in wchrs):
        mspeak(19)
        return True
    else:
        mspeak(20)
        return False

def hours():  ### MAGIC
    print()
    hoursx(magic.wkday, 'Mon - Fri:')
    hoursx(magic.wkend, 'Sat - Sun:')
    hoursx(magic.holid, 'Holidays: ')
    (d,_) = datime()
    if magic.hend < d or magic.hend < magic.hbegin:
        return
    if magic.hbegin > d:
        d = magic.hbegin - d
        print('\nThe next holiday will be in %d day%s, namely %s.'
              % (d, '' if d == 1 else 's', magic.hname))
    else:
        print('\nToday is a holiday, namely ' + magic.hname + '.')

def hoursx(horae, day):  ### MAGIC
    if not any(horae):
        print(' ' * 10, day, '  Open all day', sep='')
    else:
        first = True
        from = 0
        while True:
            while from < 24 and horae[from]:
                from += 1
            if from >= 24:
                if first:
                    print(' ' * 10, day, '  Closed all day', sep='')
                break
            else:
                till = from + 1
                while till < 24 and not horae[till]:
                    till += 1
                if first:
                    print(' ' * 10, day, '%4d:00 to%3d:00' % (from, till),
                          sep='')
                else:
                    print(' ' * 20, '%4d:00 to%3d:00' % (from, till), sep='')
             first = False
             from = till

def newhrs():  ### MAGIC
    mspeak(21)
    magic.wkday = newhrx('weekdays:')
    magic.wkend = newhrx('weekends:')
    magic.holid = newhrx('holidays:')
    mspeak(22)
    hours()

def newhrx(day):  ### MAGIC
    horae = [False] * 24
    print('Prime time on', day)
    while True:
        from = getInt('from: ')
        if not (0 <= from < 24):
            return horae
        till = getInt('till: ')
        if not (from <= till-1 < 24):
            return horae
        horae[from:till] = [True] * (till - from)

def motd(alter):  ### MAGIC
    if alter:
        mspeak(23)
        magic.msg = ''
        # This doesn't exactly match the logic used in the original Fortran,
        # but it's close:
        while len(magic.msg) < 430:
            next = raw_input('> ')
            if not next:
                return
            if len(next) > 70:
                mspeak(24)
                continue
            magic.msg += next + '\n'
        mspeak(25)
    elif magic.msg:
        print(magic.msg, end='')

def poof(mfile):  ### MAGIC
    global magic
    if mfile is None:
        try:
            mfile = open(magicfile, 'rb')
        except IOError as e:
            if e.errno == ENOENT:
                magic = Magic()
            else:
                raise
    with mfile:
        magic = load(mfile, Magic)

def main():
    global cave, game, demo, ran
    parser = argparse.ArgumentParser()
    parser.add_argument('-D', '--data-file', type=argparse.FileType('r'))
    ###parser.add_argument('-m', '--magic', action='store_true')
    parser.add_argument('-M', '--magic-file', type=argparse.FileType('rb'))
    parser.add_argument('-R', '--orig-rng', action='store_true')
    parser.add_argument('savedgame', type=argparse.FileType('rb'))
    args = parser.parse_args()
    goto = label2
    if args.orig_rng:
        ran(1)
    else:
        from random import randrange
        ran = randrange
    with (args.data_file or open('advent.dat')) as advdat:
        cave = Adventure(advdat)
    game = Game()
    if MAGIC:
        poof(args.magic_file)
    if args.savedgame is not None:
        with args.savedgame:
            goto = resume(args.savedgame.name, args.savedgame)
    else:
        if MAGIC:
            demo = start()
            motd(False)
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
                for i in xrange(5)):
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
        for i in xrange(5):
            if game.dloc[i] == game.loc:
                game.dloc[i] = 18
            game.odloc[i] = game.dloc[i]
        rspeak(3)
        game.drop(Item.AXE, game.loc)
        return label2000
    dtotal, attack, stick = 0, 0, 0
    for i in xrange(6):  # The individual dwarven movement loop
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
            game.dloc[i] = game.loc;
            if i == 5:
                # Pirate logic:
                if game.loc == CHLOC or game.prop[Item.CHEST] >= 0:
                    continue
                k = False
                stole = False
                for j in xrange(50, 65):
                    if j == Item.PYRAM and game.loc in (100, 101):
                        continue
                    if game.toting(j):
                        rspeak(128)
                        if game.place[Item.MESSAG] == 0:
                            game.move(Item.CHEST, CHLOC)
                        game.move(Item.MESSAG, CHLOC2)
                        for j2 in xrange(50, 65):
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
    game.odloc2 = game.loc
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
            game.odloc2 = game.loc
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
    global word1, in1, word2, in2
    for hint in xrange(4, 10):
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
                    hint == 7 and (any(game.atloc[l] for l in (game.loc, game.oldloc, game.oldloc2)) or game.holding <= 1) or \
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
        for i in xrange(1, 65):
            if game.toting(i) and game.prop[i] < 0:
                game.prop[i] = -1 - game.prop[i]
    # Label 2605
    game.wzdark = game.dark()
    if 0 < game.knifeloc != game.loc:
        game.knifeloc = 0
    (word1, in1, word2, in2) = getin()
    return label2608

def label2608():
    global verb
    game.foobar = min(0, game.foobar)
    if MAGIC and game.turns == 0 and (word1, word2) == ('MAGIC', 'MODE'):
        maint()
    game.turns += 1
    if MAGIC and demo and game.turns >= magic.short:
        mspeak(1)
        normend()
    if verb == Action.SAY:
        if word2:
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
        for i in xrange(1, 65):
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
    global word1, in1, word2, in2
    if word1 == 'ENTER' and word2 in ('STREA', 'WATER'):
        rspeak(70 if cave.liqloc(game.loc) == Item.WATER else 43)
        return label2012
    elif word1 == 'ENTER' and word2:
        (word1, in1) = (word2, in2)
        word2 = in2 = None
    elif word1 in ('WATER', 'OIL') and word2 in ('PLANT', 'DOOR') and \
            game.at(cave.vocab(word2, 1)):
        word2 = 'POUR'
    return label2610

def label2610():
    if word1 == 'WEST':
        game.iwest += 1
        if game.iwest == 10:
            rspeak(17)
    return label2630

def label2630():
    global obj, verb
    global word1, in1, word2, in2
    i = cave.vocab(word1, -1)
    if i == -1:
        rspeak(61 if pct(20) else 13 if pct(20) else 60)
        return label2600
    k = i % 1000
    if i // 1000 == 0:
        return domove(k)
    elif i // 1000 == 1:
        # Label 5000
        obj = k
        if game.fixed[obj] == game.loc or game.here(obj):
            return doaction()
        elif obj == Item.GRATE:
            if game.loc in (1, 4, 7):
                return domove(Movement.DEPRESSION)
            elif 9 < game.loc < 15:
                return domove(Movement.ENTRANCE)
            elif verb in (Action.FIND, Action.INVENT) and not word2:
                return doaction()
            else:
                print('\nI see no', in1, 'here.')
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
        elif verb in (Action.FIND, Action.INVENT) and not word2:
            return doaction()
        else:
            print('\nI see no', in1, 'here.')
            return label2012
    elif i // 1000 == 2:
        # Label 4000
        verb = k
        if verb in (Action.SAY, Action.SUSPEND, Action.RESUME):
            obj = word2 is not None
            # This assignment just indicates whether an object was supplied.
        elif word2:
            (word1, in1) = (word2, in2)
            word2 = in2 = None
            return label2610
        if obj:
            return transitive()
        else:
            return intransitive()
    elif i // 1000 == 3:
        rspeak(k)
        return label2012
    else:
        bug(22)


# Verb functions:

def what():
    global obj
    print()
    print(in1, 'what?')
    obj = 0
    return label2600

def actspk():
    rspeak(cave.actspk[verb])

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
    Action.SUSPEND: lambda: vsuspend(savefile),
    Action.RESUME: lambda: vresume(savefile),
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
    Action.SUSPEND: lambda: vsuspend(in2),
    Action.RESUME: lambda: vresume(in2),
}

def transitive():
    # Label 4090 (transitive verb handling)
    return tverbs.get(verb, lambda: bug(24))() or label2012

def vscore():
    score = game.score(True)
    print()
    print('If you were to quit now, you would score', score,
          'out of a possible 350.')
    game.gaveup = yes(143, 54, 54)
    if game.gaveup:
        normend()

def vfoo():
    k = cave.vocab(word1, 3)
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
            k = 0 if game.loc == 92 else 1 if game.here(Item.EGGS) else 2
            game.move(Item.EGGS, 92)
            pspeak(Item.EGGS, k)
    else:
        rspeak(151 if game.foobar else 42)

def vbrief():
    game.abbnum = 10000
    game.detail = 3
    rspeak(156)

def vhours():
    if MAGIC:
        mspeak(6)
        hours()
    else:
        print()
        print('Colossal Cave is open all day, every day.')

def vquit():
    game.gaveup = yes(22, 54, 54)
    if game.gaveup:
        normend()

def vinvent():
    spk = 98
    for i in xrange(65):
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
        ixs = [i for i in xrange(5) if game.dloc[i] == game.loc]
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
            game.fixed[Item.AXE] = -1
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
            obj == game.liqloc(game.loc):
        rspeak(94)
    else:
        actspk()

def vbreak():
    # Label 9280
    if obj == Item.VASE and game.prop[Item.VASE] == 0:
        if game.toting(Item.VASE):
            game.drop(Item.VASE, game.loc)
        game.prop[Item.VASE] = 2
        game.fixed[Item.VASE] = -1
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
        game.place[k] = -1
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
                game.fixed[Item.CHAIN] = -1
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
            spk = 130;
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
        print('I see no', in1, 'here.')
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
    global verb, obj
    global word1, in1, word2, in2
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
            (word1, in1, word2, in2) = getin()
            if word1 not in ('YES', 'Y'):
                return label2608
            pspeak(Item.DRAGON, 1)
            game.prop[Item.DRAGON] = 2
            game.prop[Item.RUG] = 0
            game.move(Item.DRAGON+100, -1)
            game.move(Item.RUG+100, 0)
            game.move(Item.DRAGON, 120)
            game.move(Item.RUG, 120)
            for i in xrange(65):
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
            if obj != WATER:
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
            game.fixed[Item.VASE] = -1
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
            game.place[game.liq()] = -1
        rspeak(108 if game.liq() == Item.OIL else 107)

def vblast():
    # Label 9230
    global bonus
    if game.prop[Item.ROD2] < 0 or not game.closed:
        actspk()
    else:
        bonus = 133
        if game.loc == 115:
            bonus = 134
        if game.here(Item.ROD2):
            bonus = 135
        rspeak(bonus)
        normend()
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
    if not game.here(Item.LAMP)
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
        pspeak(Item.VASE, game.prop[VASE] + 1)
        if game.prop[Item.VASE] != 0:
            game.fixed[Item.VASE] = -1
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
    global word1, word2, obj
    tk = in2 if in2 is not None else in1
    word1 = word2 if word2 is not None else word1
    if cave.vocab(word1, -1) in (62, 65, 71, 2025):
        word2 = None
        obj = 0
        return label2630
    else:
        print()
        print('Okay, "' + tk + '".')

def vsuspend(filename):
    if MAGIC:
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
    if MAGIC:
        (game.saved, game.savet) = datime()
    print()
    print('Saving to', filename, '...')
    try:
        with open(filename, 'w') as fp:
            pickle.dump(fp, game)
    except Exception as e:
        traceback.print_exc()
    else:
        if MAGIC:
            ciao()
        sys.exit()

def vresume(filename):
    if MAGIC and demo:
        mspeak(9)
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
    if MAGIC:
        start()
    return lambda: domove(Movement.NULL)
