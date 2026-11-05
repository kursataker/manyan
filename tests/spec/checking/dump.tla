---- MODULE dump ----
\* options: -dump dot,actionlabels graph
\* The state graph in dot, with the steps' actions: a state outside the
\* constraint is left out, and only states steps leave are ranked.
EXTENDS Naturals
VARIABLES x, s
Init == x \in {0, 1, 7} /\ s = "a\"b\\"
Inc == x < 3 /\ x' = x + 1 /\ UNCHANGED s
Reset == x = 2 /\ x' = 0 /\ s' = <<"q", {1}>>
Stay == x # 7 /\ UNCHANGED <<x, s>>
Next == Inc \/ Reset \/ Stay
Small == x < 3 \/ x = 7
====
INIT Init
NEXT Next
CONSTRAINT Small
CHECK_DEADLOCK FALSE
