---- MODULE implied_init_conjunct ----
\* Each conjunct of a property is checked as what it is: here a state
\* predicate, named by its location, with an invariant beside it.
EXTENDS Naturals
VARIABLE x
Init == x \in {0, 1}
Next == x' = 1 - x
P == x \in {0, 1} /\ [](x < 2) /\ x = 1
====
INIT Init
NEXT Next
PROPERTY P
