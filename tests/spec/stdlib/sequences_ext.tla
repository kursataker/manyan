---- MODULE sequences_ext ----
\* CommunityModules' SetToSeq, which TLC implements in Java: the set's
\* elements in order, however many. Found by tlaplus/Examples'
\* SlidingPuzzles_anim.
EXTENDS SequencesExt, TLC, Naturals
ASSUME PrintT(SetToSeq({"b", "a", "c"}))
ASSUME PrintT(SetToSeq({{2}, {1, 3}, {}}))
ASSUME PrintT(SetToSeq(1..20))
ASSUME PrintT(SetToSeq({}))
VARIABLE x
Init == x = 0
Next == UNCHANGED x
====
INIT Init
NEXT Next
