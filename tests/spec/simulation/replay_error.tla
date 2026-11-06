---- MODULE replay_error ----
\* TLC finds the action of every step of the behavior it prints, a step
\* that repeats a state too; finding it fails here as the behavior did.
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
