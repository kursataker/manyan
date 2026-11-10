---- MODULE chi_square ----
\* CommunityModules' ChiSquare, which TLC implements in Java: Commons
\* Math's chi-square test of the actual counts against the expected, at
\* the significance level of the string alpha. Found by tlaplus/Examples'
\* SimKnuthYao.
EXTENDS Statistics, TLC, Naturals
ASSUME PrintT(ChiSquare([i \in 1..6 |-> 6], [i \in 1..6 |-> 30 + i], "0.2"))
ASSUME PrintT(ChiSquare([i \in 1..3 |-> 6], <<1, 10, 30>>, "0.05"))
ASSUME PrintT(ChiSquare(<<10, 20>>, <<11, 19>>, "0.5"))
ASSUME PrintT(ChiSquare(<<10, 20>>, <<1, 40>>, "0.01"))
ASSUME PrintT(ChiSquare([a |-> 1, b |-> 2], <<1, 2>>, " 1e-1 "))
ASSUME PrintT(ChiSquare(<<100, 100, 100, 100>>, <<90, 110, 95, 120>>, "0.1"))
ASSUME PrintT(ChiSquare(<<100, 100, 100, 100>>, <<90, 110, 95, 120>>, "0.5"))
ASSUME PrintT(ChiSquare([i \in 1..20 |-> 50], [i \in 1..20 |-> 40 + i], "0.05"))
ASSUME PrintT(ChiSquare(<<3, 0>>, <<3, 0>>, "0.2") = "never")
====
