---- MODULE infinite_enum ----
EXTENDS Naturals, TLC
ASSUME PrintT(\E x \in Nat : x > 5)
====
