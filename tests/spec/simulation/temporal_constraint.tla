---- MODULE temporal_constraint ----
\* A temporal formula evaluated as a predicate is TLC's TLCRuntimeException,
\* which a simulation reports plainly, with the behavior so far.
\* options: -simulate num=2 -depth 5 -seed 3
EXTENDS Naturals
VARIABLE x
Init == x = 1
Next == x' = x + 1 /\ x < 3
Bounded == IF x = 2 THEN (x > 0) ~> (x > 1) ELSE TRUE
====
INIT Init
NEXT Next
CONSTRAINT Bounded
