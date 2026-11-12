---- MODULE tlc_ext_eval_definition_params ----
\* TLCEvalDefinition takes a definition without parameters.
EXTENDS TLC, TLCExt
Param(a, b) == a
ASSUME PrintT(TLCEvalDefinition("Param"))
====
