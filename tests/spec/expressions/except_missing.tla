---- MODULE except_missing ----
\* EXCEPT on a key outside the domain warns once per message and changes nothing.
EXTENDS Integers, Sequences, TLC
ASSUME \A i \in 1..3 : PrintT([<<1>> EXCEPT ![5] = i])
ASSUME PrintT([[a |-> <<1>>] EXCEPT !.a[3] = 0])
ASSUME PrintT([[a |-> <<1>>] EXCEPT !.c[3] = 0])
ASSUME PrintT([[a |-> <<1>>] EXCEPT !.a[1] = @ + 1])
VARIABLE x
Init == x = 0
Next == x' = [<<x>> EXCEPT ![2] = 1][1]
====
INIT Init
NEXT Next
