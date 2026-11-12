---- MODULE unbalanced_if ----
\* So for the arms of IF, through the operators they call.
\* options: -length 2
EXTENDS Integers
VARIABLE
    \* @type: Int;
    x,
    \* @type: Int;
    y
Init == x = 0 /\ y = 0
Step == x' = x + 1
Reset == x' = 0 /\ y' = 0
Next == /\ IF x < 3 THEN Step ELSE Reset
        /\ y' = x'
Inv == y < 2
====
INIT Init
NEXT Next
INVARIANT Inv
