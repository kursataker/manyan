---- MODULE substitute_tlcget ----
\* TLCGet is no constant to TLC: a substitution of it that fails as TLC
\* starts is an error, "level" is 0 then, and the counters are undefined.
EXTENDS TLC
CONSTANT L, D
level == TLCGet("level")
distinct == TLCGet("distinct")
ASSUME L = 0
====
CONSTANTS
L <- level
D <- distinct
