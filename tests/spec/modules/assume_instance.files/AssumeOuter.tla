---- MODULE AssumeOuter ----
CONSTANTS N, Leader(_)
VARIABLE x
ASSUME NonEmpty == N # {}
INSTANCE AssumeInner
ASSUME LeaderIn
====
