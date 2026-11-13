---- MODULE fingerprint_next_continue ----
\* options: -continue -deadlock
\* With -continue TLC replays an error fingerprinting a successor: it
\* prints the behavior again, without the error, which it has no message for.
EXTENDS Integers
VARIABLE v
Init == v = 0
Next == v < 1 /\ v' = [y \in 1..2 |-> [x \in 0..2 |-> CHOOSE z \in {x} : z > 1]]
====
INIT Init
NEXT Next
