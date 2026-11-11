\* options: -length 10
\* apalache-mc check --inv=DriftInv --next=NextSlow antipatterns/fold-except/MC_FoldExcept3.tla: OK
--------------------- MODULE MC_FoldExcept3 ---------------------------------

Proc == { "p1", "p2", "p3" }

VARIABLES
    \* Process clocks
    \*
    \* @type: Str -> Int;
    clocks,
    \* Drift between pairs of clocks
    \*
    \* @type: <<Str, Str>> -> Int;
    drift

INSTANCE FoldExcept    

==============================================================================
INIT Init
NEXT NextSlow
INVARIANT DriftInv
