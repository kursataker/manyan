---- MODULE tautology_always ----
\* The negation of <>TRUE => <>[]TRUE is <>TRUE /\ []<>FALSE, which TLC
\* does not fold to FALSE: it is checked, and holds, rather than being
\* reported as a tautology.
VARIABLE a
Init == a = 0
Next == UNCHANGED a
Spec == Init /\ [][Next]_a
AlwaysTrue == <>TRUE => <>[]TRUE
====
SPECIFICATION Spec
PROPERTY AlwaysTrue
