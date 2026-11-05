---- MODULE leads_to ----
EXTENDS Naturals
VARIABLES req, ack
vars == <<req, ack>>
Init == req = FALSE /\ ack = FALSE
Request == ~req /\ req' = TRUE /\ ack' = FALSE
Ack == req /\ ~ack /\ ack' = TRUE /\ UNCHANGED req
Reset == ack /\ req' = FALSE /\ ack' = FALSE
Next == Request \/ Ack \/ Reset
Spec == Init /\ [][Next]_vars /\ WF_vars(Ack) /\ WF_vars(Reset)
Responds == req ~> ack
AlwaysAck == [](req => ack)
====
SPECIFICATION Spec
PROPERTIES Responds
