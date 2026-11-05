---- MODULE let_scope ----
\* A LET definition is in scope after itself, so its body may bind its
\* own name; a RECURSIVE one is in scope in its own body.
EXTENDS Naturals
VARIABLE x
RECURSIVE fact
fact[k \in 0..5] == IF k = 0 THEN 1 ELSE k * fact[k - 1]
Pick(S) == LET e == CHOOSE e \in S : TRUE IN e
Sum(n) == LET RECURSIVE s(_)
              s(k) == IF k = 0 THEN 0 ELSE k + s(k - 1)
          IN s(n)
Init == x = Pick({fact[3]}) + Sum(4)
Next == x' = x
====
INIT Init
NEXT Next
