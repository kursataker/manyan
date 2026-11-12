\* options: -length 2
---- MODULE gen_generic ----
\* Gen in an operator whose type each call instantiates: Make makes a set,
\* a sequence and a function.
EXTENDS Integers, Sequences, FiniteSets, Apalache
VARIABLES
    \* @type: Set(Int);
    s,
    \* @type: Seq(Bool);
    q,
    \* @type: Int -> Str;
    f
Make(n) == Gen(n)
Twice(n) == Make(n)
Init == s = Make(3) /\ q = Twice(2) /\ f = (LET G(k) == Make(k) IN G(1)) /\ Cardinality(s) = 3
Next == s' = Make(2) /\ UNCHANGED <<q, f>>
Inv == ~(Cardinality(s) = 2 /\ Len(q) = 2 /\ q[2] /\ DOMAIN f = {4})
====
INIT Init
NEXT Next
INVARIANT Inv
