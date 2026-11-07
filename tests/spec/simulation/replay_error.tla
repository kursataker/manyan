---- MODULE replay_error ----
\* A behavior is printed with the action of each step as taken, the steps
\* that repeat a state left out, without evaluating Next again.
\* options: -simulate num=4 -depth 10 -seed 124
EXTENDS Naturals
VARIABLE x
Init == x = <<1, 2>>
Shift(c) == c' = [c EXCEPT ![1] = (c[2])']
Next == \/ x[1] = 1 /\ Shift(x)
        \/ UNCHANGED x
====
INIT Init
NEXT Next
