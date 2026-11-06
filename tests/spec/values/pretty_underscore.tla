---- MODULE pretty_underscore ----
\* TLC's pretty printer cannot parse a field or model value whose name has
\* other than letters and digits, and prints such a value on one line
\* (Percolator from pingcap/tla-plus); a string is parsed.
EXTENDS TLC
CONSTANTS c1, c2, gen_block
VARIABLES ts, blocks, names
Init == /\ ts = [c \in {c1, c2} |-> [start_ts |-> 0, commit_ts |-> 0]]
        /\ blocks = <<gen_block, "2222222222222222222222222", "3333333333333333333333333333333333">>
        /\ names = <<"gen_block", "2222222222222222222222222", "3333333333333333333333333333333333">>
Next == UNCHANGED <<ts, blocks, names>>
Inv == names = <<>>
====
CONSTANTS c1 = c1 c2 = c2 gen_block = gen_block
INIT Init
NEXT Next
INVARIANT Inv
