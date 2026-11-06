---- MODULE tlcget_level_constraint ----
\* A constraint on TLCGet("level") bounds the search (TLCGetLevel from
\* TLC's tests); it is no constant TLC evaluates as it starts.
EXTENDS Naturals, TLC
VARIABLE x
Init == x = 0
Next == x' = x + 1
C == TLCGet("level") < 3
====
INIT Init
NEXT Next
CONSTRAINT C
