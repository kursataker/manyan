---- MODULE tlc_ext_eval_definition_string ----
\* TLCEvalDefinition takes a string.
EXTENDS TLC, TLCExt
ASSUME PrintT(TLCEvalDefinition(42))
====
