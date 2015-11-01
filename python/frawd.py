#!/usr/bin/python
# Program for calculating the second magic word needed for wizard-mode
# authentication

from   __future__ import print_function
import argparse
import datetime

now = datetime.datetime.now()

parser = argparse.ArgumentParser()
parser.add_argument('-H', '--hour', type=int, default=now.hour)
parser.add_argument('-M', '--minute', type=int, default=now.minute)
parser.add_argument('-m', '--magic-num', type=int, default=11111)
parser.add_argument('word')
args = parser.parse_args()

word = args.word.strip().upper()
if not word.isalpha() or len(word) != 5:
    raise SystemExit('The input word must consist of exactly five letters.')

val = [ord(c) - 64 for c in word]
t = args.hour * 100 + args.minute - args.minute % 10
d = args.magic_num
for y in range(5):
    print(chr((abs(val[y] - val[(y+1) % 5]) * (d % 10) + (t % 10)) % 26 + 65),
          end='')
    t //= 10
    d //= 10
print()
