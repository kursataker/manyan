---- MODULE string_sequences ----
\* The operators of Sequences read a string as the sequence of its characters.
EXTENDS Integers, Sequences, FiniteSets, TLC
ASSUME PrintT(<<1>> \o "ab")
ASSUME PrintT("ab" \o "c")
ASSUME PrintT(Head("ab"))
ASSUME PrintT(Tail("ab"))
ASSUME PrintT(Append("ab", "c"))
ASSUME PrintT(SubSeq("abcd", 2, 3))
ASSUME PrintT(SubSeq("abc", 2, 1))
ASSUME PrintT(SelectSeq("abc", LAMBDA ch : ch = "a"))
ASSUME PrintT("ab" \in Seq({"a", "b"}))
ASSUME PrintT(Len(Tail("abc")))
====
