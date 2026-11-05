---- MODULE binding_cache ----
\* A binding read from a state that is still changing is read again.
EXTENDS Naturals
VARIABLES x, y
Twice(a) == 2 * a
Init == x \in 0..2 /\ y = Twice(x)
Up(v) == v < 4 /\ v' = v + 1 /\ v' > v /\ Twice(v') = Twice(v) + 2
Next == \/ Up(x) /\ UNCHANGED y
        \/ LET n == x' IN x' \in {x + 1, x + 2} /\ n < 5 /\ y' = Twice(n)
Inv == y % 2 = 0 /\ x <= 4
====
INIT Init
NEXT Next
INVARIANT Inv
