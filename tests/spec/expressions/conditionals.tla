---- MODULE conditionals ----
EXTENDS Integers, TLC
Sign(n) == CASE n > 0 -> 1 [] n = 0 -> 0 [] OTHER -> -1
Pick(n) == CASE n = 1 -> "one"
             [] n = 2 -> "two"
             [] n = 1 -> "never"
ASSUME PrintT(<<IF 1 < 2 THEN "yes" ELSE "no", IF FALSE THEN 1 ELSE IF TRUE THEN 2 ELSE 3>>)
ASSUME PrintT(<<Sign(5), Sign(0), Pick(1), Pick(2)>>)

====
