#!/usr/bin/env perl6
# Program for calculating the second magic word needed for wizard-mode
# authentication
use v6;

sub MAIN(Int :m($magnm) = 11111, Int where ^24 :H($hour) = localtime.hour,
 Int where ^60 :M($minute) = localtime.minute, Str where { $^w.chars == 5
 && $^w.uc.ord.all ~~ 65..90 } $word) {
 my @val = $word.uc.ord »-» 64;
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
