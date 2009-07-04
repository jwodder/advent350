# Verb functions:

sub intransitive() {
# Label 4080 (intransitive verb handling):

# As this function is only called at a single point in the code, it doesn't
# actually need to be a separate routine, but I think we can all agree that
# MAIN is complicated enough as it is without stuffing yet another 120-line
# "given" block inside of it.

 $goto = 2012;
 given $verb {
  when NOTHING { rspeak 54 }
  when WALK { rspeak @actspk[$verb] }
  when DROP | SAY | WAVE | CALM | RUB | THROW | FIND | FEED | BREAK | WAKE {
# 8000:
   say "$in1 what?";
   $obj = 0;
   $goto = 2600;
  }
  when TAKE {
   if @atloc[$loc] != 1 || $dflag >= 2 && @dloc[^5].any == $loc {
    say "$in1 what?";
    $obj = 0;
    $goto = 2600;
   } else {
    $obj = @atloc[$loc;0];
    vtake;
   }
  }
  when OPEN | LOCK {
   $obj = CLAM if here CLAM;
   $obj = OYSTER if here OYSTER;
   $obj = DOOR if at DOOR;
   $obj = GRATE if at GRATE;
   if $obj != 0 && here CHAIN {
    say "$in1 what?";
    $obj = 0;
    $goto = 2600;
   } else {
    $obj = CHAIN if here CHAIN;
    if $obj == 0 { rspeak 28 }
    else { vopen }
   }
  }
  when EAT {
   if here FOOD {
    destroy FOOD;
    rspeak 72;
   } else {
    say "$in1 what?";
    $obj = 0;
    $goto = 2600;
   }
  }
  # Yes, this is supposed to be an assignment:
  when QUIT { normend if $gaveup = yes(22, 54, 54) }
  when INVENT {
   my int $spk = 98;
   for ^65 -> $i {
    next if $i == BEAR || !toting $i;
    rspeak 99 if $spk == 98;
    $blklin = False;
    pspeak $i, -1;
    $blklin = True;
    $spk = 0;
   }
   $spk = 141 if toting BEAR;
   rspeak $spk;
  }
  when SCORE {
   my int $score = score(True);
   say "If you were to quit now, you would score $score of a possible 350.";
   normend if $gaveup = yes(143, 54, 54);
  }
  when FOO {
   my $k = vocab $word1, 3;
   if $foobar == 1-$k {
    $foobar = $k;
    if $k != 4 {rspeak 54; return; }
    $foobar = 0;
    if @place[EGGS] == 92 || toting(EGGS) && $loc == 92 { rspeak 42 }
    else {
     @prop[TROLL] = 1 if @place[EGGS] & @place[TROLL] & @prop[TROLL] == 0;
     $k = $loc == 92 ?? 0 !! here(EGGS) ?? 1 !! 2;
     move EGGS, 92;
     pspeak EGGS, $k;
    }
   } else { rspeak($foobar ?? 151 !! 42) }
  }
  when BRIEF {
   $abbnum = 10000;
   $detail = 3;
   rspeak 156;
  }
  when READ {
   $obj = MAGZIN if here MAGZIN;
   $obj = $obj * 100 + TABLET if here TABLET;
   $obj = $obj * 100 + MESSAG if here MESSAG;
   $obj = OYSTER if $closed && toting OYSTER;
   if $obj > 100 || $obj == 0 || dark {
    say "$in1 what?"; $obj = 0; $goto = 2600;
   } else { vread }
  }
  when SUSPEND { vsuspend("%*ENV<HOME>/.adventure") }
  when RESUME { vresume("%*ENV<HOME>/.adventure") }
  when HOURS {
#ifdef ADVMAGIC
   mspeak(6); hours();
#else
   printf("Colossal Cave is open all day, every day.\n");
#endif
  }
  when ON { von }
  when OFF { voff }
  when KILL { vkill }
  when POUR { vpour }
  when DRINK { vdrink }
  when FILL { vfill }
  when BLAST { vblast }
  default { bug 23 }
 }
}

sub transitive() {
# Label 4090 (transitive verb handling):
 $goto = 2012;
 given $verb {
  when TAKE { vtake }
  when DROP { vdrop }
  when SAY { vsay }
  when OPEN | LOCK { vopen }
  when NOTHING { rspeak 54 }
  when ON { von }
  when OFF { voff }
  when WAVE {
# 9090:
   if !toting($obj) && !($obj == ROD && toting(ROD2)) { rspeak 29 }
   elsif $obj != ROD || !at(FISSUR) || !toting($obj) || $closing {
    rspeak @actspk[$verb]
   } else {
    @prop[FISSUR] = 1 - @prop[FISSUR];
    pspeak FISSUR, 2 - @prop[FISSUR];
   }
  }
  when CALM | WALK | QUIT | SCORE | FOO | BRIEF | HOURS {
   rspeak @actspk[$verb]
  }
  when KILL { vkill }
  when POUR { vpour }
  when EAT {
# 9140:
   if $obj == FOOD {destroy FOOD; rspeak 72; }
   elsif $obj == BIRD | SNAKE | CLAM | OYSTER | DWARF | DRAGON | TROLL | BEAR {
    rspeak 71
   } else { rspeak @actspk[$verb] }
  }
  when DRINK { vdrink }
  when RUB { rspeak($obj == LAMP ?? @actspk[$verb] !! 76) }
  when THROW {
# 9170:
   $obj = ROD2 if toting(ROD2) && $obj == ROD && !toting(ROD);
   if !toting $obj { rspeak @actspk[$verb] }
   elsif 50 <= $obj < 65 && at(TROLL) {
    drop $obj, 0;
    move TROLL, 0;
    move TROLL+100, 0;
    drop TROLL2, 117;
    drop TROLL2+100, 122;
    juggle CHASM;
    rspeak 159;
   } elsif $obj == FOOD && here BEAR {$obj = BEAR; vfeed; }
   elsif $obj == AXE {
    my int $i = (^5).first({ @dloc[$_] == $loc }) // -1;
    if $i != -1 {
     if (^3).pick == 0 { rspeak 48 }
     else {
      @dseen[$i] = False;
      @dloc[$i] = 0;
      rspeak(++$dkill == 1 ?? 149 !! 47);
     }
     drop AXE, $loc;
     domove NULL;
    } elsif at(DRAGON) && @prop[DRAGON] == 0 {
     rspeak 152;
     drop AXE, $loc;
     domove NULL;
    } elsif at(TROLL) {
     rspeak 158;
     drop AXE, $loc;
     domove NULL;
    } elsif here(BEAR) && @prop[BEAR] == 0 {
     drop AXE, $loc;
     @fixed[AXE] = -1;
     @prop[AXE] = 1;
     juggle BEAR;  # Don't try this at home, kids.
     rspeak 164;
    } else {$obj = 0; vkill; }
   } else { vdrop }
  }
  when FIND | INVENT {
# 9190:
   if toting $obj { rspeak 24 }
   elsif $closed { rspeak 138 }
   elsif $obj == DWARF && $dflag >= 2 && @dloc[^5].any == $loc { rspeak 94 }
   elsif at($obj) || (liq == $obj && at(BOTTLE)) || $obj == liqloc($loc) {
    rspeak 94
   } else { rspeak @actspk[$verb] }
  }
  when FEED { vfeed }
  when FILL { vfill }
  when BLAST { vblast }
  when READ { vread }
  when BREAK {
# 9280:
   if $obj == VASE && @prop[VASE] == 0 {
    drop VASE, $loc if toting VASE;
    @prop[VASE] = 2;
    @fixed[VASE] = -1;
    rspeak 198;
   } elsif $obj != MIRROR { rspeak @actspk[$verb] }
   elsif !$closed { rspeak 148 }
   else {rspeak 197; rspeak 136; normend; }
  }
  when WAKE {
# 9290:
   if $obj == DWARF && $closed {rspeak 199; rspeak 136; normend; }
   else { rspeak @actspk[$verb] }
  }
  when SUSPEND { vsuspend($in2) }
  when RESUME { vresume($in2) }
  default { bug 24 }
 }
}

sub vtake() {
# 9010:
 if toting $obj {rspeak @actspk[$verb]; return; }
 my int $spk = 25;
 $spk = 115 if $obj == PLANT && @prop[PLANT] <= 0;
 $spk = 169 if $obj == BEAR && @prop[BEAR] == 1;
 $spk = 170 if $obj == CHAIN && @prop[BEAR] != 0;
 if fixed $obj {rspeak $spk; return; }
 if $obj == WATER | OIL {
  if !here(BOTTLE) || liq != $obj {
   $obj = BOTTLE;
   if toting(BOTTLE) && @prop[BOTTLE] == 1 { vfill }
   else {
    $spk = 105 if @prop[BOTTLE] != 1;
    $spk = 104 if !toting BOTTLE;
    rspeak $spk;
   }
   return;
  }
  $obj = BOTTLE;
 }
 if $holding >= 7 {rspeak 92; return; }
 if $obj == BIRD && @prop[BIRD] == 0 {
  if toting ROD {rspeak 26; return; }
  if !toting CAGE {rspeak 27; return; }
  @prop[BIRD] = 1;
 }
 carry BIRD+CAGE-$obj, $loc if $obj == BIRD | CAGE && @prop[BIRD] != 0;
 carry $obj, $loc;
 $k = liq;
 @place[$k] = -1 if $obj == BOTTLE && $k != 0;
 rspeak 54;
}

sub vopen() {
# 9040:
 my int $spk = @actspk[$verb];
 given $obj {
  when CLAM | OYSTER {
   my $k = ($obj == OYSTER);
   $spk = 124 + $k;
   $spk = 120 + $k if toting OBJ;
   $spk = 122 + $k if !toting TRIDENT;
   $spk = 61 if $verb == LOCK;
   if $spk == 124 {
    destroy CLAM;
    drop OYSTER, $loc;
    drop PEARL, 105;
   }
  }
  when DOOR { $spk = @prop[DOOR] == 1 ?? 54 !! 111 }
  when CAGE { $spk = 32 }
  when KEYS { $spk = 55 }
  when CHAIN {
   if !here KEYS { $spk = 31 }
   elsif $verb == LOCK {
    $spk = 172;
    $spk = 34 if @prop[CHAIN] != 0;
    $spk = 173 if $loc != 130;
    if $spk == 172 {
     @prop[CHAIN] = 2;
     drop CHAIN, $loc if toting CHAIN;
     @fixed[CHAIN] = -1;
    }
   } else {
    $spk = 171;
    $spk = 41 if @prop[BEAR] == 0;
    $spk = 37 if @prop[CHAIN] == 0;
    if $spk == 171 {
     @prop[CHAIN] = @fixed[CHAIN] = 0;
     @prop[BEAR] = 2 if @prop[BEAR] != 3;
     @fixed[BEAR] = 2 - @prop[BEAR];
    }
   }
  }
  when GRATE {
   if !here KEYS { $spk = 31 }
   elsif $closing {
    $spk = 130;
    $clock2 = 15 if !$panic;
    $panic = True;
   } else {
    $spk = 34 + @prop[GRATE];
    @prop[GRATE] = ($verb != LOCK);
    $spk += 2 * @prop[GRATE];
   }
  }
 }
 rspeak $spk;
}

sub vread() {
# 9270:
 if dark() { say "I see no $in1 here." }
 else {
  my int $spk = @actspk[$verb];
  $spk = 190 if $obj == MAGZIN;
  $spk = 196 if $obj == TABLET;
  $spk = 191 if $obj == MESSAG;
  $spk = 194 if $obj == OYSTER && @hinted[2] && toting OYSTER;
  if $obj != OYSTER || @hinted[2] || !toting(OYSTER) || !$closed { rspeak $spk }
  else { @hinted[2] = yes(192, 193, 54) }
 }
}

sub vkill() {
# 9120:
 if $obj == 0 {
  $obj = DWARF if $dflag >= 2 && @dloc[^5].any == $loc;
  $obj = $obj * 100 + SNAKE if here SNAKE;
  $obj = $obj * 100 + DRAGON if at(DRAGON) && @prop[DRAGON] == 0;
  $obj = $obj * 100 + TROLL if at TROLL;
  $obj = $obj * 100 + BEAR if here(BEAR) && @prop[BEAR] == 0;
  if $obj > 100 {say "$in1 what?"; $obj = 0; $goto = 2600; return; }
  if $obj == 0 {
   $obj = BIRD if here(BIRD) && $verb != THROW;
   $obj = $obj * 100 + CLAM if here(CLAM | OYSTER);
   if $obj > 100 {say "$in1 what?"; $obj = 0; $goto = 2600; return; }
  }
 }
 given $obj {
  when BIRD {
   if $closed { rspeak 137 }
   else {
    destroy BIRD;
    @prop[BIRD] = 0;
    $tally2++ if @place[SNAKE] == 19;
    rspeak 45;
   }
  }
  when 0 { rspeak 44 }
  when CLAM | OYSTER { rspeak 150 }
  when SNAKE { rspeak 46 }
  when DWARF {
   if $closed {rspeak 136; normend; }
   else { rspeak 49 }
  }
  when DRAGON {
   if @prop[DRAGON] != 0 { rspeak 167 }
   else {
    rspeak 49;
    ($verb, $obj) = (0, 0);
    ($word1, $in1, $word2, $in2) = getin;
    if !($word1 eq 'YES' | 'Y') {$goto = 2608; return; }
    pspeak DRAGON, 1;
    @prop[DRAGON, RUG] = 2, 0;
    move DRAGON+100, -1;
    move RUG+100, 0;
    move DRAGON, 120;
    move RUG, 120;
    move $_, 120 for grep { @place[$_] == 119 | 121 }, ^65;
    $loc = 120;
    domove NULL;
   }
  }
  when TROLL { rspeak 157 }
  when BEAR { rspeak(165 + (@prop[BEAR]+1) idiv 2) }
 }
}

sub vpour() {
# 9130:
 $obj = liq if $obj == BOTTLE | 0;
 if $obj == 0 {say "$in1 what?"; $obj = 0; $goto = 2600; }
 elsif !toting $obj { rspeak @actspk[$verb] }
 elsif !($obj == OIL | WATER) { rspeak 78 }
 else {
  @prop[BOTTLE] = 1;
  @place[OBJ] = 0;
  if at DOOR {
   @prop[DOOR] = ($obj == OIL);
   rspeak 113 + @prop[DOOR];
  } elsif at PLANT {
   if $obj != WATER { rspeak 112 }
   else {
    pspeak PLANT, @prop[PLANT] + 1;
    @prop[PLANT] = (@prop[PLANT] + 2) % 6;
    @prop[PLANT2] = @prop[PLANT] idiv 2;
    domove NULL;
   }
  } else { rspeak 77 }
 }
}

sub vdrink() {
# 9150:
 if $obj == 0 && liqloc($loc) != WATER && (liq != WATER || !here BOTTLE) {
  say "$in1 what?";
  $obj = 0;
  $goto = 2600;
 } elsif $obj == 0 | WATER {
  if liq == WATER && here BOTTLE {
   @prop[BOTTLE] = 1;
   @place[WATER] = 0;
   rspeak 74;
  } else { rspeak @actspk[$verb] }
 } else { rspeak 110 }
}

sub vfill() {
# 9220:
 if $obj == VASE {
  if liqloc($loc) == 0 { rspeak 144 }
  elsif !toting VASE { rspeak 29 }
  else {
   rspeak 145;
   @prop[VASE] = 2;
   @fixed[VASE] = -1;
# In the original Fortran, when the vase is filled with water or oil, its
# property is set so that it breaks into pieces, *but* the code then branches
# to label 9024 to actually drop the vase.  Once you cut out the unreachable
# states, it turns out that the vase remains intact if the pillow is present,
# but even if it survives it is still marked as a fixed object and can't be
# picked up again.  This is probably a bug in the original code, but who am I
# to fix it?
   @prop[VASE] = 0 if at PILLOW;
   pspeak VASE, @prop[VASE] + 1;
   drop $obj, $loc;
  }
 } else {
  if !($obj == 0 | BOTTLE) { rspeak @actspk[$verb] }
  elsif $obj == 0 && !here BOTTLE {
   say "$in1 what?";
   $obj = 0;
   $goto = 2600;
  } elsif liq != 0 { rspeak 105 }
  elsif liqloc($loc) == 0 { rspeak 106 }
  else {
   @prop[BOTTLE] = @cond[$loc] +& 2;
   @place[liq] = -1 if toting BOTTLE;
   rspeak(liq == OIL ?? 108 !! 107);
  }
 }
}

sub vblast() {
# 9230:
 if @prop[ROD2] < 0 || !$closed { rspeak @actspk[$verb] }
 else {
  $bonus = 133;
  $bonus = 134 if $loc == 115;
  $bonus = 135 if here ROD2;
  rspeak $bonus;
  normend;
  # Fin
 }
}

sub von() {
# 9070:
 if !here LAMP { rspeak @actspk[$verb] }
 elsif $limit < 0 { rspeak 184 }
 else {
  @prop[LAMP] = 1;
  rspeak 39;
  $goto = 2000 if $wzdark;
 }
}

sub voff() {
# 9080:
 if !here LAMP { rspeak @actspk[$verb] }
 else {
  @prop[LAMP] = 0;
  rspeak 40;
  rspeak 16 if dark;
 }
}

sub vdrop() {
# 9020:
 $obj = ROD2 if toting(ROD2) && $obj == ROD && !toting ROD;
 if !toting $obj {rspeak @actspk[$verb]; return; }
 if $obj == BIRD && here SNAKE {
  rspeak 30;
  if $closed {rspeak 136; normend; }
  destroy SNAKE;
  @prop[SNAKE] = 1;
 } elsif $obj == COINS && here VEND {
  destroy COINS;
  drop BATTER, $loc;
  pspeak BATTER, 0;
  return;
 } elsif $obj == BIRD && at(DRAGON) && @prop[DRAGON] == 0 {
  rspeak 154;
  destroy BIRD;
  @prop[BIRD] = 0;
  $tally2++ if @place[SNAKE] == 19;
  return;
 } elsif $obj == BEAR && at(TROLL) {
  rspeak 163;
  move TROLL, 0;
  move TROLL+100, 0;
  move TROLL2, 117;
  move TROLL2+100, 122;
  juggle CHASM;
  @prop[TROLL] = 2;
 } elsif $obj == VASE && $loc != 96 {
  @prop[VASE] = at(PILLOW) ?? 0 !! 2;
  pspeak VASE, @prop[VASE] + 1;
  @fixed[VASE] = -1 if @prop[VASE] != 0;
 } else { rspeak 54 }
 my int $k = liq;
 $obj = BOTTLE if $k == $obj;
 @place[$k] = 0 if $obj == BOTTLE && $k != 0;
 drop BIRD, $loc if $obj == CAGE && @prop[BIRD] != 0;
 @prop[BIRD] = 0 if $obj == BIRD;
 drop $obj, $loc;
}

sub vfeed() {
# 9210:
 given $obj {
  when BIRD { rspeak 100 }
  when SNAKE {
   if !$closed && here BIRD {
    destroy BIRD;
    @prop[BIRD] = 0;
    $tally2++;
    rspeak 101;
   } else { rspeak 102 }
  }
  when TROLL { rspeak 182 }
  when DRAGON { rspeak(@prop[DRAGON] != 0 ?? 110 !! 102) }
  when DWARF {
   if !here FOOD { rspeak @actspk[$verb] }
   else {$dflag++; rspeak 103; }
  }
  when BEAR {
   if !here FOOD { rspeak (102, @actspk[$verb] xx 2, 110)[@prop[BEAR]] }
   else {
    destroy FOOD;
    @prop[BEAR] = 1;
    @fixed[AXE] = 0;
    @prop[AXE] = 0;
    rspeak 168;
   }
  }
  default { rspeak 14 }
 }
}

sub vsay() {
# 9030:
 my Str $tk = $in2 // $in1;
 $word1 = $word2 // $word1;
 if vocab($word1, -1) == 62 | 65 | 71 | 2025 {
  $word2 = undef;
  $obj = 0;
  $goto = 2630;
 } else { say "Okay, \"$tk\"." }
}


# Below are the routines for saving & restoring games.  All user data is
# written out in binary form in the order that the variables are declared.
# @atloc is stored by preceding each sub-array by the number of elements within
# it.  As pack() and unpack() have not been fully specified for Perl 6 yet (and
# thus certainly won't be available in Rakudo for a while), the data is written
# & read using homemade routines.

sub writeInt(IO $out, int32 $i) {
 $out.write(Buf.new($i, size => 32), 4)
}

sub writeBool(IO $out, bool *@bits) {
 my Int $x = 0;
 $x +|= 1 +< $_ if @bits[$_] for ^@bits;
 #for @bits.kv -> $k, $v { $x +|= 1 +< $k if $v }
 my Buf $blob .= new($x);
 $out.write($blob, $blob.bytes #< ??? > );
}

sub readInt(IO $in --> int32) {
 !!!
}

sub readBool(IO $in, int $qty --> List of bool) {
 !!!
}

sub vsuspend(Str $file) {
#ifdef ADVMAGIC
 if $demo {rspeak 201; return; }
 say "I can suspend your adventure for you so that you can resume later, but";
 say "you will have to wait at least $latency minutes before continuing.";
#else
 say "I can suspend your adventure for you so that you can resume later.";
#endif
 return if !yes(200, 54, 54);
#ifdef ADVMAGIC
 datime(&saved, &savet);
#endif
 say "\nSaving to $file ...";

 my IO $adv = open $file, :w, :bin;
 # What exactly happens if the file fails to open?

 writeInt $adv, $_ for $loc, $newloc, $oldloc, $oldloc2, $limit, $turns,
  $iwest, $knifeloc, $detail, $numdie, $holding, $foobar, $bonus, $tally,
  $tally2, $abbnum, $clock1, $clock2;
 writeBool $adv, $wzdark, $closing, $lmwarn, $panic, $closed, $gaveup;
 writeInt $adv, $_ for @prop, @abb, @hintlc;
 writeBool $adv, @hinted;
 writeInt $adv, $_ for @dloc, @odloc;
 writeBool $adv, @dseen;
 writeInt $adv, $_ for $dflag, $dkill, @place, @fixed;
 for @atloc {
  writeInt $adv, $_.elems;
  writeInt $adv, $_ for @($_);
 }
#ifdef ADVMAGIC
 writeInt $adv, $saved;
 writeInt $adv, $savet;
 
 fclose(adv);
 ciao();
#else
 fclose(adv);
 exit(0);
#endif
}

sub vresume(Str $file) {
 if $turns != 0 {
  say "To resume an earlier Adventure, you must abandon the current one.";
  # This message is taken from the 430 pt. version of Adventure (version 2.5).
  return if !yes(200, 54, 54);
 }
 say "\nRestoring from $file ...";

 my IO $adv = open $file, :r, :bin;
 $loc = readInt $adv;
 $newloc = readInt $adv;
 $oldloc = readInt $adv;
 $oldloc2 = readInt $adv;
 $limit = readInt $adv;
 $turns = readInt $adv;
 $iwest = readInt $adv;
 $knifeloc = readInt $adv;
 $detail = readInt $adv;
 $numdie = readInt $adv;
 $holding = readInt $adv;
 $foobar = readInt $adv;
 $bonus = readInt $adv;
 $tally = readInt $adv;
 $tally2 = readInt $adv;
 $abbnum = readInt $adv;
 $clock1 = readInt $adv;
 $clock2 = readInt $adv;
 ($wzdark, $closing, $lmwarn, $panic, $closed, $gaveup) = readBool $adv, 6;
 # Doesn't Perl 6 have some built-in way to initialize an array of known length
 # with the return values from calling a function repeatedly?  I thought I saw
 # something like that somewhere in the Synopses.
 @prop[$_] = readInt $adv for ^@prop;
 @abb[$_] = readInt $adv for ^@abb;
 @hintlc[$_] = readInt $adv for ^@hintlc;
 @hinted = readBool $adv, +@hinted;
 @dloc[$_] = readInt $adv for ^@dloc;
 @odloc[$_] = readInt $adv for ^@odloc;
 @dseen = readBool $adv, +@dseen;
 $dflag = readInt $adv;
 $dkill = readInt $adv;
 @place[$_] = readInt $adv for ^@place;
 @fixed[$_] = readInt $adv for ^@fixed;
 for ^@atloc -> $i {
  my int $qty = readInt $adv;
  @atloc[$i;$_] = readInt $adv for ^$qty;
 }
#ifdef ADVMAGIC
 # If the user attempts to restart a non-magic game with the magic version of
 # this program, just assume that $saved is 0.  (Yes, I know an int32 can't be
 # undefined.  This is just a placeholder/reminder until I actually add in IO
 # error checking.)
 $saved = readInt($adv) // 0;
 $savet = readInt($adv) // 0;

 start();
#endif
 domove NULL;
}
