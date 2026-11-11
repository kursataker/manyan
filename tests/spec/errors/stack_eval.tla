---- MODULE stack_eval ----
\* Calls deeper than -stack: TLC's StackOverflowError, as when its Java
\* stack overflows.
\* options: -stack 50
EXTENDS Naturals
VARIABLE x
RECURSIVE Sum(_)
Sum(n) == IF n = 0 THEN 0 ELSE n + Sum(n - 1)
Init == x = Sum(100)
Next == UNCHANGED x
====
INIT Init
NEXT Next
