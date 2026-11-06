---- MODULE extends_conflict ----
\* Two extended modules define R: SANY warns, and the first definition is
\* the one used. An instance's R gives way to it too, with another warning.
EXTENDS ConflictB, ConflictA, TLC
LOCAL INSTANCE ConflictA
ASSUME PrintT(R)
====
