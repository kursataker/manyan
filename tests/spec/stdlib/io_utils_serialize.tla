---- MODULE io_utils_serialize ----
\* CommunityModules' Serialize, which TLC implements in Java: the format
\* TXT writes the string to the file, with Java's open options; another
\* format is left to the definition. Found by tlaplus/Examples'
\* SlidingPuzzles_anim.
EXTENDS IOUtils, TLC
Txt(options) == [format |-> "TXT", charset |-> "UTF-8", openOptions |-> options]
ASSUME PrintT(Serialize("one\n", "out.txt", Txt(<<"WRITE", "CREATE", "TRUNCATE_EXISTING">>)))
ASSUME PrintT(Serialize("two\n", "out.txt", Txt(<<"APPEND">>)))
ASSUME PrintT(Serialize("x", "out.txt", Txt(<<"CREATE_NEW">>)).exitValue)
ASSUME PrintT(Serialize("x", "out.txt", [format |-> "JSON"]))
VARIABLE x
Init == x = 0
Next == UNCHANGED x
====
INIT Init
NEXT Next
