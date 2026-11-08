---- MODULE tlc_ext_cache ----
\* TLCCache keeps a constant expression's value for each value of its
\* closure, wherever it is evaluated (ConstantContextTLCCache); an
\* expression of a state is evaluated each time.
EXTENDS Naturals, TLC, TLCExt
ASSUME TLCSet(1, 0)
Inc(v) == CASE TLCSet(1, TLCGet(1) + 1) -> v
A == TLCCache(Inc({1, 2, 3}), {1})
B == TLCCache(Inc({i \in 1..5 : i \in A}), {2})
VARIABLE x
ASSUME 3 \in A /\ 3 \in B
Init == x = 0 /\ A = B
Next == x < 2 /\ x' = TLCCache(Inc(x + 1), x) /\ 1 \in A /\ PrintT(TLCGet(1))
====
INIT Init
NEXT Next
CHECK_DEADLOCK FALSE
