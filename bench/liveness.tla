---- MODULE liveness ----
\* A token ring under weak fairness: each process eventually holds the
\* token, and a request is eventually served (the liveness checker).
EXTENDS Naturals
CONSTANTS N, K
Procs == 0..N-1
VARIABLES token, work, want
vars == <<token, work, want>>
Init == token = 0 /\ work = [p \in Procs |-> 0] /\ want = {}
Work == token \in want /\ work' = [work EXCEPT ![token] = (@ + 1) % K] /\ want' = want \ {token} /\ UNCHANGED token
Pass == token \notin want /\ token' = (token + 1) % N /\ UNCHANGED <<work, want>>
Ask(p) == p # token /\ p \notin want /\ want' = want \cup {p} /\ UNCHANGED <<token, work>>
Next == Work \/ Pass \/ \E p \in Procs : Ask(p)
Spec == Init /\ [][Next]_vars /\ WF_vars(Pass) /\ WF_vars(Work)
Visits == \A p \in Procs : []<>(token = p)
Served == \A p \in Procs : (p \in want) ~> (p \notin want)
====
CONSTANTS N = 5 K = 3
SPECIFICATION Spec
PROPERTY Visits
PROPERTY Served
