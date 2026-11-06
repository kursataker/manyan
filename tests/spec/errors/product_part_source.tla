---- MODULE product_part_source ----
\* Sorting the sets of a UNION inside a product fails: TLC names the
\* product, the UNION and its set of sets.
EXTENDS Naturals
VARIABLE x
Init == x \in {1} \X UNION {{1}, Nat}
Next == UNCHANGED x
====
INIT Init
NEXT Next
