---- MODULE values ----
\* Records, sequences and symmetry in random behaviors.
\* options: -simulate -depth 40 -seed 11
EXTENDS Integers, Sequences, TLC
CONSTANT P
VARIABLES r, s, q
Init == r \in [a : {1, 2}, b : {"p", "q"}] /\ s = {} /\ q = <<>>
Next == \/ r' = [r EXCEPT !.a = r.a + 1] /\ UNCHANGED <<s, q>>
        \/ \E p \in P : s' = s \cup {p} /\ q' = Append(q, p) /\ UNCHANGED r
        \/ q /= <<>> /\ q' = Tail(q) /\ UNCHANGED <<r, s>>
Inv == r.a < 6
Sym == Permutations(P)
====
CONSTANT P = {p1, p2, p3}
INIT Init
NEXT Next
INVARIANT Inv
SYMMETRY Sym
