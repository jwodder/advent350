# Variables saved, in order:
my int $loc, $newloc, $oldloc, $oldloc2, $limit;
my int $turns, $iwest, $knifeloc, $detail;
my int $numdie, $holding, $foobar, $bonus;
my int $tally;
my int $tally2;
my int $abbnum;
my int $clock1;
my int $clock2;
my bool $wzdark, $closing, $lmwarn, $panic, $closed, $gaveup;
my int @prop[65];
my int @abb[141];
my int @hintlc[10];
my bool @hinted[10];
my int @dloc[6];
my int @odloc[6];
my bool @dseen[6];
my int $dflag, $dkill;
my int @place[65];
my int @fixed[65];
my int @atloc[141;*];
# The magic version also needs to save $saved and $savet.

sub writeInt(IO $out, int32 $i) {
 $out.write(Buf.new((^4).map: { $i +> 8*(3-$_) +& 0xFF }), 4)
}

sub writeBool(IO $out, bool *@bits) {
 my Int $x = 0;
 $x +|= 1 +< $_ if @bits[$_] for @bits.keys;
 my Buf $blob .= new($x);
 $out.write($blob, #< $blob.bytes ??? > );
}

sub readInt(IO $in --> int32) {

}

sub readBool(IO $in, int $qty --> List of bool) {

}

sub savegame(Str $file) {
 my IO $adv = open $file, :w, :bin;
 # What exactly happens if the file fails to open?

 # As pack() and unpack() have not been fully specified for Perl 6 yet (and
 # thus certainly won't be available in Rakudo for a while), the game data is
 # written & read using homemade routines that produce byte strings that will
 # hopefully be forwards-compatible with the results of the future pack() &
 # unpack() (on 32-bit systems, at least).

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

}
