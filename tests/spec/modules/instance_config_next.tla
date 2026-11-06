---- MODULE instance_config_next ----
\* INIT and NEXT of an unnamed instance; a violation's trace names the
\* instance's actions.
EXTENDS Naturals
VARIABLE t
INSTANCE Clock WITH N <- 3
Small == t < 2
====
INIT Init
NEXT Next
INVARIANT Small
