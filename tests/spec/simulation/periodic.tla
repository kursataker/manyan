---- MODULE periodic ----
\* TLC evaluates _PERIODIC at each progress report, the last after the
\* postconditions; FALSE there stops the simulation, an error not in the
\* exit status.
\* options: -simulate num=2 -depth 3 -seed 1
EXTENDS Naturals, TLC
VARIABLE x
Init == x = 0
Next == x < 3 /\ x' = x + 1
Post == PrintT(<<"post", TLCGet("stats").generated>>)
Periodic == PrintT(<<"periodic", TLCGet("stats").generated>>) /\ FALSE
====
INIT Init
NEXT Next
POSTCONDITION Post
_PERIODIC Periodic
CHECK_DEADLOCK FALSE
