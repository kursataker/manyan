---- MODULE seq_ops ----
\* Concatenation and subsequences of symbolic sequences.
EXTENDS Integers, Sequences
VARIABLES
    \* @type: Seq(Int);
    s
Init == s = <<>>
Next == \E v \in 1..2 : Len(s) < 4 /\ s' = Append(s, v)
Laws == /\ Len(s \o s) = 2 * Len(s)
        /\ SubSeq(s, 1, Len(s)) = s
        /\ Len(s) > 0 => s = <<Head(s)>> \o Tail(s)
NoPair == s /= <<2, 1, 2>>
====
INIT Init
NEXT Next
INVARIANT Laws NoPair
