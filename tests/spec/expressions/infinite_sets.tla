---- MODULE infinite_sets ----
EXTENDS Integers, Sequences, TLC
ASSUME PrintT(<<5 \in Nat, -1 \in Nat, -1 \in Int, "a" \in STRING, <<1, 2>> \in Seq(Nat), <<-1>> \in Seq(Nat)>>)
ASSUME PrintT(<<{1, 2} \in SUBSET Nat, [a |-> 1] \in [a : Nat], <<1, 2>> \in [1..2 -> Nat]>>)
ASSUME PrintT(<<7 \in {x \in Nat : x % 2 = 1}, {1, 2} \subseteq Nat>>)
====
