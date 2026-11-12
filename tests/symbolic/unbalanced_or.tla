---- MODULE unbalanced_or ----
\* Apalache finds what x' = e assigns by the syntax alone, and refuses a
\* disjunct that assigns other variables than its siblings, though TLC
\* takes either here.
\* options: -length 2
VARIABLE
    \* @type: Int;
    x,
    \* @type: Int;
    y
Init == x = 0 /\ y = 0
Next == (x' = 1 \/ y' = 2) /\ x' = 1 /\ y' = 2
Inv == x # 1
====
INIT Init
NEXT Next
INVARIANT Inv
