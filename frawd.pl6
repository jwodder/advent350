#!/usr/bin/env perl6
use v6;

sub MAIN(Int :$magnm = 11111, Int :$hour where 0..23 = Time::localtime.hour,
 Int :$minute where 0..59 = Time::localtime.minute, Str $word where
 { $^w.chars == 5 && $^w.comb.map({ .uc.ord }).all ~~ 65..90 }) {
 my @val = $word.comb.map({ .uc.ord }) »-» 64;
 my $t = $hour * 100 + $minute - $minute % 10;
 my $d = $magnm;
 for ^5 -> $y {
  print chr(((@val[$y] - @val[($y+1) % 5]).abs * ($d % 10) + ($t % 10)) % 26
   + 65);
  $t = ($t / 10).floor;
  $d = ($d / 10).floor;
 }
 print "\n";
}
