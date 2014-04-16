#! /bin/sh

# UCLA CS Test Script


echo Hello World | tr a-z A-Z | sort || echo sort failed

echo Sup && echo dude | sort

echo Hi > test.txt && cat < test.txt

echo Yo > test2.txt && cat test2.txt

echo Bye > test3.txt && cat test3.txt | wc -w

(echo CS; echo 111) | wc -w

(echo Operating; echo Systems && echo Principles)



