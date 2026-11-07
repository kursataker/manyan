---- MODULE union_kept ----
\* UNION of sets that are not all sets of their elements, which TLC keeps
\* as it is: membership, printing, counting, comparing and quantifying.
\* Found by tlaplus/Examples' MCDisruptor_MPMC, whose TypeOk is a record
\* set with a field UNION {[0..3 -> SUBSET Readers]}.
EXTENDS Integers, FiniteSets, TLC
VARIABLE x
U == UNION {1..3, {5}}
F == UNION {[{1, 2} -> {7, 8}]}
R == UNION {[a : {1, 2}, b : {"x"}], [a : {3}, b : {"y"}]}
ASSUME PrintT(<<U, Cardinality(U), 2 \in U, 4 \in U, U = {1, 2, 3, 5}>>)
ASSUME PrintT(<<[i \in {1, 2} |-> 7] \in F, [i \in {1, 2} |-> 9] \in F, Cardinality(F)>>)
ASSUME PrintT(<<[a |-> 3, b |-> "y"] \in R, [a |-> 3, b |-> "x"] \in R, R>>)
ASSUME PrintT(<<{y \in U : y > 1}, [y \in UNION {1..2} |-> y * 10], UNION {SUBSET {1, 2}}>>)
ASSUME PrintT(<<UNION {}, UNION {{}}, (UNION {{1}, {2}}) \cup (UNION {1..2})>>)
Init == x \in UNION {1..2, {4}}
Next == x' \in UNION {{x}, (x + 1)..(x + 1)} /\ x < 6
Inv == x \in UNION {1..5}
====
INIT Init
NEXT Next
INVARIANT Inv
