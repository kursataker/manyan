---- MODULE params_order_message ----
\* An error message prints a function of several bounds in the order TLC
\* computed it, the first bound varying fastest.
EXTENDS Integers, TLC
ASSUME PrintT([x \in {3, 1, 2} |-> <<[y \in {"b", "a"}, z \in 1..2 |-> 0]>>][9])
====
