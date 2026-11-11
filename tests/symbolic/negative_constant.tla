---- MODULE negative_constant ----
\* Apalache's configuration gives a constant a negative integer.
EXTENDS Integers
CONSTANT
    \* @type: Int;
    Low
VARIABLES
    \* @type: Int;
    x
Init == x = 0
Next == x' = x - 1
Inv == x > Low
====
CONSTANT Low = -3
INIT Init
NEXT Next
INVARIANT Inv
