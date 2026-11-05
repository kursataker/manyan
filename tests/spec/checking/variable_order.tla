---- MODULE variable_order ----
\* TLC lists variables as its symbol table enumerates them, not as declared.
EXTENDS Naturals, TLC
CONSTANT lim
VARIABLES pc, alpha, s, queue
Free == 3
I == INSTANCE Naturals
THEOREM Sound == TRUE
VARIABLE done
Init == pc = "a" /\ alpha = 0 /\ s = {} /\ queue = <<>> /\ done = FALSE
Next == /\ alpha < lim
        /\ alpha' = alpha + 1
        /\ UNCHANGED <<pc, s, queue, done>>
====
INIT Init
NEXT Next
CONSTANT lim = 2
