---- MODULE tlc_ext_eval_definition_local ----
\* A LOCAL definition of a module the root extends is no name of the root's.
EXTENDS TLC, TLCExt, Lib
ASSUME PrintT(TLCEvalDefinition("Hidden"))
====
