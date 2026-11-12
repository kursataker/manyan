---- MODULE error_fairness_order ----
\* As TLC, weak fairness's ENABLED is evaluated on each step, before strong
\* fairness's on the state: the error is the WF's.
VARIABLE s
Init == s = 0
Next == s' \in {0, 1}
Spec == Init /\ [][Next]_s
          /\ SF_s(<<s, s'>> \in {<<0, 1>>})
          /\ WF_s(<<s, s'>> \in {<<1, 0>>})
P == <>(s = 1)
====
SPECIFICATION Spec
PROPERTY P
