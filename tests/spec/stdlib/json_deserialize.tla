---- MODULE json_deserialize ----
\* CommunityModules' ndJsonDeserialize and JsonDeserialize, which TLC
\* implements in Java: arrays are tuples, objects records, numbers Gson's
\* getAsInt, and a key's last value is kept. Found by tlaplus/Examples'
\* EWD998ChanTrace.
EXTENDS Json, TLC
ASSUME PrintT(ndJsonDeserialize("log.ndjson"))
ASSUME PrintT(JsonDeserialize("doc.json"))
VARIABLE x
Init == x = 0
Next == UNCHANGED x
====
INIT Init
NEXT Next
