# Changelog

A release is tagged `vX.Y.Z`, the version `TPP_VERSION` in `tpp.h` names.
Pushing the tag builds, tests and publishes it, with this file's section
for it as its notes.

## 0.1.0

The first release: a model checker for TLA⁺ that checks what TLC checks
and prints what TLC 2.19 prints.

- The TLA⁺ language as TLC evaluates it, the standard modules, and TLC's
  model configuration.
- Checking of assumptions, invariants, action properties, deadlock and
  temporal properties under fairness, with TLC's traces and
  counterexamples; `-workers N` for a parallel search with the same output
  as one worker.
- Random simulation (`-simulate`), drawing TLC's behaviors for a seed.
- TLC's error messages and exit statuses.
- A C API (`tpp.h`).

Not yet supported: generation mode, writing simulated behaviors to files,
checkpoints, distributed checking.

Known differences from TLC, found by the fuzzers:

- A set of sets holding an infinite set, such as `{{-1}, Nat}`, is built
  at once, so `1 \in {} \ UNION {{-1}, Nat}` is an error where TLC, which
  never builds it, answers `FALSE`.
- A simulation of `tests/spec/liveness/sf_refine.tla` may report a
  temporal property violated that TLC finds holds.

Binaries: macOS (arm64, x86_64), Linux (x86_64, arm64; static), Windows
(x86_64, built with Cygwin, `cygwin1.dll` beside `tpp.exe`; it runs on
Windows on Arm too, emulated, as Cygwin has no arm64 port). In a Cygwin
shell, delete the bundled `cygwin1.dll`: two in one session conflict.
