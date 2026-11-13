---- MODULE fingerprint_successor_again_continue ----
\* options: -continue
\* With -continue TLC reports a successor it cannot fingerprint again as it
\* replays the step.
EXTENDS Integers
VARIABLE x
Init == x = 0
Next == IF {1} \in SUBSET(1..31) THEN x' = SUBSET(SUBSET(1..32)) ELSE x' = 2
====
INIT Init
NEXT Next
