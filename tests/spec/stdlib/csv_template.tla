---- MODULE csv_template ----
\* CSVWrite's Java format fails on a conversion that is not of a string,
\* each value being one; TLC reports Java's exception with the method.
EXTENDS CSV, TLC
ASSUME CSVWrite("%d", <<1>>, "out.csv")
====
