---- MODULE fingerprint_union ----
\* UNION {S}, whose set {S} TLC evaluates: a frame of the call stack.
\* Found by tlaplus/Examples' TestGraphs.
EXTENDS Integers
VARIABLE v
Init == v = UNION {{[x \in 0..2 |-> 6 \div x]}}
Next == UNCHANGED v
====
INIT Init
NEXT Next
