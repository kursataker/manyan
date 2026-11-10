---- MODULE csv_ops ----
\* CommunityModules' CSV, which TLC implements in Java: CSVWrite puts the
\* values, as TLC prints them, in a Java format and appends the line;
\* CSVWriteRecord the record's fields, in its order; CSVRead splits each
\* line by a regular expression; CSVRecords counts the lines, 0 for no file.
\* Found by tlaplus/Examples' SimKnuthYao and SimTokenRing.
EXTENDS CSV, TLC, Naturals
F == "out.txt"
ASSUME PrintT(CSVRecords(F))
ASSUME CSVWrite("%1$s#%2$s#%3$s", <<"abc", 42, {"x", "y"}>>, F)
ASSUME CSVWrite("%s|%S|%3$5s|%-4s.", <<"a", [p |-> 1], 7, <<>>>>, F)
ASSUME CSVWrite("%s", <<[i \in 1..30 |-> i * 1000]>>, F)
ASSUME CSVWrite("plain", <<>>, F)
ASSUME CSVWrite("%s;%s", [i \in 1..2 |-> i], F)
ASSUME PrintT(CSVRecords(F))
ASSUME CSVWriteRecord([b |-> "x", a |-> 1], ";", TRUE, F)
ASSUME CSVWriteRecord([b |-> "y", a |-> <<2>>], ";", FALSE, F)
ASSUME PrintT(CSVRecords(F))
ASSUME PrintT(CSVRead(<<"zz", "c1">>, "#", "in.csv"))
ASSUME PrintT(CSVRead(<<"one">>, "#", "in.csv"))
ASSUME PrintT(CSVRead([i \in 1..2 |-> IF i = 1 THEN "x" ELSE "y"], "[0-9]", "in.csv"))
ASSUME PrintT(CSVRead(<<"c1">>, ",", "none.csv"))
ASSUME PrintT(CSVRecords("none.csv"))
ASSUME PrintT(CSVRecords("in.csv"))
VARIABLE x
Init == x = 0
Next == UNCHANGED x
====
INIT Init
NEXT Next
