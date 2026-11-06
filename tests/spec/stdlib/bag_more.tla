---- MODULE bag_more ----
\* The rest of Bags.
EXTENDS Naturals, Bags, TLC
B == ("a" :> 2) @@ ("b" :> 1)
C == ("a" :> 1)
ASSUME PrintT(<<B (-) C, C (-) B, B (+) C, C \sqsubseteq B, B \sqsubseteq C, BagCardinality(B), CopiesIn("a", B), CopiesIn("z", B)>>)
ASSUME PrintT(<<BagIn("b", B), BagIn("b", C), BagToSet(B), IsABag(B), IsABag(("a" :> 0)), EmptyBag, BagUnion({B, C})>>)
ASSUME PrintT(<<BagOfAll(LAMBDA x : x % 2, SetToBag({1, 2, 3})), SubBag(C)>>)
====
