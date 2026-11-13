---- MODULE next_error_replay_continue ----
\* options: -continue -deadlock
\* With -continue TLC reports the error in Next again after its replay of the
\* step has printed what it prints.
EXTENDS Naturals, TLC
VARIABLE x
Init == x = 0 /\ PrintT(<<"init", x>>)
Next == /\ PrintT(<<"next", x>>)
        /\ \/ x < 2 /\ x' = x + 1
           \/ x = 2 /\ x' = x[1]
====
INIT Init
NEXT Next
