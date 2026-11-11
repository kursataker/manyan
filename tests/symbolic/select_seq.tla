---- MODULE select_seq ----
\* SelectSeq keeps the elements a test passes, in order, from a sequence
\* the behavior builds.
EXTENDS Integers, Sequences
VARIABLES
    \* @type: Seq(Int);
    s
Even(i) == i % 2 = 0
Init == s = <<>>
Next == Len(s) < 4 /\ \E i \in 1..3 : s' = Append(s, i)
Inv == SelectSeq(s, Even) /= <<2, 2>>
====
INIT Init
NEXT Next
INVARIANT Inv
