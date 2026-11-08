---- MODULE filter_non_boolean ----
\* A filter whose predicate is not boolean: TLC names what it was.
EXTENDS Integers, TLC
ASSUME PrintT({x \in {1} : "a"})
====
