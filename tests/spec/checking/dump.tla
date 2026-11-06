---- MODULE dump ----
\* options: -dump dot,actionlabels graph
\* The state graph in dot, with the steps' actions: a state outside the
\* constraint is left out. One state a rank, as TLC orders a rank's states
\* at random.
EXTENDS Naturals
VARIABLES x, s
Init == x = 0 /\ s = "a\"b\\"
Inc == x < 3 /\ x' = x + 1 /\ UNCHANGED s
Reset == x = 2 /\ x' = 0 /\ s' = <<"q", {1}>>
Stay == UNCHANGED <<x, s>>
Next == Inc \/ Reset \/ Stay
Small == x < 3
====
INIT Init
NEXT Next
CONSTRAINT Small
CHECK_DEADLOCK FALSE
