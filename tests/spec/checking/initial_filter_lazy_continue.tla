---- MODULE initial_filter_lazy_continue ----
\* options: -continue
\* With -continue TLC reports the initial states violating an invariant
\* again as it replays Init after the error.
EXTENDS Naturals
VARIABLE x
Init == x \in {r \in [a : 1..3] : 1 \div (3 - r.a) >= 0}
Next == UNCHANGED x
Inv == x.a > 5
====
INIT Init
NEXT Next
INVARIANT Inv
