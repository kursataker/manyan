---- MODULE tlcget_generated_post ----
\* A postcondition reads the search's count of states generated, of a
\* search that ends as of one that finds an invariant violated.
EXTENDS Naturals, TLC
VARIABLE x
Init == x \in {0, 1}
Next == \/ x' = (x + 1) % 5
        \/ x' = (x + 2) % 5
Post == PrintT(<<"generated", TLCGet("generated"), TLCGet("distinct")>>)
====
INIT Init
NEXT Next
POSTCONDITION Post
