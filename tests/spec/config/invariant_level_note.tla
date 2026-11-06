---- MODULE invariant_level_note ----
\* An invariant that is not a state predicate, whose level depends on a
\* variable outside its temporal operators: TLC adds a note to its error.
EXTENDS Naturals
VARIABLE x
Init == x = 1
Next == x' = x + 1 /\ x < 3
Eventually(a) == IF a = 1 THEN <>(x = 3) ELSE TRUE
Inv == Eventually(x)
====
INIT Init
NEXT Next
INVARIANT Inv
