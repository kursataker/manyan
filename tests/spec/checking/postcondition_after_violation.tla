---- MODULE postcondition_after_violation ----
\* After a violation TLC checks the postconditions too, once it has rebuilt
\* the behavior; the violation's exit status stands.
EXTENDS Naturals, TLC
VARIABLE x
Init == x = 0
Next == x < 3 /\ x' = x + 1
Inv == x < 2
Post == TLCGet("distinct") \div 0 = 3
====
INIT Init
NEXT Next
INVARIANT Inv
POSTCONDITION Post
