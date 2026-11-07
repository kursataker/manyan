---- MODULE prefix_conflict ----
\* UNION binds at 8, as \ does, so they conflict.
EXTENDS Naturals
ASSUME UNION {{1}} \ {1} = {}
====
