---- MODULE logic ----
EXTENDS TLC
ASSUME PrintT(<<TRUE /\ FALSE, TRUE \/ FALSE, ~TRUE, \lnot FALSE, \neg TRUE>>)
ASSUME PrintT(<<FALSE => TRUE, TRUE => FALSE, TRUE <=> TRUE, FALSE \equiv TRUE>>)
ASSUME PrintT(<<FALSE /\ (1 = "a"), TRUE \/ (1 = "a"), FALSE => (1 = "a")>>)
ASSUME PrintT(BOOLEAN)
====
