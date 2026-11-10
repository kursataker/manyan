---- MODULE tlcget_revision ----
\* TLCGet("revision"): that of the TLC tpp follows. A -depth of -1 is no
\* bound, though the configuration says -1. Found by tlaplus/Examples'
\* SimKnuthYao and SimTokenRing, whose assumptions check both.
\* options: -generate num=2 -depth -1 -seed 1
EXTENDS Naturals, TLC
ASSUME PrintT(TLCGet("revision"))
ASSUME PrintT(TLCGet("config").depth)
VARIABLE x
Init == x = 0
Next == x < 30 /\ x' = x + 1
Done == x < 30 \/ PrintT(TLCGet("level"))
====
INIT Init
NEXT Next
CONSTRAINT Done
CHECK_DEADLOCK FALSE
