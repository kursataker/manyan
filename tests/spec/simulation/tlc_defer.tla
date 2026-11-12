---- MODULE tlc_defer ----
\* TLCDefer(e) is TRUE; once the simulation picks a successor, it
\* evaluates the last TLCDefer met since the successor before was made:
\* the action's, unless a check of the successor met one after it.
\* options: -simulate num=3 -depth 5 -seed 2
EXTENDS Naturals, TLC, TLCExt
VARIABLE x, y
Init == x = 0 /\ y = 0 /\ TLCDefer(PrintT(<<"init", x>>))
Next == x < 8 /\ TLCDefer(PrintT(<<"pre", x, x'>>)) /\
        \E i \in {1, 2, 3} :
           /\ x' = x + i /\ y' = i
           /\ IF i = 3 THEN TLCDefer(PrintT(<<"three", x, x'>>)) /\ TLCDefer(PrintT(<<"last", i>>)) ELSE TRUE
Inv == x # 4 \/ TLCDefer(PrintT(<<"four", x>>))
AC == y' # 2 \/ TLCDefer(PrintT(<<"two", x, x'>>))
====
INIT Init
NEXT Next
INVARIANT Inv
ACTION_CONSTRAINT AC
CHECK_DEADLOCK FALSE
