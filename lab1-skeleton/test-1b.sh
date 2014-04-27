#!/bin/sh

# UCLA CS Test Script

# Semicolons
:
      :
: : :

# Simple Commands

expr 1 + 3 - 4 / 2

# AND Commands

echo And1 && echo And2

# OR Commands

cat < DOESNOTEXIST || echo Alternative

echo OrWorks || echo OrNot

# Sequence Commands

echo seq1; echo seq2

# Pipe Commands

echo first | echo second | echo third
ls | sort -r | cat

cat /usr/share/dict/words | head -n 20 > pipetemp.txt

head -n 5 < pipetemp.txt > pipeout.txt

cat pipeout.txt

# I/O Redirection

echo first file > test2.txt

cat test2.txt

(cat) < test2.txt

echo overwrite > test2.txt

cat test2.txt

cat < test2.txt

cat < DOESNOTEXIST

# Mixed commands

echo Hello World | tr a-z A-Z | sort || echo sort failed

echo Sup && echo dude | sort

echo Hi > test.txt && cat < test.txt

echo Byte > test1.txt && cat test1.txt | wc -w

(echo CS; echo 111) | wc -w

(echo Operating; echo Systems && echo Principles)

echo Roger || echo Chen && echo Kailin


# cat /usr/share/dict/words | head -n 20 > testwords.txt

# (head -n 5) < testwords.txt > testsort.txt

# cat testsort.txt

# Bad Commands

km

gh

ls -29133

cp

cp DOESNOTEXIT


# Cleanup

rm test.txt
rm test1.txt
rm test2.txt
rm pipetemp.txt
rm pipeout.txt

ls
