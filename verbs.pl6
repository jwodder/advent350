# NOTE: When all of these verb routines are called, $spk is always (as far as I
# know) set to @actspk[$verb].


# Label 4080 (intransitive verb handling):
 given $verb {
  when NOTHING {rspeak 54; #< GOTO 2012 > }
  when WALK {rspeak @actspk[$verb]; #< GOTO 2012 > }
  when DROP | SAY | WAVE | CALM | RUB | THROW | FIND | FEED | BREAK | WAKE {
# 8000:
   say "$in1 what?";
   $obj = 0;
   #< GOTO 2600 >
  }
  when TAKE {
   if @atloc[$loc] != 1 || $dflag >= 2 && @dloc[^5].any == $loc {
    say "$in1 what?";
    $obj = 0;
    #< GOTO 2600 >
   }
   $obj = @atloc[$loc;0];
   vtake;
  }
  when OPEN | LOCK {
   $obj = CLAM if here CLAM;
   $obj = OYSTER if here OYSTER;
   $obj = DOOR if at DOOR;
   $obj = GRATE if at GRATE;
   if $obj != 0 && here CHAIN {
    say "$in1 what?";
    $obj = 0;
    #< GOTO 2600 >
   }
   $obj = CHAIN if here CHAIN;
   if $obj == 0 {rspeak 28; #< GOTO 2012 > }
   vopen;
  }
  when EAT {
   if here FOOD {
    destroy FOOD;
    rspeak 72;
    #< GOTO 2012 >
   } else {
    say "$in1 what?";
    $obj = 0;
    #< GOTO 2600 >
   }
  }
  when QUIT {
   $gaveup = yes(22, 54, 54);
   if $gaveup { normend }
   else { #< GOTO 2012 > }
  }
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
   #< GOTO 2012 >
  }
  when SCORE {
   my int $score = score(True);
   say "If you were to quit now, you would score $score of a possible 350.";
   $gaveup = yes(143, 54, 54);
   if $gaveup { normend }
   else { #< GOTO 2012 > }
  }
  when FOO {
   my $k = vocab $word1, 3;
   if $foobar == 1-$k {
    $foobar = $k;
    if $k != 4 {rspeak 54; #< GOTO 2012 > }
    $foobar = 0;
    if @place[EGGS] == 92 || toting EGGS && $loc == 92 {
     rspeak 42;
     #< GOTO 2012 >
    }
    @prop[TROLL] = 1 if @place[EGGS] & @place[TROLL] & @prop[TROLL] == 0;
    $k = $loc == 92 ?? 0 !! here EGGS ?? 1 !! 2;
    move EGGS, 92;
    pspeak EGGS, $k;
    #< GOTO 2012 >
   } else {
    rspeak($foobar ?? 151 !! 42);
    #< GOTO 2012 >
   }
  }
  when BRIEF {
   $abbnum = 10000;
   $detail = 3;
   rspeak 156;
   #< GOTO 2012 >
  }
  when READ {
   $obj = MAGZIN if here MAGZIN;
   $obj = $obj * 100 + TABLET if here TABLET;
   $obj = $obj * 100 + MESSAG if here MESSAG;
   $obj = OYSTER if $closed && toting OYSTER;
   if $obj > 100 || $obj == 0 || dark {
    say "$in1 what?"; $obj = 0; #< GOTO 2600 >
   }
   vread;
  }
  when SUSPEND {
  #««
   if $demo {rspeak 201; #< GOTO 2012 > }
   say "I can suspend your adventure for you so that you can resume later, but";
   say "you will have to wait at least $latency minutes before continuing.";
   if yes(200, 54, 54) {
    ($saved, $savet) = datime;
    $setup = -1;
    ciao;
    # Save the game data somewhere in here.
   } else { #< GOTO 2012 > }
  »»
   
   # See label 8305 for what to (possibly) do on restoration.

  }
  when HOURS {
  #««
   mspeak 6;
   hours;
   #< GOTO 2012 >
  »»

  }
  when ON {
   if !here LAMP { rspeak @actspk[$verb] }
   elsif $limit < 0 { rspeak 184 }
   else {
    @prop[LAMP] = 1;
    rspeak 39;
    if $wzdark { #< GOTO 2000 > }
   }
   #< GOTO 2012 >
  }
  when OFF {
   if !here LAMP { rspeak @actspk[$verb] }
   else {
    @prop[LAMP] = 0;
    rspeak 40;
    rspeak 16 if dark;
   }
   #< GOTO 2012 >
  }
  when KILL { vkill }
  when POUR {
# 9130:
   $obj = liq if $obj == BOTTLE | 0;
   if $obj == 0 { #< GOTO 8000 > }
   if !toting $obj {rspeak @actspk[$verb]; #< GOTO 2012 > }
   if !($obj == OIL | WATER) {rspeak 78; #< GOTO 2012 > }
   @prop[BOTTLE] = 1;
   @place[OBJ] = 0;
   if at DOOR {
    @prop[DOOR] = ($obj == OIL);
    rspeak 113 + @prop[DOOR];
    #< GOTO 2012 >
   } elsif at PLANT {
    if $obj != WATER {rspeak 112; #< GOTO 2012 > }
    pspeak PLANT, @prop[PLANT] + 1;
    @prop[PLANT] = (@prop[PLANT] + 2) % 6;
    @prop[PLANT2] = @prop[PLANT] idiv 2;
    domove NULL;
    #< GOTO 2 >
   } else {rspeak 77; #< GOTO 2012 > }
  }
  when DRINK {
# 9150:
   if $obj == 0 && liqloc($loc) != WATER && (liq != WATER || !here BOTTLE) {
    #< GOTO 8000 >
   }
   if $obj == 0 | WATER {
    if liq == WATER && here BOTTLE {
     @prop[BOTTLE] = 1;
     @place[WATER] = 0;
     rspeak 74;
    } else { rspeak @actspk[$verb] }
   } else { rspeak 110 }
   #< GOTO 2012 >
  }
  when FILL {
# 9220:
   if $obj == VASE {
    if liqloc($loc) == 0 { rspeak 144 }
    elsif !toting VASE { rspeak 29 }
    else {
     rspeak 145;
     @prop[VASE] = 2;
     @fixed[VASE] = -1;

     # In the original Fortran, when the vase is filled with water or oil, its
     # property is set so that it breaks into pieces, *but* the code then
     # branches to label 9024 to actually drop the vase.  Once you cut out the
     # unreachable states, it turns out that the vase remains intact if the
     # pillow is present, but even if it survives it is still marked as a fixed
     # object and can't be picked up again.  This is probably a bug in the
     # original code, but who am I to fix it?

     @prop[VASE] = 0 if at PILLOW;
     pspeak VASE, @prop[VASE] + 1;
     drop $obj, $loc;
    }
   } else {
    if $obj != 0 & BOTTLE { rspeak @actspk[$verb] }
    if $obj == 0 && !here BOTTLE { #< GOTO 8000 > }
    if liq != 0 { rspeak 105 }
    elsif liqloc($loc) == 0 { rspeak 106 }
    else {
     @prop[BOTTLE] = @cond[$loc] +& 2;
     @place[liq] = -1 if toting BOTTLE;
     rspeak(liq == OIL ?? 108 !! 107);
    }
   }
   #< GOTO 2012 >
  }
  when BLAST { #< GOTO 9230 > }
  default { bug 23 }
 }


# Label 9010 (transitive carry/take):
sub vtake() {
 if toting $obj {rspeak @actspk[$verb]; #< GOTO 2012 > }
 my int $spk = 25;
 $spk = 115 if $obj == PLANT && @prop[PLANT] <= 0;
 $spk = 169 if $obj == BEAR && @prop[BEAR] == 1;
 $spk = 170 if $obj == CHAIN && @prop[BEAR] != 0;
 if fixed $obj {rspeak $spk; #< GOTO 2012 > }
 if $obj == WATER | OIL {
  if !here BOTTLE || liq != $obj {
   $obj = BOTTLE;
   if toting BOTTLE && @prop[BOTTLE] == 1 { #< GOTO 9220 > }
   $spk = 105 if @prop[BOTTLE] != 1;
   $spk = 104 if !toting BOTTLE;
   rspeak $spk;
   #< GOTO 2012 >
  }
  $obj = BOTTLE;
 }
 if $holding >= 7 {
  rspeak 92;
  #< GOTO 2012 >
 }
 if $obj == BIRD && @prop[BIRD] == 0 {
  if toting ROD {
   rspeak 26;
   #< GOTO 2012 >
  }
  if !toting CAGE {
   rspeak 27;
   #< GOTO 2012 >
  }
  @prop[BIRD] = 1;
 }
 carry BIRD+CAGE-$obj, $loc if $obj == BIRD | CAGE && @prop[BIRD] != 0;
 carry $obj, $loc;
 $k = liq;
 @place[$k] = -1 if $obj == BOTTLE && $k != 0;
 rspeak 54;
 #< GOTO 2012 >
}

# Label 9040 (transitive lock/unlock):
sub vopen() {
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
 #< GOTO 2012 >
}

sub vread() {
# 9270:
 if dark {say "I see no $in1 here."; #< GOTO 2012 > }
 my int $spk = @actspk[$verb];
 $spk = 190 if $obj == MAGZIN;
 $spk = 196 if $obj == TABLET;
 $spk = 191 if $obj == MESSAG;
 $spk = 194 if $obj == OYSTER && @hinted[2] && toting OYSTER;
 if $obj != OYSTER || @hinted[2] || !toting OYSTER || !$closed { rspeak $spk }
 else { @hinted[2] = yes(192, 193, 54) }
 #< GOTO 2012 >
}

sub vkill() {
# 9120:
 if $obj == 0 {
  $obj = DWARF if $dflag >= 2 && @dloc[^5].any == $loc;
  $obj = $obj * 100 + SNAKE if here SNAKE;
  $obj = $obj * 100 + DRAGON if at(DRAGON) && @prop[DRAGON] == 0;
  $obj = $obj * 100 + TROLL if at TROLL;
  $obj = $obj * 100 + BEAR if here(BEAR) && @prop[BEAR] == 0;
  if $obj > 100 { #< GOTO 8000 > }
  if $obj == 0 {
   $obj = BIRD if here(BIRD) && $verb != THROW;
   $obj = $obj * 100 + CLAM if here(CLAM | OYSTER);
   if $obj > 100 { #< GOTO 8000 > }
  }
 }
 my int $spk = @actspk[$verb];
 given $obj {
  when BIRD {
   if $closed {rspeak 137; #< GOTO 2012 > }
   destroy BIRD;
   @prop[BIRD] = 0;
   $tally2++ if @place[SNAKE] == 19;
   $spk = 45;
  }
  when 0 { $spk = 44 }
  when CLAM | OYSTER { $spk = 150 }
  when SNAKE { $spk = 46 }
  when DWARF {
   if $closed {rspeak 136; normend; #< GOTO somewhere? > }
   else { $spk = 49 }
  }
  when DRAGON {
   if @prop[DRAGON] != 0 { $spk = 167 }
   else {
    rspeak 49;
    ($verb, $obj) = (0, 0);
    my Str $reply = $*IN.get;
    if $reply !~~ m:i/^^\h*y/ { #< GOTO 2608 > }
    pspeak DRAGON, 1;
    @prop[DRAGON, RUG] = 2, 0;
    move DRAGON+100, -1;
    move RUG+100, 0;
    move DRAGON, 120;
    move RUG, 120;
    move $_, 120 for grep { @place[$_] == 119 | 121 }, ^65;
    $loc = 120;
    domove NULL;
    #< GOTO 2 >
   }
  }
  when TROLL { $spk = 157 }
  when BEAR { $spk = 165 + (@prop[BEAR]+1) idiv 2 }
 }
 rspeak $spk;
 #< GOTO 2012 >
}
