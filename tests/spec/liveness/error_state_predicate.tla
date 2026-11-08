---- MODULE error_state_predicate ----
\* A state predicate of a property without a tableau that fails: TLC
\* evaluates it on the state it expands, not on the initial state when it
\* finds it, nor on the successors, so the error names x = 0.
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next == x' = (x + 1) % 3
P == <>[](x \in 1)
====
INIT Init
NEXT Next
PROPERTY P
