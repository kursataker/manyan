---- MODULE recursive_outside_domain ----
\* A function definition stays lazy (§6.6); computed in full it fails, so it
\* prints as its definition.
EXTENDS Integers, TLC
r[x \in 0..2] == 6 \div x
ASSUME PrintT(r[3])
====
