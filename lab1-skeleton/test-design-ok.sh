#! /bin/sh

# UCLA CS111 Lab 1A Design

cat >& /etc/passwd | tr a-z A-Z | sort -u || echo sort failed!

a<&b >&c  | d < &e >> f | g<   &h >| i

2 <       > /etc/passwd

(a && b)<&c>>d

(a && b) >&       c

(a >> b) >> c

(a && b     ) >  | c

(a >| b) && ( c >& d)
