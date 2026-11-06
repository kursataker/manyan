---- MODULE symmetry_action ----
\* Under symmetry a successor is kept as the state found first with its
\* fingerprint, here a permutation of it, so a property's action is
\* checked on the step itself: <<p2, 2>> is found before <<p1, 2>>, which
\* <<p1, 1>> steps to keeping x. The property is an instance's, which the
\* tableau checks.
EXTENDS Naturals, TLC
CONSTANT P
VARIABLES x, y
Other(p) == CHOOSE q \in P : q # p
Init == x \in P /\ y = 0
Next == \/ y = 0 /\ y' = 1 /\ x' = x
        \/ y = 0 /\ y' = 2 /\ x' = Other(x)
        \/ y = 1 /\ y' = 2 /\ x' = x
        \/ UNCHANGED <<x, y>>
K == INSTANCE Keep
Keeps == K!Spec
Sym == Permutations(P)
====
CONSTANT P = {p1, p2}
INIT Init
NEXT Next
PROPERTY Keeps
SYMMETRY Sym
