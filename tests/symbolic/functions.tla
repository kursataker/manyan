---- MODULE functions ----
\* Processes move through a program counter; two reach "cs" together.
EXTENDS Integers
CONSTANTS
    \* @type: Set(Int);
    Procs
VARIABLES
    \* @type: Int -> Str;
    pc
Init == pc = [p \in Procs |-> "idle"]
Try(p) == pc[p] = "idle" /\ pc' = [pc EXCEPT ![p] = "wait"]
Enter(p) == pc[p] = "wait" /\ pc' = [pc EXCEPT ![p] = "cs"]
Leave(p) == pc[p] = "cs" /\ pc' = [pc EXCEPT ![p] = "idle"]
Next == \E p \in Procs : Try(p) \/ Enter(p) \/ Leave(p)
Mutex == \A p, q \in Procs : p /= q => ~(pc[p] = "cs" /\ pc[q] = "cs")
====
CONSTANTS Procs = {1, 2, 3}
INIT Init
NEXT Next
INVARIANT Mutex
