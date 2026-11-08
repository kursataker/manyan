---- MODULE subseteq_shortcuts ----
\* As TLC, SUBSET A \subseteq SUBSET B is A \subseteq B, and an interval
\* is in an interval by its bounds: neither is enumerated.
EXTENDS Integers, TLC
ASSUME PrintT(<<SUBSET (1..40) \subseteq SUBSET (0..50), SUBSET (1..40) \subseteq SUBSET (2..50),
                3..4000000 \subseteq 1..2147483647, 3..2 \subseteq 5..6>>)
====
