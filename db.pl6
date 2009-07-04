sub indexLines(Str *@lines --> List of Str) {
 gather for @lines {
  FIRST { take undef }
  state Str $text = '';
  state int $i = 1;
  my($num, $t) = .split: "\t", 2;
  if $i == $num { $text ~= $t }
  else {
   take ($text ~~ /\>\$\</ ?? undef !! $text), undef xx $num - $i - 1;
   ($text, $i) = ($t, $num);
  }
  LAST { take $text }
 }
}

my Str @longDesc <== indexLines <== $=adventData01.lines(:!chomp);

my Str @shortDesc <== indexLines <== $=adventData02.lines(:!chomp);

my int @travel[*;*;*]
 <== map { .defined ?? .split("\n").map: *.split("\t") !! undef }
 <== indexLines <== $=adventData03.lines(:!chomp);

(my Array of int %vocab).push: $=adventData04.lines.map: { .split("\t").[1,0] }

my Array of Str @itemDesc = gather for $=adventData05.lines(:!chomp) {
 state int $i = 0;
 state int $j = -1;
 state Str @accum = ();
 my($n, $msg) = .split: "\t", 2;
 if 0 < $n < 100 {
  take @accum.map({ /\>\$\</ ?? undef !! $_ }), [] xx $n - $i - 1;
  ($i, $j) = ($n, -1);
  @accum = $msg, ;
 } else {
  @accum.push: '' if $n / 100 != $j;
  @accum[*-1] ~= $msg;
  $j = $n / 100;
 }
 LAST { take @accum.map: { /\>\$\</ ?? undef !! $_ } }
}

my Str @rmsg <== indexLines <== $=adventData06.lines(:!chomp);

# Read in the item locations from data section 7:
for $=adventData07.lines {
 my($obj, $p, $f) = .split: "\t";
 @place[$obj] = $p;
 @fixed[$obj] = $f // 0;
}
for @fixed.keys({ @fixed[$_] > 0 }).reverse -> $k {
 drop $k + 100, @fixed[$k];
 drop $k, @place[$k];
}
drop $_, @place[$_]
 for @fixed.keys({ @place[$_] != 0 && @fixed[$_] <= 0 }).reverse;

my int @actspk[32] <== indexLines <== $adventData08.lines;

my int @cond = 0, *;
for $=adventData09.lines {
 my($bit, @locs) = .split: "\t";
 @cond[$_] +|= 1 +< $bit for @locs;
}

my Pair @classes <== map { [=>] .split("\t") } <== $=adventData10.lines(:!chomp);

my int @hints[*;4] <== map { .defined ?? .split("\t") !! undef }
 <== indexLines <== $=adventData11.lines;

my Str @magicMsg <== indexLines <== $=adventData12.lines(:!chomp);
