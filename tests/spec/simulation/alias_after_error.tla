---- MODULE alias_after_error ----
\* After an error, TLC evaluates the ALIAS on each state of the behavior
\* again, to make the CounterExample of its postconditions: with none, too.
\* options: -simulate num=1 -seed 1
EXTENDS Naturals, TLC
VARIABLE x
Init == x = 0
Next == x < 4 /\ x' = x + 1
Inv == x # 2
Alias == [y |-> x, p |-> PrintT(<<"alias", x>>)]
====
INIT Init
NEXT Next
INVARIANT Inv
ALIAS Alias
