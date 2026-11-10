---- MODULE SequencesExt ----
\* CommunityModules' SetToSeq, with its definition: TLC replaces it by its
\* Java, as Manyan by its own.
LOCAL INSTANCE FiniteSets
LOCAL INSTANCE Naturals
SetToSeq(S) == CHOOSE f \in [1..Cardinality(S) -> S] : \A i, j \in DOMAIN f : f[i] = f[j] => i = j
====
