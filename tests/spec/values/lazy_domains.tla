---- MODULE lazy_domains ----
\* As in TLC, a filter or function is computed at once only over a set of its elements or an interval.
EXTENDS Integers, FiniteSets, TLC
ASSUME PrintT(1 \in {x \in UNION {0..0, {1}} : 6 \div x > 1})
ASSUME PrintT([x \in {0, 1} \X {1} |-> 6 \div x[1]][<<1, 1>>])
ASSUME PrintT([x \in SUBSET {1} |-> 6 \div Cardinality(x)][{1}])
ASSUME PrintT(LET f == [x \in SUBSET {1} |-> 6 \div Cardinality(x)] IN <<f>>)
ASSUME PrintT([x \in 3 |-> 1])
ASSUME PrintT([x \in Nat |-> \b101])
ASSUME PrintT(CHOOSE x \in {y \in 0..3 : y > 1} : TRUE)
ASSUME PrintT(1 \in {x \in {0, 1} \ {} : 6 \div x > 1} \/ TRUE)
====
