---- MODULE compare_source ----
\* An error comparing the parts of a set, record or tuple an expression
\* made names each of them in the fingerprint stack trace, outermost last;
\* sorting a set, TLC compares each new element with those before it.
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next == x' = x
Inv == {[a |-> {1}], [a |-> {"s"}]} = {}
====
INIT Init
NEXT Next
INVARIANT Inv
