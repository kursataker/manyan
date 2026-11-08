---- MODULE instance_mapping_violated ----
\* A substitution computed from the state, read through an instance, has
\* the state's value in each state: Get drops the last element instead of
\* the first, which the queue's Next does not allow.
EXTENDS Naturals, Sequences
VARIABLES buf, len
Q == INSTANCE Queue WITH q <- [i \in 1..len |-> buf[i]], Max <- 3
Init == buf = <<0, 0, 0>> /\ len = 0
Put(m) == len < 3 /\ buf' = [buf EXCEPT ![len + 1] = m] /\ len' = len + 1
Get == len > 0 /\ buf' = buf /\ len' = len - 1
Next == (\E m \in 1..2 : Put(m)) \/ Get
Spec == Init /\ [][Next]_<<buf, len>>
Inv == Q!TypeOK /\ Q!Size = len
Refines == Q!Spec
====
SPECIFICATION Spec
INVARIANT Inv
PROPERTY Refines
