---- MODULE override_constant ----
\* Apalache's OVERRIDE_N == e gives the constant N the value e, where the
\* configuration gives none, and OVERRIDE_Max a definition Max.
\* options: -length 4
EXTENDS Integers
CONSTANT
    \* @type: Int;
    N
VARIABLE
    \* @type: Int;
    x
OVERRIDE_N == 3
Max == 10
OVERRIDE_Max == N
Init == x = 0
Next == x' = x + 1
Inv == x <= Max
====
INIT Init
NEXT Next
INVARIANT Inv
