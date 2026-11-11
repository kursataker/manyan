---- MODULE apalache_module ----
\* Apalache's module: := assigns, FunAsSeq makes a sequence of a function.
EXTENDS Integers, Sequences, Apalache
VARIABLES
    \* @type: Seq(Int);
    s,
    \* @type: Int;
    k
Init == s := <<>> /\ k := 0
Next == k < 3 /\ k' := k + 1 /\ s' := FunAsSeq([i \in 1..4 |-> i * i], k + 1, 4)
Inv == Len(s) < 3 \/ s[3] /= 9
====
INIT Init
NEXT Next
INVARIANT Inv
