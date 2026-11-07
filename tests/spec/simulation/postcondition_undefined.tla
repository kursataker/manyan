---- MODULE postcondition_undefined ----
\* The simulator reports a postcondition it cannot resolve plainly, with
\* its status, even after an error.
\* options: -simulate num=2 -depth 5 -seed 2
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next == x < 3 /\ x' = x + 1
====
INIT Init
NEXT Next
POSTCONDITION Nope
