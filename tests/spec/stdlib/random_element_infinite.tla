---- MODULE random_element_infinite ----
\* RandomElement of a set of functions draws each value from the range:
\* from Nat, which TLC cannot enumerate, it fails naming Nat.
EXTENDS Naturals, TLC
ASSUME PrintT(RandomElement([{1} -> Nat]))
VARIABLE x
Init == x = 0
Next == UNCHANGED x
====
INIT Init
NEXT Next
