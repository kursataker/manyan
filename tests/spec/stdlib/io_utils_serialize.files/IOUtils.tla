---- MODULE IOUtils ----
\* CommunityModules' IOUtils' Serialize, with a definition of its own:
\* TLC replaces it by its Java for the format TXT, as Manyan by its own.
Serialize(value, dest, options) == [exitValue |-> 7, stdout |-> "definition", stderr |-> ""]
====
