sub domove(int $motion) {
# 8:
 $goto = 2;
 $newloc = $loc;
 bug 26 if !@travel[$loc];
 given $motion {
  when NULLMOVE { return }
  when BACK {
   my int $k = forced($oldloc) ?? $oldloc2 !! $oldloc;
   ($oldloc2, $oldloc) = ($oldloc, $loc);
   if $k == $loc { rspeak 91 }
   else {
    my int $k2 = 0;
    for @travel[$loc].keys -> $kk {
     my $ll = @travel[$loc;$kk;0] % 1000;
     if $ll == $k {
      dotrav @travel[$loc;$kk;1];
      return;
     } elsif $ll <= 300 {
      $k2 = $kk if forced $ll && @travel[$ll;0;0] % 1000 == $k
     }
    }
    if $k2 != 0 { dotrav @travel[$loc;$k2;1] }
    else { rspeak 140 }
   }
  }
  when LOOK {
   rspeak 15 if $detail++ < 3;
   $wzdark = False;
   @abb[$loc] = 0;
  }
  when CAVE { rspeak($loc < 8 ?? 57 !! 58) }
  default {($oldloc2, $oldloc) = ($oldloc, $loc); dotrav $motion; }
 }
 # next bigLoop;
}

sub dotrav(int $motion) {
# 9:
 my int $rdest = -1;
 for @travel[$loc] -> $kk {
  if $kk[1..*].any == 1 | $motion ff * {
   my int $ll = $kk[0];
   my int $rcond = $ll idiv 1000;
   my int $robject = $rcond % 100;
   given $rcond {
    when 0 | 100 { $rdest = $ll % 1000 }
    when 0 ^..^ 100 { $rdest = $ll % 1000 if pct $_ }
    when 100 ^.. 200 { $rdest = $ll % 1000 if toting $robject }
    when 200 ^.. 300 { $rdest = $ll % 1000 if toting($robject) || at $robject }
    default { $rdest = $ll % 1000 if @prop[$robject] != $_ idiv 100 - 3 }
   }
   last if $rdest != -1;
  }
 }
 given $rdest {
  when -1 {
   given $motion {
    when 29 | 30 | (43..50) { rspeak 9 }
    when  7 | 36 | 37 { rspeak 10 }
    when 11 | 19 { rspeak 11 }
    when 62 | 65 { rspeak 42 }
    when 17 { rspeak 80 }
    default { rspeak($verb == FIND | INVENT ?? 59 !! 12) }
   }
  }
  when 0..300 { $newloc = $rdest }
  when 301 {
   if !$holding || $holding == 1 && toting EMERALD { $newloc = 99 + 100 - $loc }
   else {$newloc = $loc; rspeak 117; }
  }
  when 302 {
   drop EMERALD, $loc;
   $newloc = $loc == 33 ?? 100 !! 33;
  }
  when 303 {
   if @prop[TROLL] == 1 {
    pspeak TROLL, 1;
    @prop[TROLL] = 0;
    move TROLL2, 0;
    move TROLL2+100, 0;
    move TROLL, 117;
    move TROLL+100, 122;
    juggle CHASM;
    $newloc = $loc;
   } else {
    $newloc = $loc == 117 ?? 122 !! 117;
    @prop[TROLL] = 1 if @prop[TROLL] == 0;
    if toting BEAR {
     rspeak 162;
     @prop[CHASM] = 1;
     @prop[TROLL] = 2;
     drop BEAR, $newloc;
     @fixed[BEAR] = -1;
     @prop[BEAR] = 3;
     $tally2++ if @prop[SPICES] < 0;
     $oldloc2 = $newloc;
     death;
    }
   }
  }
  when 500 ^.. * { rspeak $rdest-500 }
  default { bug 20 }
 }
}

sub death() {
# 99:
 if $closing {
  rspeak 131;
  $numdie++;
  normend;
 } else {
  my $yea = yes(81 + $numdie*2, 82 + $numdie*2, 54);
  $numdie++;
  normend if $numdie == MAXDIE || !$yea;
  @place[WATER, OIL] = 0, 0;
  @prop[LAMP] = 0 if toting LAMP;
  drop $_, $_ == LAMP ?? 1 !! $oldloc2 for (^65).grep(*.toting).reverse;
  ($loc, $oldloc) = 3, 3;
  $goto = 2000;
  # next bigLoop;
 }
}

sub score(Bool $scoring --> int) {
 my int $score = 0;
 for 50..64 -> $i {
  $score += 2 if @prop[$i] >= 0;
  $score += $i == CHEST ?? 12 !! $i > CHEST ?? 14 !! 10
   if @place[$i] == 3 && @prop[$i] == 0;
 }
 $score += (MAXDIE - $numdie) * 10;
 $score += 4 if !($scoring || $gaveup);
 $score += 25 if $dflag != 0;
 $score += 25 if $closing;
 if $closed {
  given $bonus {
   when 0 { $score += 10 }
   when 133 { $score += 45 }
   when 134 { $score += 30 }
   when 135 { $score += 25 }
  }
 }
 $score++ if @place[MAGZIN] == 108;
 $score += 2;
 $score -= @hints[$_;1] if @hinted[$_] for 1..9;
 return $score;
}

sub normend() {
 my $score = score(False);
 say "You scored $score out of a possible 350 using $turns turns.";
 my($rank, $next) = @classes.grep({ .key >= $score }).[0,1];
 if $rank {
  speak $rank.value;
  if $next {
   my $diff = $next.key - $score + 1;
   say "To achieve the next higher rating, you need $diff more point",
    $diff == 1 ?? '.' !! 's.';
  } else {
   say "To achieve the next higher rating would be a neat trick!";
   say "Congratulations!!";
  }
 } else { say "You just went off my scale!!" }
 exit 0;
}

sub doaction() {
# 5010:
 if $word2 {
# 2800:
  ($word1, $in1) = ($word2, $in2);
  $word2 = $in2 = undef;
  $goto = 2610;
 } elsif $verb { transitive }
 else {
  say "What do you want to do with the $in1?";
  $goto = 2600;
 }
 # next bigLoop;
}
