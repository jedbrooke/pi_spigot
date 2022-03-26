#!/bin/bash
# numbers from http://www.numberworld.org/digits/Pi/
PROG=./pi_spigot
echo "checking 0-50"
cmp <(echo "243f6a8885a308d313198a2e03707344a4093822299f31d008") <($PROG 0) && echo "success"
echo "checking 50-100"
cmp <(echo "2efa98ec4e6c89452821e638d01377be5466cf34e90c6cc0ac") <($PROG 50) && echo "success"
echo "checking 9950-10000"
cmp <(echo "f169b5f18a8c73ee0b5e57368f6c79f4bb7a595926aab49ec6") <($PROG 9950) && echo "success"
echo "checking 99950-100000"
cmp <(echo "443388751069558b3e62e612bc302ec487aa9a6ea22673c1a5") <($PROG 99950) && echo "success"
echo "checking 999950-1000000"
cmp <(echo "bc89273abbced2884adaa7f46c59b44c28e672c29ffd342362") <($PROG 999950) && echo "success"