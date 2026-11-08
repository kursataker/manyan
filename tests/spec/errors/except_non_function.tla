---- MODULE except_non_function ----
\* EXCEPT on a value that is no function, at the top or down its path,
\* fails with TLC's message, which names no location, a model value as
\* any value. DOMAIN of a non-function and SortSeq of a non-sequence
\* are TLC's messages too.
EXTENDS Integers, Sequences, TLC, TLCExt
CONSTANT m
ASSUME AssertError("Attempted to apply EXCEPT construct to the value {1}.", [{1} EXCEPT ![1] = 0])
ASSUME AssertError("Attempted to apply EXCEPT construct to the value 1.", [<<1>> EXCEPT ![1][2] = 0])
ASSUME AssertError("Attempted to apply EXCEPT construct to the value \"ab\".", ["ab" EXCEPT ![1] = 0])
ASSUME AssertError("Attempted to apply EXCEPT construct to the value m.", [m EXCEPT ![1] = 0])
ASSUME AssertError("The first argument of SortSeq should be a natural number, but instead it is:\n{3}", SortSeq({3}, <))
VARIABLE x
Init == x = <<1>>
Next == x' = [x EXCEPT ![1][1] = 2]
====
CONSTANT m = m
INIT Init
NEXT Next
