---- MODULE assert_initial ----
\* A failed Assert in Init is TLC's general error, not an assertion
\* violation, and its state is not printed.
EXTENDS Naturals, TLC
VARIABLE x
Init == x \in {0, 1} /\ Assert(x = 0, "one")
Next == x < 3 /\ x' = x + 1
====
INIT Init
NEXT Next
