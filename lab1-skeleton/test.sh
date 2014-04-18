#!/bin/sh

# UCLA CS Test Script

echo OrWorks || echo OrNot

echo Hello World | tr a-z A-Z | sort || echo sort failed

echo Sup && echo dude | sort

echo Hi > test.txt && cat < test.txt

echo Hello > test0.txt

echo Yo > test2.txt

echo Byte > test3.txt && cat test3.txt | wc -w

echo seq1; echo seq2

(echo CS; echo 111) | wc -w

(echo Operating; echo Systems && echo Principles)
