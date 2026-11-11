\* options: -length 10
\* apalache-mc check --length=10 --init=Init --next=Next --inv=Inv Bug20201118.tla: OK
------------ MODULE Bug20201118 ---------------------------
EXTENDS Integers

(*
 * @typeAlias: P = { sender: Str, data: { amount: Int } };
 *)
Bug20201118_typedefs == TRUE

VARIABLES
    \* @type: P;
    p,
    \* @type: Bool;
    error

AccountIds == { "", "A", "B" }

Data == [
  amount: 0..3
]

Packets == [
  sender: AccountIds,
  data: Data
]

\* @type: P => Bool;
WellFormed(packet) ==
  /\ packet.sender /= ""
  /\ packet.data.amount > 0

\* @type: P => Bool;
Pre(packet) ==
  LET data == packet.data IN
  WellFormed(packet)

Init ==
  /\ p \in Packets
  /\ error = FALSE

Action(packet) ==
  \/ /\ error' = FALSE
     /\ Pre(packet)
  \/ /\ error' = TRUE
     /\ ~Pre(packet)

Next ==
  /\ p' \in Packets
  /\ Action(p')

Inv == (error = TRUE) =>
        (p.data.amount <= 0 \/ p.sender = "")
===========================================================
INIT Init
NEXT Next
INVARIANT Inv
