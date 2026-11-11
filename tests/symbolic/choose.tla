---- MODULE choose ----
\* CHOOSE picks an element that satisfies the predicate.
EXTENDS Integers
VARIABLES
    \* @type: Set(Int);
    s,
    \* @type: Int;
    m
Init == s = {3, 5} /\ m = 0
Next == /\ s' = s \cup {m + 1}
        /\ m' = CHOOSE v \in s' : \A w \in s' : v >= w
Inv == m < 5
====
INIT Init
NEXT Next
INVARIANT Inv
