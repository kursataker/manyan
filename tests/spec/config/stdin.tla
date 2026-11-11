---- MODULE stdin ----
\* A configuration read from a pipe, whose size is not known before it is read.
\* options: -config /dev/stdin
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next == x' = (x + 1) % 3
Inv == x < 3
====
INIT Init
NEXT Next
INVARIANT Inv
