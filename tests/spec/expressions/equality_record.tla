---- MODULE equality_record ----
\* A record equals only what TLC makes a record of.
EXTENDS TLC
ASSUME PrintT([a |-> 1] = <<1>>)
====
