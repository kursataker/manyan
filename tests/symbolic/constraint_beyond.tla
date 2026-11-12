---- MODULE constraint_beyond ----
\* x = 3 is past the constraint, so no error, and no deadlock where Next
\* stops at 5.
EXTENDS Naturals
VARIABLE
    \* @type: Int;
    x
Init == x = 0
Next == x < 5 /\ x' = x + 1
Small == x < 2
Inv == x < 3
====
INIT Init
NEXT Next
CONSTRAINT Small
INVARIANT Inv
