---- MODULE set_ops ----
\* Intersection, difference, subsets and filters of symbolic sets.
EXTENDS Integers, FiniteSets
VARIABLES
    \* @type: Set(Int);
    a,
    \* @type: Set(Int);
    b
Init == a \in SUBSET (1..4) /\ b \in SUBSET (1..4)
Next == UNCHANGED <<a, b>>
Laws == /\ a \cap b \subseteq a
        /\ (a \ b) \cup (a \cap b) = a
        /\ {x \in a : x > 2} \subseteq a
        /\ Cardinality(a \cup b) + Cardinality(a \cap b) = Cardinality(a) + Cardinality(b)
NotBoth == ~(a = {1, 3} /\ b = {2, 3} /\ a \cap b = {3})
====
INIT Init
NEXT Next
INVARIANT Laws NotBoth
