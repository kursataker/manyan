---- MODULE refinement ----
\* A ring buffer refines a queue: the queue is an INSTANCE of a nested
\* module under a refinement mapping, checked as a property.
EXTENDS Naturals, Sequences
CONSTANTS Msgs, K
---- MODULE Queue ----
VARIABLE q
Init == q = <<>>
Next == \/ Len(q) < K /\ \E m \in Msgs : q' = Append(q, m)
        \/ q # <<>> /\ q' = Tail(q)
Spec == Init /\ [][Next]_q
====
VARIABLES buf, head, len
Init == buf = [i \in 0..K-1 |-> CHOOSE m \in Msgs : TRUE] /\ head = 0 /\ len = 0
Put(m) == len < K /\ buf' = [buf EXCEPT ![(head + len) % K] = m] /\ len' = len + 1 /\ UNCHANGED head
Get == len > 0 /\ head' = (head + 1) % K /\ len' = len - 1 /\ UNCHANGED buf
Next == (\E m \in Msgs : Put(m)) \/ Get
Spec == Init /\ [][Next]_<<buf, head, len>>
Abstract == INSTANCE Queue WITH q <- [i \in 1..len |-> buf[(head + i - 1) % K]]
Refines == Abstract!Spec
====
CONSTANTS Msgs = {1, 2, 3} K = 6
SPECIFICATION Spec
PROPERTY Refines
