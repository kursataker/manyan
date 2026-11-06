---- MODULE union_mixed_kinds ----
\* Of two enumerated sets whose elements do not compare, TLC makes the
\* union, looking for each of the second's elements in the first.
EXTENDS TLC
ASSUME PrintT({<<1, 2>>} \cup {0, 1})
====
