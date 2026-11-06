---- MODULE error_initial_module ----
\* A standard module's operator fails on the initial state: TLC prints
\* its message but not the state.
EXTENDS Naturals, TLC
VARIABLE x
Init == x = 0
Next == x < 3 /\ x' = x + 1
Spec == Init /\ [][Next]_x /\ WF_x(Next)
Div == <>(10 \div x = 7)
Asserted == <>Assert(x # 0, "zero")
====
SPECIFICATION Spec
PROPERTY Div Asserted
