\* options: -length 1
---- MODULE gen_test ----
\* Apalache's TestGen, a test isolated with Gen: Prepare, Test, Assertion.
EXTENDS Integers, Apalache
VARIABLES
    (*
     @typeAlias: MSG = { seqno: Int, ballot: Int };
     @type: Set(MSG);
    *)
    msgs,
    \* @type: Int;
    seqno
Prepare == seqno = Gen(1) /\ msgs = Gen(5) /\ \A m \in msgs: m.seqno < seqno
Test ==
    /\ \E b \in Int: msgs' = { [seqno |-> seqno, ballot |-> b] } \union msgs
    /\ seqno' = seqno + 1
Assertion == \A m1, m2 \in msgs: m2.seqno > m1.seqno => m2.ballot >= m1.ballot
====
INIT Prepare
NEXT Test
INVARIANT Assertion
