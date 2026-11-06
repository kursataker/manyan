---- MODULE no_next ----
\* A simulation without a next-state action and with nothing to check:
\* each behavior deadlocks in its initial state, as TLC reports.
\* options: -simulate num=1 -depth 5 -seed 1
VARIABLE x
Init == x \in {1, 2}
Spec == Init
====
SPECIFICATION Spec
