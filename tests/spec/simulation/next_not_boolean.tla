---- MODULE next_not_boolean ----
\* A conjunct of Next that is no boolean: TLC's Assert.fail raises a
\* TLCRuntimeException, which its simulator reports plainly.
\* options: -simulate num=1 -depth 5 -seed 1
EXTENDS Naturals
VARIABLE x
Init == x = 0
Next == x' = x + 1 /\ x
====
INIT Init
NEXT Next
