---- MODULE finiteset_ops ----
EXTENDS Naturals, FiniteSets, TLC
ASSUME PrintT(<<Cardinality({}), Cardinality({1, 2, 2}), Cardinality(SUBSET (1..4)), IsFiniteSet({1}), Cardinality([a : 1..3, b : 1..2])>>)
====
