---- MODULE tlcget_stats ----
\* TLCGet("stats") of a simulation: the actions into the state evaluated,
\* the successor's too in a constraint, the behavior's number, the states
\* generated, and the mean and variance of the behaviors' lengths, as TLC
\* keeps them in a long. The duration is left out, as it varies.
\* options: -simulate num=15 -depth 12 -seed 3
EXTENDS Naturals, TLC
VARIABLE x
Init == x \in {0, 1, 2}
A == x < 9 /\ x' = x + 1
B == x < 7 /\ x' = x + 3
Next == A \/ B
Stats == [k \in DOMAIN TLCGet("stats") \ {"duration"} |-> TLCGet("stats")[k]]
ASSUME PrintT(Stats)
Constraint == x # 9 \/ TLCGet("stats").traces % 4 # 0 \/ PrintT(Stats)
====
INIT Init
NEXT Next
CONSTRAINT Constraint
CHECK_DEADLOCK FALSE
