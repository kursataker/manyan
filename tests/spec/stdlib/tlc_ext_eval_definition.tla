---- MODULE tlc_ext_eval_definition ----
\* TLCEvalDefinition(name) evaluates the body of the root module's
\* definition of that name, through instances, where it is called: not
\* what the configuration replaces it by, nor Nat's Java.
EXTENDS Naturals, TLC, TLCExt, Lib
I == INSTANCE Inner WITH K <- 3
Over == 1
LOCAL Local == 5
f[n \in 1..2] == n * 10
VARIABLE x
Twice == 2 * x
Init == x = 0
Next == x < 2 /\ x' = x + 1 /\ PrintT(TLCEvalDefinition("Twice"))
ASSUME PrintT(<<TLCEvalDefinition("Shown"), TLCEvalDefinition("I!InnerDef"), TLCEvalDefinition("Over"),
                TLCEvalDefinition("Local"), TLCEvalDefinition("Nat"), TLCEvalDefinition("f")>>)
====
CONSTANT Over <- Shown
INIT Init
NEXT Next
CHECK_DEADLOCK FALSE
