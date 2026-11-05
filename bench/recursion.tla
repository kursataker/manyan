---- MODULE recursion ----
\* Digits change one at a time; invariants compute over them with
\* recursive function definitions and CHOOSE.
EXTENDS Naturals
CONSTANTS N, B
VARIABLE d
Init == d = [i \in 1..N |-> 0]
Next == \E i \in 1..N : d' = [d EXCEPT ![i] = (@ + 1) % B] \/ d' = [d EXCEPT ![i] = (@ + B - 1) % B]
Sum[i \in 0..N] == IF i = 0 THEN 0 ELSE Sum[i - 1] + d[i]
Value[i \in 0..N] == IF i = 0 THEN 0 ELSE Value[i - 1] * B + d[i]
Max == CHOOSE m \in {d[i] : i \in 1..N} : \A i \in 1..N : d[i] <= m
fact[n \in 0..N] == IF n = 0 THEN 1 ELSE n * fact[n - 1]
Bounds == Sum[N] <= N * Max /\ Value[N] < fact[N] * B ^ N
Digits == d \in [1..N -> 0..B-1]
====
CONSTANTS N = 7 B = 5
INIT Init
NEXT Next
INVARIANT Bounds
INVARIANT Digits
