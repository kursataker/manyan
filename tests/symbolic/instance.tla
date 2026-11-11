---- MODULE instance ----
\* A module instantiated twice: its variable substituted by x and by y, its
\* constant by a definition; y reaches Max first.
EXTENDS Integers
VARIABLES
    \* @type: Int;
    x,
    \* @type: Int;
    y
A == INSTANCE Counter WITH n <- x, Max <- 5
B == INSTANCE Counter WITH n <- y, Max <- 3
Init == A!Init /\ B!Init
Next == \/ A!Next /\ UNCHANGED y
        \/ B!Next /\ UNCHANGED x
Inv == B!Small
====
INIT Init
NEXT Next
INVARIANT Inv
