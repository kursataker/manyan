---- MODULE sequence_sets ----
\* Seq(S) and SelectSeq.
EXTENDS Naturals, Sequences, TLC
ASSUME PrintT(<<<<1, 2>> \in Seq({1, 2}), <<3>> \in Seq({1, 2}), <<>> \in Seq({}), [i \in 1..2 |-> 1] \in Seq(Nat)>>)
ASSUME PrintT(<<SelectSeq(<<1, 2, 3, 4>>, LAMBDA x : x % 2 = 0), SelectSeq(<<>>, LAMBDA x : TRUE), Append(<<>>, <<>>)>>)
ASSUME PrintT({s \in Seq({1}) : Len(s) <= 2} = {<<>>, <<1>>, <<1, 1>>})
====
