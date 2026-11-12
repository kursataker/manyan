---- MODULE constraint_invariant ----
\* TLC checks the invariant on a state outside the constraint, x = 2,
\* but goes no further from it.
EXTENDS Naturals
VARIABLE
    \* @type: Int;
    x
Init == x = 0
Next == x' = x + 1
Small == x < 2
Inv == x < 2
====
INIT Init
NEXT Next
CONSTRAINT Small
INVARIANT Inv
