---- MODULE subexpressions ----
EXTENDS Naturals, TLC
Op(a) == \A x \in {1, 2} : lab(x) :: LET d(y) == a + x + y IN d(1) > 0 /\ in :: a
Sum(q) == {x + q : x \in {1, 2}}
Pick(p) == CHOOSE x \in {7} : eq(x) :: x = p
Apply2(F(_, _), a, b) == F(a, b)
Apply3(F(_, _, _), a, b, c) == F(a, b, c)
Fact(c) == CASE c > 0 -> c [] OTHER -> 0
THEOREM Thm == lab :: 1 + 1 = 2
ASSUME Named == <<3, 4>> \in {<<3, 4>>}
ASSUME Op(10)!lab(2)!:!d(3) = 15
ASSUME Op(10)!lab(2)!in = 10
ASSUME Op(10)!(2)!1!<<!<< = 13
ASSUME Op(10)!1 = {1, 2}
ASSUME Apply2(Sum!@, 5, 2) = 7
ASSUME Apply3(Op!lab!:!d, 1, 2, 3) = 6
ASSUME Pick(3)!1 = {7} /\ Pick(3)!eq(4)!<< = 4 /\ ~Pick(3)!(4)
ASSUME Fact(5)!1!<< /\ Fact(5)!1!>> = 5 /\ Fact(5)!2!>> = 0
ASSUME Thm /\ Thm!lab!1 = 2 /\ Thm!: /\ Named!1!>> = 4
Sq == [i \in 1..3 |-> i * i][2]
Field == [a |-> 1, b |-> 2].b
ASSUME Sq!<<[3] = 9 /\ Sq!>> = 2 /\ Field!2 = "b"
====
