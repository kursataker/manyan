---- MODULE trace_file ----
\* -simulate file=FILE writes each behavior that ends without an error to
\* FILE_0_N, a module defining its states, as TLC does.
\* options: -simulate num=3,file=walk -depth 4 -seed 7
EXTENDS Integers
VARIABLES x, y
Init == x \in {0, 10} /\ y = "a"
Next == x' \in {x - 1, x + 1} /\ UNCHANGED y
====
INIT Init
NEXT Next
