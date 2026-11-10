---- MODULE tlc_cache_state ----
\* A simulation keeps TLCCache's value of a state's expression in the state:
\* each action of the state finds it, an initial state keeps it for every
\* behavior from it, a later state only for its behavior.
\* options: -simulate num=4 -depth 6 -seed 1
EXTENDS Naturals, TLC, TLCExt
VARIABLE x
ASSUME TLCSet(1, 0)
Inc(v) == CASE TLCSet(1, TLCGet(1) + 1) -> v
C == TLCCache(Inc(x), x)
Init == x \in {0, 10}
A == x' = x + 1 /\ C >= 0 /\ PrintT(<<"A", x, TLCGet(1)>>)
B == x' = x + 2 /\ C >= 0 /\ PrintT(<<"B", x, TLCGet(1)>>)
Next == (x < 12 /\ x # 3) /\ (A \/ B)
====
INIT Init
NEXT Next
CHECK_DEADLOCK FALSE
