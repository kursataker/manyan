---- MODULE string_sequences ----
\* Len, Tail, SubSeq and \o take a string as its characters; the other
\* operators of Sequences take no string.
EXTENDS Naturals, Sequences, TLC
ASSUME PrintT(<<Len("abc"), Tail("abc"), "ab" \o "cd", SubSeq("abcde", 2, 4)>>)
ASSUME PrintT(<<Len(""), Tail("a"), SubSeq("abc", 3, 2)>>)
ASSUME PrintT(Head("ab"))
====
