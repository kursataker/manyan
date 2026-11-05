---- MODULE params_order ----
\* TLC computes a function of several bounds with the first bound varying
\* fastest; PrintT prints it normalized.
EXTENDS Integers, TLC
ASSUME LET f == [x \in 1..2, y \in {"b", "a"} |-> PrintT(<<x, y>>)] IN f = f
ASSUME PrintT([x \in 1..2, y \in 1..3 |-> x + y])
ASSUME PrintT([p \in (1..2) \X (1..2) |-> PrintT(p)])
====
