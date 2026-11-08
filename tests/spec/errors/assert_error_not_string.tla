---- MODULE assert_error_not_string ----
\* AssertError's message is a string literal: any other expression is
\* TLC's error, naming it, inside AssertError's preamble.
EXTENDS Naturals, TLCExt
E == "x"
ASSUME AssertError(E, 1 \div 0)
====
