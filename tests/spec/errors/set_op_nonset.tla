---- MODULE set_op_nonset ----
\* S \cup T, S \cap T and S \ T of a value that is not a set: TLC fails
\* only where it enumerates the result or looks in it. 2 is found in {2}
\* before 3 is looked in.
EXTENDS Naturals, TLC
ASSUME 2 \in {1} \cup 3 \cup {2}
ASSUME PrintT(1 \cup {1})
====
