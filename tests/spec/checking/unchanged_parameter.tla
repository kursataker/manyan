---- MODULE unchanged_parameter ----
\* UNCHANGED of an operator's parameter leaves the variable its argument
\* names unchanged, assigning it (BlockingQueueSplit from lemmy/BlockingQueue).
VARIABLES a, b
Notify(ws) == \/ ws = {} /\ UNCHANGED ws
              \/ ws # {} /\ \E x \in ws : ws' = ws \ {x}
Wait(ws, t) == ws' = ws \cup {t}
Init == a = {} /\ b = {}
Next == \/ Notify(a) /\ Wait(b, 1)
        \/ Notify(b) /\ Wait(a, 2)
====
INIT Init
NEXT Next
