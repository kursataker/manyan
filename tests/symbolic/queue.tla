---- MODULE queue ----
\* A bounded queue: sent and received values stay in order.
EXTENDS Integers, Sequences
VARIABLES
    \* @type: Seq(Int);
    q,
    \* @type: Int;
    next,
    \* @type: Int;
    got
Init == q = <<>> /\ next = 0 /\ got = -1
Send == Len(q) < 3 /\ q' = Append(q, next) /\ next' = next + 1 /\ UNCHANGED got
Recv == Len(q) > 0 /\ got' = Head(q) /\ q' = Tail(q) /\ UNCHANGED next
Next == Send \/ Recv
InOrder == \A i \in 1..Len(q) : q[i] = next - Len(q) + i - 1
Small == got < 4
====
INIT Init
NEXT Next
INVARIANT InOrder Small
