\* options: -length 0
\* apalache-mc check --length=0 --init=Init --next=Next --inv=Inv Bug3491.tla: OK
---- MODULE Bug3491 ----
EXTENDS Integers

VARIABLE
  \* @type: Int;
  v

\* @type: <<Int, <<Int, Int>>>> -> Int;
F == [x \in {1, 2}, <<a, b>> \in {<<3, 4>>} |-> x + a + b]

Inv == /\ F[2, <<3, 4>>] = 9
       /\ DOMAIN F = {1, 2} \X {<<3, 4>>}

Init == v = 0
Next == UNCHANGED v
====
INIT Init
NEXT Next
INVARIANT Inv
