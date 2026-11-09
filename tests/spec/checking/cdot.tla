---- MODULE cdot ----
\* options: -Dtlc2.tool.impl.Tool.cdot=true
\* TLC's incomplete action composition, behind its Java property: the
\* states in between count as generated, the conjuncts after A \cdot B
\* follow both A and B, and ENABLED and properties compose too. Found by
\* tlaplus/Examples' MCCRDT.
EXTENDS Naturals
VARIABLES x, y
Inc == x' = x + 1 /\ y' = y
Twice == x' \in {x, 2 * x} /\ y' = y + 1
Next == (Inc \cdot Twice) /\ x' < 10
Spec == x = 1 /\ y = 0 /\ [][Next]_<<x, y>> /\ WF_<<x, y>>(Next)
Step == [][Inc \cdot Twice]_<<x, y>>
Grows == <>(y = 3)
====
SPECIFICATION Spec
PROPERTY Step Grows
CHECK_DEADLOCK FALSE
