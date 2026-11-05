---- MODULE log ----
\* A leader appends to its log; followers copy a longer prefix of it.
EXTENDS Naturals, Sequences
CONSTANTS Followers, Values, MaxLen
VARIABLES leader, logs
Init == leader = <<>> /\ logs = [f \in Followers |-> <<>>]
Append1 == /\ Len(leader) < MaxLen
           /\ \E v \in Values : leader' = Append(leader, v)
           /\ UNCHANGED logs
Sync(f) == /\ Len(logs[f]) < Len(leader)
           /\ \E n \in (Len(logs[f]) + 1)..Len(leader) :
                logs' = [logs EXCEPT ![f] = SubSeq(leader, 1, n)]
           /\ UNCHANGED leader
Done == Len(leader) = MaxLen /\ (\A f \in Followers : logs[f] = leader) /\ UNCHANGED <<leader, logs>>
Next == Append1 \/ (\E f \in Followers : Sync(f)) \/ Done
Prefix == \A f \in Followers :
            /\ Len(logs[f]) <= Len(leader)
            /\ logs[f] = SubSeq(leader, 1, Len(logs[f]))
====
CONSTANTS Followers = {f1, f2, f3} Values = {a, b} MaxLen = 6
INIT Init
NEXT Next
INVARIANT Prefix
