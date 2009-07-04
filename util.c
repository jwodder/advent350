sub toting(int $item --> Bool) { @place[$item] == -1 }

sub here(int $item --> Bool) { @place[$item] == $loc || toting $item }

sub at(int $item --> Bool) { $loc == @place[$item] | @fixed[$item] }

sub liq2(int $p --> int) { (WATER, 0, OIL)[$p] }

sub liq( --> int) { liq2(@prop[BOTTLE] max -1-@prop[BOTTLE]) }

sub liqloc(int $loc --> int) { liq2(@cond[$loc] +& 4 ?? @cond[$loc] +& 2 !! 1) }

sub bitset(int $loc, int $n --> Bool) { @cond[$loc] +& 1 +< $n }

sub forced(int $loc --> Bool) { @travel[$loc;0;1] == 1 }

sub dark( --> Bool) { !(@cond[$loc] +& 1 || (@prop[LAMP] && here(LAMP)) }

sub pct(int $x --> Bool) { (^100).pick < $x }

sub speak(Str $s) {
 return if !$s;
 print "\n" if $blklin;
 print $s;
}

sub pspeak(int $item, int $state) { speak @itemDesc[$item;$state+1] }
sub rspeak(int $msg) { speak @rmsg[$msg] if $msg != 0 }

sub yes(int $x, int $y, int $z --> Bool) {
 loop {
  rspeak $x if $x != 0;
  my Str ($reply) = getin;  # Ignore everything after the first word.
  if $reply eq 'YES' | 'Y' {
   rspeak $y if $y != 0;
   return True;
  } elsif $reply eq 'NO' | 'N' {
   rspeak $z if $z != 0;
   return False;
  } else { say "Please answer the question." }
 }
}

sub destroy(int $obj) { move $obj, 0 }

sub juggle(int $obj) {
 move $obj, @place[$obj];
 move $obj+100, @fixed[$obj];
}

sub move(int $obj, int $where) {
 my int $from = $obj > 100 ?? @fixed[$obj-100] !! @place[$obj];
 carry $obj, $from if 0 < $from <= 300;
 drop $obj, $where;
}

sub put(int $obj, int $where, int $pval --> int) {
 move $obj, $where;
 return -1 - $pval;
}

sub carry(int $obj, int $where) {
 if $obj <= 100 {
  return if @place[$obj] == -1;
  @place[$obj] = -1;
  $holding++;
 }
 @atloc[$where].splice:
  @atloc[$where].keys.first({ @atloc[$where;$_] == $obj }), 1;
}

sub drop(int $obj, int $where) {
 if $obj > 100 {
  @fixed[$obj-100] = $where
 } else {
  $holding-- if @place[$obj] == -1;
  @place[$obj] = $where;
 }
 @atloc[$where].unshift: $obj if $where > 0;
}

sub bug(int $num) {
 say "Fatal error, see source code for interpretation.";

# Given the above message, I suppose I should list the possible bug numbers in
# the source somewhere, and right here is as good a place as any:
# 5 - Required vocabulary word not found
# 20 - Special travel number (500>L>300) is outside of defined range
# 22 - Vocabulary type (N/1000) not between 0 and 3
# 23 - Intransitive action verb not defined
# 24 - Transitive action verb not defined
# 26 - Location has no travel entries

 #say "Probable cause: erroneous info in database.";  # Not in this version
 say "Error code = $num";
 exit -1;
}

sub vocab(Str $word, int $type --> int) {
 my int @matches = %vocab{$word};
 if $type >= 0 { @matches.=grep: { $_ idiv 1000 == $type } }
 if !@matches {
  if $type >= 0 { bug 5 }
  return -1;
 } else { return $type >= 0 ?? @matches[0] % 1000 !! [min] @matches }
 # When returning values of a specified type, there can be no more than one
 # match; if there is more than one, someone's been messing with the data
 # sections.
}

sub getin( --> List of Str) {
 print "\n";
 loop {
  print "> ";
  my Str $raw1, $raw2 = $*IN.get.words;
  next if !$raw1.defined && $blklin;
  my Str $word1, $word2 = ($raw1, $raw2).map:
   { .defined ?? .substr(0, 5).uc !! undef };
  return $word1, $raw1, $word2, $raw2;
 }
}
