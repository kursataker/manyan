---- MODULE constants ----
\* Constants from the configuration bound the search.
EXTENDS Integers
CONSTANTS
    \* @type: Int;
    N
VARIABLES
    \* @type: Int;
    x
Init == x = 0
Next == x < N /\ x' = x + 1
Inv == x < N
====
CONSTANTS N = 3
INIT Init
NEXT Next
INVARIANT Inv
