---- MODULE is_finite_set ----
\* IsFiniteSet as TLC's isFinite answers it: a function is a finite set,
\* a filter is finite when the set it filters is, and a product, a set of
\* records, functions or sequences when it is empty or has finite parts.
EXTENDS Integers, FiniteSets, Sequences, TLC
VARIABLE v
Init == v = 0
Next == UNCHANGED v
ASSUME PrintT(<<IsFiniteSet({x \in 1..10 : x > 20}), IsFiniteSet(<<1, 2>>),
                IsFiniteSet([a |-> 1]), IsFiniteSet(Nat \ {1}), IsFiniteSet({1} \ Nat),
                IsFiniteSet(Nat \cap {1}), IsFiniteSet([a : {}, b : Nat]),
                IsFiniteSet(UNION {{1}, 2..3}), IsFiniteSet([{} -> Nat]), IsFiniteSet(Seq({}))>>)
ASSUME PrintT(<<IsFiniteSet([Nat -> {1}]), IsFiniteSet([Nat -> {}]), IsFiniteSet(Nat \X {}),
                IsFiniteSet([a : Nat]), IsFiniteSet([{1} -> Nat]), IsFiniteSet(Seq({1}))>>)
====
INIT Init
NEXT Next
