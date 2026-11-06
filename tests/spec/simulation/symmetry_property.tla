---- MODULE symmetry_property ----
\* Under symmetry, a simulation checks a property's actions on each
\* behavior's own steps, behavior after behavior.
\* options: -simulate num=3 -depth 3 -seed 318
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
