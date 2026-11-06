---- MODULE error_initial_then_invariant ----
\* After an error in a property on an initial state TLC replays Init,
\* checking its invariant: an error in a set operation it keeps as its
\* definition is one it cannot fingerprint (CBCCasperSpec from
\* Isaac-DeFrain/TLAplusFun).
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next == UNCHANGED x
Spec == Init /\ [][Next]_x
P == <>(x = "s")
Inv == \A m \in (x \ {1}) : TRUE
====
SPECIFICATION Spec
PROPERTY P
INVARIANT Inv
