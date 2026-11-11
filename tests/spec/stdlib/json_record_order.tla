---- MODULE json_record_order ----
\* Json's Java writes a record TLC has not normalized as it was written,
\* one stored in a state in TLC's order.
EXTENDS Json, TLC
VARIABLE x
Init == x = [q |-> 1, p |-> 2, a |-> 3]
Next == UNCHANGED x
Inv == /\ PrintT(ToJson([b |-> 1, a |-> 2, zz |-> 3, c |-> 4]))
       /\ PrintT(ToJson(x))
       /\ PrintT(ToJsonObject([zz |-> x, b |-> 2]))
====
INIT Init
NEXT Next
INVARIANT Inv
