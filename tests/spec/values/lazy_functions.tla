---- MODULE lazy_functions ----
\* A function constructor stays lazy where it is applied (§6.6): only the
\* argument's value is computed, so 6 \div 0 is never evaluated.
EXTENDS Integers, Sequences, TLC
f == [x \in 0..2 |-> 6 \div x]
Op(n) == [x \in 0..n |-> 6 \div x]
r[x \in 0..2] == 6 \div x
ASSUME PrintT([x \in 0..2 |-> 6 \div x][1])
ASSUME PrintT(f[2])
ASSUME PrintT(LET g == [x \in 0..2 |-> 6 \div x] IN g[3 - 1])
ASSUME PrintT(Op(2)[1])
ASSUME PrintT((IF TRUE THEN [x \in 0..2 |-> 6 \div x] ELSE <<>>)[2])
ASSUME PrintT([y \in 1..2 |-> [x \in 0..2 |-> 6 \div x]][1][2])
ASSUME PrintT(r[3 - 1])
\* Printed, a function that cannot be computed in full shows its definition.
ASSUME PrintT(r)
ASSUME PrintT([x \in Nat |-> x])
ASSUME PrintT([x, y \in Nat |-> x])
ASSUME PrintT([x \in Nat, y \in {1} |-> x])
ASSUME PrintT([<<a, b>> \in Nat \X Nat |-> a])
ASSUME PrintT([[x \in Nat |-> x] EXCEPT ![1] = 5])
ASSUME PrintT(<<[x \in Nat |-> x], [s \in Seq({1}) |-> 1]>>)
ASSUME PrintT([a |-> [x \in Nat |-> x]])
====
