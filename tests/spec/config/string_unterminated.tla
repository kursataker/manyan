---- MODULE string_unterminated ----
\* A string in the configuration that a line ends is JavaCC's lexical
\* error, its text escaped as Java does (TLC's Github1455).
VARIABLE x
Init == x = 0
Next == x' = x
====
INIT Init
NEXT Next
	CONSTANT a = "x\"y\\z
