---- MODULE spec ----
\* SPECIFICATION Init /\ [][Next]_vars: a stuttering step is a step.
EXTENDS Integers
VARIABLES
    \* @type: Int;
    x,
    \* @type: Int;
    y
\* @type: <<Int, Int>>;
vars == <<x, y>>
Init == x = 0 /\ y = 0
Next == x < 3 /\ x' = x + 1 /\ y' = y
Spec == Init /\ [][Next]_vars
Inv == x < 3
====
SPECIFICATION Spec
INVARIANT Inv
