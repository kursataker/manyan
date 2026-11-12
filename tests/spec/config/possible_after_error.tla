---- MODULE possible_after_error ----
\* The _POSSIBLE predicates are checked after an invariant's violation
\* too, with what was witnessed until then; the exit status is the
\* violation's.
EXTENDS Naturals
VARIABLE x
Init == x \in {0, 1}
Next == x < 3 /\ x' = x + 1
Two == x = 2
Big == x = 7
Small == x < 3
====
INIT Init
NEXT Next
_POSSIBLE Two Big
INVARIANT Small
CHECK_DEADLOCK FALSE
