---- MODULE numbers ----
EXTENDS Integers, TLC
ASSUME PrintT(<<0, 007, 42, \b1010, \B11, \o17, \O7, \hff, \HFF, \h0>>)
ASSUME PrintT(<<-1, - 2, 2147483647, -2147483647>>)
====
