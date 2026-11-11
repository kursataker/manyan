---- MODULE unbounded_int ----
\* x ranges over all integers: the solver finds 7 * 7 = 49.
EXTENDS Integers
VARIABLES
    \* @type: Int;
    x
Init == x \in Int
Next == x' = x
Inv == x * x /= 49
====
INIT Init
NEXT Next
INVARIANT Inv
