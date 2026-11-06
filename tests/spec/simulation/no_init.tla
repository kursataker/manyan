---- MODULE no_init ----
\* A specification whose initial predicate is TRUE, so dropped: model
\* checking finds no states and no error; a simulation has no state to
\* start from.
\* options: -simulate num=2 -depth 5 -seed 1
Init == TRUE
Next == TRUE
Spec == Init /\ [][Next]_<<>>
====
SPECIFICATION Spec
