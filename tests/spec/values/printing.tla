---- MODULE printing ----
EXTENDS Integers, Sequences, TLC
ASSUME PrintT(<<TRUE, -3, "q\"\\", {}, <<>>, [i \in 1..2 |-> i], [i \in {0, 1} |-> i], [i \in {"f"} |-> 1]>>)
ASSUME PrintT(<<{<<1, 2>>, <<1>>, <<>>}, {{1, 2}, {3}, {}}, {FALSE, TRUE}, {-1, 5, 0}>>)
ASSUME PrintT(<<[x \in {1, 2} |-> {}], [x \in {<<1>>} |-> 0], [x \in {{}} |-> 0]>>)
ASSUME Print("label", 1) = 1
ASSUME Print(<<1>>, {2}) = {2}
====
