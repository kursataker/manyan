---- MODULE symmetry ----
\* Clients request and are granted resources; clients are interchangeable
\* model values, so a symmetry set folds equivalent states.
EXTENDS Naturals, FiniteSets, TLC
CONSTANTS Clients, Resources, Max, Free
VARIABLES owner, asked
Init == owner = [r \in Resources |-> Free] /\ asked = [c \in Clients |-> {}]
Held(c) == {r \in Resources : owner[r] = c}
Ask(c, S) == /\ asked[c] = {} /\ S # {} /\ Cardinality(Held(c)) + Cardinality(S) <= Max
             /\ \A r \in S : owner[r] # c
             /\ asked' = [asked EXCEPT ![c] = S] /\ UNCHANGED owner
Grant(c, r) == /\ r \in asked[c] /\ owner[r] = Free
               /\ owner' = [owner EXCEPT ![r] = c]
               /\ asked' = [asked EXCEPT ![c] = @ \ {r}]
Release(c) == /\ Held(c) # {}
              /\ owner' = [r \in Resources |-> IF owner[r] = c THEN Free ELSE owner[r]]
              /\ asked' = [asked EXCEPT ![c] = {}]
Next == \E c \in Clients : (\E S \in SUBSET Resources : Ask(c, S)) \/ (\E r \in Resources : Grant(c, r)) \/ Release(c)
Exclusive == \A r \in Resources : owner[r] = Free \/ owner[r] \in Clients
Bounded == \A c \in Clients : Cardinality(Held(c)) <= Max
Sym == Permutations(Clients)
====
CONSTANTS Clients = {c1, c2, c3, c4} Resources = {r1, r2, r3, r4} Max = 2 Free = Free
INIT Init
NEXT Next
INVARIANT Exclusive
INVARIANT Bounded
SYMMETRY Sym
