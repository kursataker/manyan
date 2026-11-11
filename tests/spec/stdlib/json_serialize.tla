---- MODULE json_serialize ----
\* JsonSerialize writes a value's JSON, ndJsonSerialize a sequence's, one
\* element a line; each makes the file's directories.
EXTENDS Json, TLC, IOUtils
ASSUME JsonSerialize("out.json", <<[a |-> 1], {1, 2}, "s">>)
ASSUME JsonSerialize("dir/sub/rec.json", [a |-> <<>>, b |-> {}])
ASSUME ndJsonSerialize("out.ndjson", <<[a |-> 1], <<2, 3>>, "s">>)
ASSUME ndJsonSerialize("empty.ndjson", <<>>)
ASSUME PrintT(IOExec(<<"cat", "out.json", "dir/sub/rec.json">>).stdout)
ASSUME PrintT(IOExec(<<"cat", "out.ndjson", "empty.ndjson">>).stdout)
ASSUME PrintT(JsonDeserialize("out.json"))
ASSUME PrintT(ndJsonDeserialize("out.ndjson"))
VARIABLE x
Init == x = 0
Next == UNCHANGED x
====
INIT Init
NEXT Next
