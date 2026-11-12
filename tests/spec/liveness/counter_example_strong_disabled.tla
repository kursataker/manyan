---- MODULE counter_example_strong_disabled ----
\* Every behavior violates P. TLC checks SF_s(A) first as A eventually
\* never enabled: its counterexample stutters where A and B are disabled,
\* not the cycle through 1, 2, 3 and 5 where they are taken.
VARIABLE s
Succ == <<{3}, {2, 3, 5}, {2, 3}, {2}, {1, 3, 5}>>
Init == s \in {1, 3, 4, 5}
Next == s' \in Succ[s]
A == s' \in <<{}, {}, {3}, {2}, {3}>>[s]
B == s' \in <<{3}, {2, 3}, {3}, {}, {3, 5}>>[s]
Spec == Init /\ [][Next]_s /\ SF_s(A) /\ SF_s(B)
P == []<>(s \in {})
====
SPECIFICATION Spec
PROPERTY P
