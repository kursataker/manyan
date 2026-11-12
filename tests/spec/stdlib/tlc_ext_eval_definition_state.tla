---- MODULE tlc_ext_eval_definition_state ----
\* A definition of a state, evaluated where there is none: TLC's error, in
\* the Java method of TLCEvalDefinition.
EXTENDS Naturals, TLC, TLCExt
VARIABLE x
Inv == x = 0
ASSUME PrintT(TLCEvalDefinition("Inv"))
Init == x = 0
Next == UNCHANGED x
====
INIT Init
NEXT Next
