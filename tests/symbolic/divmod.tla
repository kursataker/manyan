---- MODULE divmod ----
\* x \div 3 and x % 3 of a number in -20..20: 3 * q + r = x always,
\* and r = 2 is reached.
EXTENDS Integers
VARIABLES
    \* @type: Int;
    x
Init == x \in -20..20
Next == x' = x - 1
Exact == (3 * (x \div 3)) + (x % 3) = x /\ x % 3 >= 0
NotTwo == x % 3 /= 2 \/ x > -5
====
INIT Init
NEXT Next
INVARIANT Exact NotTwo
