---- MODULE filter_state_infinite ----
\* An initial state holds a filter of an infinite set: fingerprinting it
\* fails enumerating the product, and TLC prints the filter as it is.
EXTENDS Naturals
VARIABLE x
Init == x = {<<a, b>> \in Nat \X Nat : a > b}
Next == UNCHANGED x
====
INIT Init
NEXT Next
