---- MODULE FPM ----
EXTENDS Integers
VARIABLE v
F == [x \in 0..2 |-> 6 \div x]
Init == v = [a |-> F, b |-> 1]
Next == UNCHANGED v
Spec == Init /\ [][Next]_v
====
