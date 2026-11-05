---- MODULE fingerprint_init ----
\* Computing an initial state's function fails: TLC cannot fingerprint it.
EXTENDS Integers
VARIABLE v
Init == v = [a |-> [x \in 0..2 |-> 6 \div x], b |-> 1]
Next == UNCHANGED v
====
INIT Init
NEXT Next
