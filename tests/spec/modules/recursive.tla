---- MODULE recursive ----
EXTENDS Naturals, Sequences, TLC
RECURSIVE Fact(_), Sum(_)
Fact(n) == IF n = 0 THEN 1 ELSE n * Fact(n - 1)
Sum(s) == IF s = <<>> THEN 0 ELSE Head(s) + Sum(Tail(s))
fib[n \in 0..15] == IF n < 2 THEN n ELSE fib[n - 1] + fib[n - 2]
ASSUME PrintT(<<Fact(10), Sum(<<1, 2, 3, 4>>), fib[15]>>)
====
