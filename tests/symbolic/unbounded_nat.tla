---- MODULE unbounded_nat ----
\* No natural is negative, and halving keeps it so.
EXTENDS Integers
VARIABLES
    \* @type: Int;
    x
Init == x \in Nat
Next == x' = x \div 2
Inv == x >= 0 /\ x % 2 \in {0, 1}
====
INIT Init
NEXT Next
INVARIANT Inv
