---- MODULE replace_spec_init ----
\* The configuration replaces a definition the specification is made of:
\* Spec's initial predicate is MCInit.
EXTENDS ReplBase
MCInit == x \in {0, 1}
====
CONSTANT Init <- MCInit
SPECIFICATION Spec
CHECK_DEADLOCK FALSE
