---- MODULE record_names ----
\* TLC prints a function of strings as a record only if each is a name:
\* letters, digits and _, with a letter, and not WF_ or SF_ and more (FZ3
\* from tlaplus-x/tlx).
EXTENDS TLC
ASSUME PrintT(<<[s \in {"", "c"} |-> 1], "a b" :> 1, "_" :> 1, "1a" :> 1, "WF_x" :> 1, "WF_" :> 1, [a |-> 1, b2 |-> 2]>>)
====
