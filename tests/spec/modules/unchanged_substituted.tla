---- MODULE unchanged_substituted ----
\* An instance's variable stands for a definition of the instantiating
\* module: UNCHANGED of it leaves the variables of the definition unchanged.
EXTENDS Naturals
VARIABLES x, y, s
vars == <<x, y>>
INSTANCE Stutter
Init == x = 0 /\ y = 0 /\ s = 0
Step == x' = (x + 1) % 3 /\ y' = x /\ UNCHANGED s
Next == Step \/ Pause
====
INIT Init
NEXT Next
