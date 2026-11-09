---- MODULE io_utils ----
\* CommunityModules' IOUtils, whose operators TLC implements in Java:
\* IOEnv is the environment, atoi reads a number, IOExec and IOEnvExec run
\* a command. Found by tlaplus/Examples' TestGraphs.
EXTENDS IOUtils, TLC
ASSUME PrintT("TPP_NO_SUCH_VARIABLE" \in DOMAIN IOEnv)
ASSUME PrintT(<<atoi("42"), atoi("-17"), atoi("+3")>>)
ASSUME PrintT(IOExec(<<"echo", "out">>))
ASSUME PrintT(IOExec(<<"sh", "-c", "echo err >&2; exit 3">>))
ASSUME PrintT(IOEnvExec([X |-> "hi", N |-> 7], <<"sh", "-c", "echo $X $N">>))
VARIABLE x
Init == x = 0
Next == UNCHANGED x
====
INIT Init
NEXT Next
