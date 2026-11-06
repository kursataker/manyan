---- MODULE Counter ----
EXTENDS Naturals
CONSTANT Max
VARIABLE c
LOCAL Step(n) == IF n < Max THEN n + 1 ELSE 0
RECURSIVE Sum(_)
Sum(n) == IF n = 0 THEN 0 ELSE n + Sum(n - 1)
Init == c = 0
Next == c' = Step(c)
Total == Sum(c)
====
