# Optimizations

What made tpp fast, what was tried and gave nothing, and how to measure.
Each entry gives wall-clock seconds on the bench specs (arm64, best of 5)
before and after; [benchmark.md](benchmark.md) has the current comparison
with TLC.

## Measuring

- `bench/quick.sh [-workers N] OLD NEW`: the best of 5 runs of each engine
  on each bench spec, side by side. Compare binaries built from two
  commits, at the same time; nothing else may run meanwhile (a stray
  background job once hid a result).
- `sample PID 3 -file out.sample` on a build with `-O2 -g`, run on a model
  big enough to sample (counters with `Max = 8`: 4.78M states). Self
  samples point at the hot function; to split an inlined one, build a copy
  with `__attribute__((noinline))` on its parts.
- To count work instead of time, build a copy that counts `eval` calls by
  kind and arena allocations by caller (`__builtin_return_address`). For
  counters: 148 evaluations and 53 allocations per state expanded.
- `make bench` re-measures [benchmark.md](benchmark.md) against TLC with the
  profile-guided build; do it before a release.

## Kept

In the order they were made.

| change | effect |
|---|---|
| Fingerprints hash values directly, not their encoding; the encoding is made only for new states; a fingerprint slot keeps the state beside it | counters 2.34 → 1.63 |
| Standard-module operators of up to four arguments take them on the stack | (in the above) |
| Workers: forked processes expand wide levels over a shared store; the search merges in order, so output is that of one process | counters 1.62 → 0.58 with 8 |
| `sigsetjmp(.., 0)`: no signal-mask system calls per handler | (in the above) |
| A binding read from the current state keeps its value for the state's expansion | closure 284 → 1.12 |
| Workers claim a new state once: one invariant check per state, not per worker | duplicate work no longer grows with workers |
| Union of sorted sets merges; small sets sort by insertion; arrays the caller fills are not zeroed | closure 1.16 → 0.90, counters 1.66 → 1.48 |
| Workers merge each batch as it is done and find seen states themselves | counters 0.46 → 0.38 with 8 |
| Atoms and tuples compare without the general case | closure 0.86 → 0.83 |
| An argument that is a bound name with a fixed value binds the value, not a thunk; `x \in S` quantifies without a stack | closure 0.83 → 0.70, counters 1.27 → 1.18 |
| Fingerprints: an atom hashes inline | counters 1.18 → 1.15 |
| Fingerprints are sums of element hashes, kept by each set and function, so EXCEPT and Append update them by one term and decoding computes them | counters 1.15 → 1.04 |
| `eval` is an inline lookup in a table of one function per kind, so each call site has its own indirect branch to predict | closure 0.68 → 0.60, counters 1.04 → 0.88 |
| Set membership compares atoms and flat tuples inline in its binary search | closure 0.61 → 0.57 |
| EXCEPT looks its key up once, not three times | counters 0.88 → 0.86 |
| `-workers N` is N processes: the search expands a batch while it waits for one, instead of spinning | counters (4.78M states) 1.65 → 1.55 with 8 |
| Workers take room in the claim table 64 claims at a time, not one atomic add per claim | counters 0.28 → 0.23 with 8 |
| Profile-guided optimization (`make`), trained on the conformance suite (training on the bench specs instead gains about as much, so the suite is a fair profile) | clang: closure 0.55 → 0.48, counters 0.85 → 0.72, queue 0.18 → 0.16; gcc 15: counters 1.00 → 0.82 |
| An instance's substitution of state level, but a variable, is computed once per state, as TLC keeps a LazyValue | refinement with K = 8 2.09 → 1.55, counters (no instance) +1% |
| `<<x, y>> \in S`, of atoms and an enumerated S, probes S with the components, not a tuple made of them | closure 0.48 → 0.46 |

## Tried and dropped

| idea | why not |
|---|---|
| A fast path for `K_DECL` in `eval` | no gain; unsafe for a constant whose `var_index` is 0 |
| Fingerprint table split into fingerprints and states | no gain |
| A wrapper evaluating leaves without the dispatch | slower |
| `-fno-stack-protector` | about 2%, not worth a build flag |
| `eval` as a switch calling one noinline function per kind | 2%; the table above does better |
| A tuple's value array in the same allocation as the tuple | no measurable gain |
| Prefetching fingerprint slots, successors queued 8 deep | none, slightly slower: the out-of-order core already overlaps the miss |
| Interning strings and model values (counters made 5.8M model values) | no gain: arena allocation is a pointer bump, and the scratch arena is reset each state, so it stays in cache |

## Where the time goes now

Counters, one worker: about 60% evaluating (148 tree nodes per state at
about 7 ns each), 35% keeping states (fingerprint lookup 16%, encoding,
records), 6% decoding. Closure: a third in set membership of tuples. Two
workers reach 1.6x one; the parallel path costs about 20% more work (claims,
encoding twice, a second lookup when merging).

The next large step for evaluation is fewer, cheaper nodes: resolving names
to slots, and fusing common shapes (`f[x]` with `f` a variable and `x` a
bound name, comparisons of integers) at resolve time.
