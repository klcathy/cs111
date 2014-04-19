#!/bin/sh

# UCLA CS Test Script

# echo OrWorks || echo OrNot

# echo Hello World | tr a-z A-Z | sort || echo sort failed

# echo Sup && echo dude | sort

# echo Hi > test.txt && cat < test.txt

# echo Hello > test0.txt

# echo Byte > test1.txt && cat test1.txt | wc -w

# echo seq1; echo seq2

# (echo CS; echo 111) | wc -w

# (echo Operating; echo Systems && echo Principles)

# cat /usr/share/dict/words | head -n 20 > pipetemp.txt

# head -n 5 < pipetemp.txt > pipeout.txt

# cat pipeout.txt

cat /usr/share/dict/words | head -n 20 > testwords.txt

(head -n 5) < testwords.txt > testsort.txt

cat testsort.txt
