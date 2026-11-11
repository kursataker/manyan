\* options: -length 2
---- MODULE gen ----
\* Gen(n): any value of its type, with sets, functions and sequences of up
\* to n elements; each call and each step makes another.
EXTENDS Integers, Sequences, FiniteSets, Apalache
VARIABLES
    \* @type: Set(Int);
    s,
    \* @type: Seq(Bool);
    q,
    \* @type: { a: Int, b: <<Int, Bool>> };
    r,
    \* @type: Int -> Int;
    f
Init == s = Gen(3) /\ q = Gen(2) /\ r = Gen(1) /\ f = Gen(2) /\ Cardinality(s) = 3
Next ==
    /\ s' = Gen(2) /\ \A x \in s': x \notin s
    /\ f' = Gen(3) /\ Cardinality(DOMAIN f') = 3
    /\ UNCHANGED <<q, r>>
Inv == ~(Len(q) = 2 /\ q[1] /\ ~q[2] /\ r.b[2] /\ r.a = 7 /\ Cardinality(DOMAIN f) = 3 /\ 1 \in DOMAIN f /\ f[1] = 5)
====
INIT Init
NEXT Next
INVARIANT Inv
