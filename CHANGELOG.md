2026-11-11 0.9.0:

- simulation: an alias whose _format Java cannot apply prints the behavior
  once (#1); a conjunct of Next that is no boolean is reported plainly, as
  TLC's TLCRuntimeException; TLCCache keeps a state's value in the state,
  an initial state's for every behavior from it
- no memory is lost once manyan_check returns: make sanitize checks it on
  Linux, and make sanitize-linux runs that in Docker; manyan_alloc gives a
  native memory that lives as values do

2026-11-10 0.8.0:

- renamed to Manyan (github.com/soasme/manyan): the command is manyan, the
  sources manyan.c, manyan.h, manyan-std.c and manyan-std.h, and the C API
  manyan_*, Manyan* and MANYAN_* in place of tpp_*, TPP* and TPP_*

2026-11-10 0.7.0:

- make full-bench checks tlaplus/Examples against TLC (docs/fullbench.md)
- tpp-std.c: the operators TLC computes in Java, on a public native API in
  tpp.h (tpp_define); CommunityModules' IOUtils, Json deserializers,
  Graphs, UndirectedGraphs, SequencesExt's SetToSeq and SVG (docs/std.md)
- action composition (\cdot, with -Dtlc2.tool.impl.Tool.cdot=true) and
  -Dk=v; fairness of the form <>[][A]_v and P => WF_v(A) with P constant
- under a VIEW, a property's actions are evaluated on the real steps; a
  property that is an instance's operator is named by the call
- the search drops what each quantifier's body and filter's test made
  (MCInnerSerial over 4 GB -> 0.47 GB), at 2-4% of the closure bench

2026-11-08 0.6.1:

- edge cases from tlaplus/Examples and TLC's own test models: instances,
  property splitting, ENABLED, configuration constants, tab columns,
  SUBSET and \subseteq, Bags, TLCGet and TLCSet, safety-like temporal
  properties, and TLC's message wherever a boolean, set or function is
  expected
- TLCExt: AssertError, AssertEq, TLCCache, TLCModelValue,
  TLCGetOrDefault, TLCNoOp, TLCGetAndSet; RealTime and Toolbox modules
- RandomSubset draws from up to 2^31 - 1 numbers without making the
  interval; Init draws on from the assumptions' generator
- a stored state takes one byte for a small number: half the memory for
  states of records (PaxosCommit 2.1 GB -> 1.1 GB)
- a state-level instance substitution is computed once per state
  (refinement bench 0.11 s -> 0.07 s)
- faster comparisons of sets and tuples, r.f, and standard operators
  through instances (Huang 66 s -> 43 s)
- computing initial states drops each candidate's garbage (MCBakery
  1.2 GB -> 0.4 GB)

2026-11-08 0.6.0:

- TLC 1.8.0 (d8fb231) is the oracle; every test is blessed by it, and
  tpp follows its messages, warnings, exit statuses, precedence, step
  labels, random draws and simulation
- ALIAS and POSTCONDITION

2026-11-07 0.5.1:

- make builds tpp profile-guided, 10 to 25% faster; make release removed
- builds without warnings under GCC 15
- examples/DieHard

2026-11-07 0.5.0:

- faster set operations, products, record and function sets, strings,
  constant sets and symmetry (partition 0.27 s -> 0.19 s, GameOfLife
  11.9 s -> 2.8 s, MCBakery 4.5 s -> 2.6 s)
- liveness checking takes 30% less memory and no longer leaks
- UNION S stays lazy as in TLC (MCDisruptor_MPMC now matches TLC)
- constant definitions and assumptions keep only their value
  (MCReachabilityTestAllGraphs 5 GB -> 3 MB)
- fixed: ENABLED recomputed constants, so a RandomElement could differ
- TLC's errors for infinite sets in states and fingerprint stack traces

2026-11-07 0.4.0:

- includes 0.3.0, never tagged
- -view, -userFile, -continue, -difftrace and -simulate file=FILE, also
  in the C API; -fp, -fpmem, -fpbits, -gzip and -cleanup are ignored
- random draws, TLCGet counters and simulation levels as TLC 2.19
- configuration checks, SANY's level rules and duplicate-definition
  warnings, as TLC and SANY
- edge cases from tlaplus/Examples and other public specs: lazy unions,
  recursive functions in actions, UNCHANGED through instances and
  parameters, VIEW with SYMMETRY, TLCSet("exit"), constant definitions
  as Init and Next
- tuples, sets, records, CASE arms and bound names of any number
- symmetry reduction 3x faster
- fixed undefined behavior found by the sanitizers on tlaplus/Examples
- tests/tla/Workers.tla models the parallel search

2026-11-06 0.2.0:

- -dump dot FILE, -nowarning; -metadir is ignored
- instances, the configuration, SYMMETRY and \A in actions as TLC treats
  them
- simulation checks temporal properties as TLC does
- Any, TLCSet, TLCGet
- more benchmarks: liveness, symmetry, partition, recursion, refinement

2026-11-06 0.1.0:

- first release: a TLA+ model checker that checks what TLC checks and
  prints what TLC 2.19 prints
- assumptions, invariants, action properties, deadlock and temporal
  properties under fairness; -workers N with one worker's output
- random simulation; TLC's error messages and exit statuses
- the C API (tpp.h)
- binaries for macOS, Linux and Windows (Cygwin; delete the bundled
  cygwin1.dll inside a Cygwin shell)
