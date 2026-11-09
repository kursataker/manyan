---- MODULE view_action ----
\* Under a VIEW the successor kept for a step may be another state with
\* its view: a property's action is evaluated, as TLC does, on the steps
\* themselves, each of which here increases y. Found by tlaplus/Examples'
\* MCDistributedReplicatedLog.
EXTENDS Naturals
VARIABLES x, y
Init == x = 0 /\ y = 0
Next == x' = 1 - x /\ y' = y + 1
Spec == Init /\ [][Next]_<<x, y>> /\ WF_<<x, y>>(Next)
Grows == <>[][y' > y]_y
V == x
====
SPECIFICATION Spec
PROPERTY Grows
VIEW V
