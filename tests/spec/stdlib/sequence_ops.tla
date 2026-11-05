---- MODULE sequence_ops ----
EXTENDS Naturals, Sequences, TLC
s == <<3, 1, 4, 1, 5>>
ASSUME PrintT(<<Len(s), Head(s), Tail(s), Append(s, 9), s \o <<2, 6>>, SubSeq(s, 2, 4), SubSeq(s, 3, 2)>>)
ASSUME PrintT(<<SelectSeq(s, LAMBDA x : x > 2), Len(<<>>), <<>> \o <<>>, Seq({1}) = Seq({1})>>)
ASSUME PrintT({t \in Seq({"a"}) : Len(t) < 3} = {})
====
