---- MODULE let ----
EXTENDS Naturals, TLC
F(n) == LET sq == n * n
            add(a, b) == a + b
            g[i \in 0..n] == IF i = 0 THEN 0 ELSE g[i - 1] + i
        IN add(sq, g[n])
ASSUME PrintT(<<F(3), LET x == 1 y == x + 1 IN <<x, y>> >>)
====
