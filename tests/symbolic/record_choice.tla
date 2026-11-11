---- MODULE record_choice ----
\* A step chooses any record of a set of records.
EXTENDS Integers
VARIABLES
    \* @type: { a: Int, b: Bool };
    r
Init == r = [a |-> 0, b |-> FALSE]
Next == r' \in [a : 0..3, b : BOOLEAN]
Inv == r.a < 3 \/ ~r.b
====
INIT Init
NEXT Next
INVARIANT Inv
