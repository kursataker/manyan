---- MODULE postcondition_after_violation ----
\* After a violation the simulator checks the postconditions, once it has
\* summarized the run; the violation's exit status stands.
\* options: -simulate num=2 -depth 5 -seed 2
EXTENDS Naturals, TLC
VARIABLE x
Init == x = 0
Next == x < 3 /\ x' = x + 1
Inv == x < 2
Post == TLCGet("diameter") = 99
====
INIT Init
NEXT Next
INVARIANT Inv
POSTCONDITION Post
