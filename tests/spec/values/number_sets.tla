---- MODULE number_sets ----
\* Nat and Int are ordered, Nat first, and compared without enumerating them.
EXTENDS Integers, TLC
ASSUME PrintT(<<{Int, Nat}, {{Int}, {Nat}}, Nat = Int, {Int} = {Int}>>)
ASSUME PrintT(CHOOSE s \in {{Int}, {Nat}} : TRUE)
====
