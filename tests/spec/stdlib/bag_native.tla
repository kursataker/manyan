---- MODULE bag_native ----
\* TLC evaluates Bags in Java: IsABag is FALSE for any non-bag, CopiesIn
\* is the count however small, BagUnion of one value is that value.
EXTENDS Integers, Bags, TLC
ASSUME PrintT(<<IsABag(<<"a">>), IsABag(3), IsABag({}), IsABag(<<>>), IsABag([a |-> 1])>>)
ASSUME PrintT(<<CopiesIn(2, 2 :> -1), BagIn(2, 2 :> -1), BagToSet(<<2, 2>>), SetToBag({3, 1, 2})>>)
ASSUME PrintT(<<BagUnion({3}), BagUnion({<<1, 2>>, 2 :> 3}), (1 :> 2 @@ 2 :> 1) (-) (1 :> 1 @@ 3 :> 4)>>)
====
