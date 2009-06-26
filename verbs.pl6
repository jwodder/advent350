# Label 4080 (intransitive verb handling):
 given $verb {
  when NOTHING {rspeak 54; #< GOTO 2012 > }
  when WALK {rspeak $spk; #< GOTO 2012 > }
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
   $spk = 28;
   $obj = CLAM if here CLAM;
   $obj = OYSTER if here OYSTER;
   $obj = DOOR if at DOOR;
   $obj = GRATE if at GRATE;
   if $obj != 0 && here CHAIN { #< GOTO 8000 > }
   $obj = CHAIN if here CHAIN;
   if $obj == 0 { #< GOTO 2011 > }
   #< GOTO 9040 >
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
   $spk = 98;
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
  when BRIEF { #< GOTO 8260 > }
  when READ { #< GOTO 8270 > }
  when SUSPEND { #< GOTO 8300 > }
  when HOURS { #< GOTO 8310 > }
  when ON { #< GOTO 9070 > }
  when OFF { #< GOTO 9080 > }
  when KILL { #< GOTO 9120 > }
  when POUR { #< GOTO 9130 > }
  when DRINK { #< GOTO 9150 > }
  when FILL { #< GOTO 9220 > }
  when BLAST { #< GOTO 9230 > }
  default { bug 23 }
 }


# Label 9010 (transitive carry/take):
sub vtake() {
 if toting $obj {rspeak $spk; #< GOTO 2012 > }
 $spk = 25;
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
