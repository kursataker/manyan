---- MODULE constant_lazy_print ----
\* A constant definition is computed once, printing once, even when its
\* value is a set TLC keeps as its definition.
EXTENDS Naturals, TLC
VARIABLE x
F == IF PrintT("F") THEN {n \in Nat : n > 2} ELSE {}
G == PrintT("G") /\ 5 \in F
Init == x = 1 /\ G /\ 7 \in F
Next == UNCHANGED x
====
INIT Init
NEXT Next
