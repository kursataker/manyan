---- MODULE tlc_values ----
\* ToString, TLCEval, Any, and what Print and Assert yield.
EXTENDS Naturals, Sequences, TLC
ASSUME PrintT(<<ToString(<<1, "a", {TRUE}>>), ToString("q"), ToString(1 :> 2), TLCEval(1 + 2), 3 \in Any, "x" \in Any>>)
ASSUME PrintT(Print("out", 7) + 1)
ASSUME Assert(1 = 1, "fine")
ASSUME Assert(1 = 2, <<"bad", 2>>)
====
