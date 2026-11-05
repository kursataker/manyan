---- MODULE if_condition ----
\* The condition of IF must be a Boolean.
EXTENDS Integers, Sequences, FiniteSets, TLC
ASSUME PrintT(IF [a |-> 1] THEN 1 ELSE 2)
====
