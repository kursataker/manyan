---- MODULE diehard ----
\* Measure 4 gallons with a 3-gallon jug and a 5-gallon jug: six steps.
EXTENDS Naturals
VARIABLES
    \* @type: Int;
    small,
    \* @type: Int;
    big
Min(m, n) == IF m < n THEN m ELSE n
Init == small = 0 /\ big = 0
Next == \/ small' = 3 /\ big' = big
        \/ big' = 5 /\ small' = small
        \/ small' = 0 /\ big' = big
        \/ big' = 0 /\ small' = small
        \/ LET p == Min(small, 5 - big) IN small' = small - p /\ big' = big + p
        \/ LET p == Min(big, 3 - small) IN big' = big - p /\ small' = small + p
TypeOK == small \in 0..3 /\ big \in 0..5
NotSolved == big # 4
====
INIT Init
NEXT Next
INVARIANT TypeOK NotSolved
