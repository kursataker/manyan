---- MODULE queue ----
\* A bounded producer/consumer channel: sequences and records.
EXTENDS Naturals, Sequences
CONSTANTS Msgs, Cap
VARIABLES q, sent, got
Init == q = <<>> /\ sent = 0 /\ got = 0
Send(m) == Len(q) < Cap /\ q' = Append(q, [val |-> m, seq |-> sent % 4]) /\ sent' = (sent + 1) % 8 /\ UNCHANGED got
Recv == q # <<>> /\ q' = Tail(q) /\ got' = (got + Head(q).val) % 7 /\ UNCHANGED sent
Next == (\E m \in Msgs : Send(m)) \/ Recv
TypeOK == Len(q) <= Cap /\ \A i \in 1..Len(q) : q[i].val \in Msgs
====
CONSTANTS Msgs = {1, 2, 3} Cap = 7
INIT Init
NEXT Next
INVARIANT TypeOK
