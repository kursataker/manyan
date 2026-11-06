---- MODULE subscript_forms ----
\* A subscript may be a record, a set or a function, as SANY parses it;
\* TLC finds no variable in them to warn of.
EXTENDS Naturals
VARIABLES x, y
Init == x = 0 /\ y = 0
Next == x' = 1 /\ UNCHANGED y
Spec == Init /\ [][Next]_[a |-> x, b |-> y] /\ WF_{x, y}(Next) /\ SF_[i \in {1} |-> x](Next)
====
SPECIFICATION Spec
