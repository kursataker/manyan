---- MODULE alias_undefined ----
\* TLC resolves the ALIAS as it prints a state: an undefined one is an
\* error of each state's alias.
EXTENDS Naturals
VARIABLE x, y
Init == x = 0 /\ y = 0
Next == x' = x + 1 /\ y' = y
Inv == x < 1
====
INIT Init
NEXT Next
INVARIANT Inv
ALIAS Nope
