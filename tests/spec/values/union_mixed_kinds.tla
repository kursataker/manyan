---- MODULE union_mixed_kinds ----
\* A union with a set TLC keeps as its definition, here a product, is
\* kept as one too: membership is tested in each part, the enumerated
\* first, so its elements need not compare (AB2 from TLAplusFun).
EXTENDS Naturals, TLC
CONSTANTS Bad, Data
ASSUME Bad \notin (Data \X {0, 1}) \cup {0, 1}
ASSUME PrintT(\A d \in Data : d \notin {0, 1} \cup (Data \X {0, 1}))
VARIABLE x
Init == x = 0
Next == UNCHANGED x
====
CONSTANTS Bad = Bad Data = {d1, d2}
INIT Init
NEXT Next
