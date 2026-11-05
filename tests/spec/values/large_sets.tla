---- MODULE large_sets ----
\* Sets too large to enumerate print as they are written.
EXTENDS Naturals, TLC
Big == 1..2000
ASSUME PrintT(<<Big \X Big \X Big, [a : Big, b : Big, c : Big], [1..3 -> Big], SUBSET (1..40)>>)
ASSUME PrintT(<<SUBSET (1..2), {1} \X {1}, [a : {1}]>>)
====
