---- MODULE unassigned_disabled ----
\* Apalache refuses an action that leaves a variable unassigned, even one
\* never enabled, which TLC never takes.
\* options: -length 2
EXTENDS Integers
VARIABLE
    \* @type: Int;
    x,
    \* @type: Int;
    y
Init == x = 0 /\ y = 0
Next == \/ x > 5 /\ x' = 1
        \/ x' = x + 1 /\ y' = y
Inv == x < 3
====
INIT Init
NEXT Next
INVARIANT Inv
