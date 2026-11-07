---- MODULE prefix_precedence ----
\* Prefix operators against infix ones that do not overlap them: SUBSET
\* and UNION bind at 8, below .., and DOMAIN at 9, above \cup.
EXTENDS Integers, TLC
ASSUME PrintT(-2 ^ 2)
ASSUME PrintT(- 1 - 1)
ASSUME PrintT(-1 * 2 + 3)
ASSUME PrintT(DOMAIN <<1, 2>> \cup {3})
ASSUME PrintT(SUBSET {1} \subseteq SUBSET {1, 2})
ASSUME PrintT(SUBSET 1..2)
ASSUME PrintT(UNION {{1}, {2}} \subseteq 1..2)
ASSUME PrintT(~ 1 = 2)
====
