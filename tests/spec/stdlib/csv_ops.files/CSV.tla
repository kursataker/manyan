---- MODULE CSV ----
\* CommunityModules' CSV, which TLC replaces by its Java, as Manyan by its own.
CSVWrite(template, val, file) == FALSE
CSVWriteRecord(record, delim, headers, file) == FALSE
CSVRead(columns, delimiter, file) == FALSE
CSVRecords(file) == "definition"
====
