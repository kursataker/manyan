---- MODULE not_non_boolean ----
\* ~ of a non-boolean: TLC names its kind, not its value.
EXTENDS Integers, TLC
ASSUME PrintT(~ {1})
====
