---- MODULE implied_init ----
\* TLC's simulation ignores a property that is a state predicate.
\* options: -simulate num=3 -depth 4 -seed 1
EXTENDS Naturals
VARIABLE x
Init == x \in {0, 1}
Next == x' = 1 - x
P == x = 0
====
INIT Init
NEXT Next
PROPERTY P
