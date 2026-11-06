---- MODULE string_sequences ----
\* The operators of Sequences read a string as its characters.
EXTENDS Naturals, Sequences, TLC
ASSUME PrintT(<<Len("abc"), Head("ab"), Tail("abc"), Append("ab", "c"), "ab" \o "cd", SubSeq("abcde", 2, 4)>>)
ASSUME PrintT(<<SelectSeq("abca", LAMBDA c : c = "a"), <<1>> \o "ab", "ab" \in Seq({"a", "b"}), "ac" \in Seq({"a", "b"})>>)
ASSUME PrintT(<<Len(""), Tail("a"), SubSeq("abc", 3, 2)>>)
====
