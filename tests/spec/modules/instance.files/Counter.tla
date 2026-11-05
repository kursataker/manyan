---- MODULE Counter ----
EXTENDS Naturals
CONSTANT Max
VARIABLE n
Inc == n + 1
AtMax == n = Max
Inv == n <= Max
====
