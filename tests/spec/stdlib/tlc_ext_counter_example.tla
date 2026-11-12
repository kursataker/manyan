---- MODULE tlc_ext_counter_example ----
\* In a postcondition after an error, CounterExample is the behavior TLC
\* reports: its states numbered, each step with its action's record, the
\* parameters that label it too; ToTrace(CounterExample) its states.
EXTENDS Naturals, TLC, TLCExt
VARIABLE x, y
Init == x \in {0, 1} /\ y = "a"
A(i) == x' = x + i /\ UNCHANGED y
Flip == y' = "b" /\ UNCHANGED x
Next == (\E i \in {2} : A(i)) \/ Flip
Inv == x < 3
Post == PrintT(CounterExample) /\ PrintT(ToTrace(CounterExample))
====
INIT Init
NEXT Next
INVARIANT Inv
POSTCONDITION Post
