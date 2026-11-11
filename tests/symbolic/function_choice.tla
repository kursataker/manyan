---- MODULE function_choice ----
\* A step chooses any function in [1..3 -> 0..2].
EXTENDS Integers
VARIABLES
    \* @type: Int -> Int;
    f
Init == f = [i \in 1..3 |-> 0]
Next == f' \in [1..3 -> 0..2]
Inv == f[1] + f[2] + f[3] < 6
====
INIT Init
NEXT Next
INVARIANT Inv
