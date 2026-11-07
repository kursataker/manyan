---- MODULE string_sequences ----
\* \o joins two strings, but not a sequence and a string.
EXTENDS Integers, Sequences, TLC
ASSUME PrintT("ab" \o "c")
ASSUME PrintT(<<1>> \o "ab")
====
