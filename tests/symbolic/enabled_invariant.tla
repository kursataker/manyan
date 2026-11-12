---- MODULE enabled_invariant ----
\* ENABLED in an invariant, as TLC checks it: Inc is disabled once x is 3.
EXTENDS Naturals
VARIABLES
    \* @type: Int;
    x,
    \* @type: Bool;
    b
\* @type: <<Int, Bool>>;
vars == <<x, b>>
Init == x = 0 /\ b = FALSE
Inc == x < 3 /\ x' = x + 1 /\ UNCHANGED b
Flip == x = 3 /\ b' = ~b /\ UNCHANGED x
Next == Inc \/ Flip
IncOn == ENABLED Inc
====
INIT Init
NEXT Next
INVARIANT IncOn
