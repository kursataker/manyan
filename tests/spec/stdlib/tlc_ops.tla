---- MODULE tlc_ops ----
EXTENDS Naturals, Sequences, TLC
ASSUME PrintT(<<1 :> "a", (1 :> "a") @@ (2 :> "b"), (1 :> "a") @@ (1 :> "z"), Permutations({1, 2, 3})>>)
ASSUME PrintT(<<SortSeq(<<3, 1, 2>>, <), SortSeq(<<"b", "a">>, LAMBDA x, y : x # y /\ x = "a")>>)
ASSUME PrintT(<<ToString(1), ToString({1, 2}), ToString(<<"a">>), TLCEval(1 + 1)>>)
ASSUME PrintT(Assert(TRUE, "unused"))
====
