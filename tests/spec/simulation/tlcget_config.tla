---- MODULE tlcget_config ----
\* TLCGet("config") describes the simulation, in an assumption too.
\* options: -simulate num=4 -depth 7 -seed 3 -deadlock
EXTENDS Naturals, TLC
VARIABLE n
C == TLCGet("config")
ASSUME PrintT(<<C.mode, C.deadlock, C.seed, C.worker, C.traces, C.depth, C.aril, C.sched>>)
Init == n = 0
Next == n < 2 /\ n' = n + 1
====
INIT Init
NEXT Next
