---- MODULE union_held_kinds ----
\* A union TLC keeps as its definition, enumerated, is the elements of its
\* parts, the enumerated first, sorted: elements that do not compare fail
\* there.
EXTENDS TLC
ASSUME PrintT([a : {1}] \cup {2})
====
