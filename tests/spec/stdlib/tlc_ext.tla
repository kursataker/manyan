---- MODULE tlc_ext ----
\* TLCExt: AssertError compares the message of the error its expression
\* fails with, preambles included, and is FALSE when it does not fail;
\* AssertEq prints both sides when they differ; TLCModelValue makes a
\* model value; TLCGetOrDefault and TLCGetAndSet read a register.
EXTENDS Naturals, Bags, TLC, TLCExt
ASSUME AssertError("Attempted to compare boolean TRUE with non-boolean:\n1", [a : {1, TRUE}] # {})
ASSUME AssertError("Overflow when computing 2147483647+1", ([x \in {1} |-> 2147483647] (+) [x \in {1} |-> 1])[1] > 0)
ASSUME AssertError("The first argument of Assert evaluated to FALSE; the second argument was:\n\"no\"", Assert(FALSE, "no"))
ASSUME ~AssertError("x", 1 + 1 = 2)
ASSUME ~AssertError("x", 1 \div 0)
ASSUME ~AssertEq(<<1, 2>>, <<1, 3>>)
ASSUME AssertEq(TLCNoOp(3), 3)
ASSUME TLCModelValue("m") = TLCModelValue("m") /\ ToString(TLCModelValue("m")) = "m"
ASSUME PrintT(<<TLCModelValue("m"), {TLCModelValue("n"), TLCModelValue("m")}>>)
ASSUME AssertError("The argument of ModelValue should be a string, but instead it is:\n1", TLCModelValue(1))
ASSUME TLCGetOrDefault(3, "none") = "none"
ASSUME TLCSet(3, 5) /\ TLCGetOrDefault(3, "none") = 5
ASSUME TLCGetAndSet(3, +, 2, 0) = 5 /\ TLCGet(3) = 7
====
