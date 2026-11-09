---- MODULE substitution_in_quantifier ----
\* A state-level substitution of an instance, first read in a
\* quantifier's body, keeps its value for the state after the body's
\* memory is dropped. Found by tlaplus/Examples' Peterson and EWD998ChanID.
EXTENDS Naturals
VARIABLE pc
Spec == pc = <<"a0", "a0">> /\ [][\E i \in 1..2 : pc' = [pc EXCEPT ![i] = IF @ = "a0" THEN "cs" ELSE "a0"]]_pc
T(label) == CASE label = "a0" -> "l0" [] label = "cs" -> "cs"
L == INSTANCE Mutex WITH pc <- [p \in 1..2 |-> T(pc[p])],
                         lock <- IF \E p \in 1..2 : pc[p] = "cs" THEN 0 ELSE 1
LSpec == L!Spec
====
SPECIFICATION Spec
PROPERTY LSpec
