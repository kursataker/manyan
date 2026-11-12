---- MODULE union_not_sets ----
\* UNION of a set whose elements are not sets, as TLC enumerates it.
EXTENDS Integers, TLC
ASSUME PrintT(UNION ({{}} \X {0}))
====
