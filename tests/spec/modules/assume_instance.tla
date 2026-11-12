---- MODULE assume_instance ----
\* An assumption named through instances, even two deep, is of the level
\* of the substitutions it uses: a variable substituted, unused, leaves it
\* constant.
EXTENDS Naturals
VARIABLE x
N == {1, 2}
Leader(r) == 2
INSTANCE AssumeOuter
ASSUME NonEmpty
ASSUME LeaderIn
Init == x = 0
Next == x < 2 /\ x' = x + 1
====
INIT Init
NEXT Next
CHECK_DEADLOCK FALSE
