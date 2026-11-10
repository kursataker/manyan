---- MODULE chi_square_alpha ----
\* ChiSquare's significance level is in (0, 0.5]; Commons Math's error
\* prints it as Java's MessageFormat does.
EXTENDS Statistics, TLC
ASSUME PrintT(ChiSquare(<<1, 2>>, <<1, 2>>, "0.50091"))
====
