# AGENTS.md

## Code taste

Before you finish, ask:

- What can I delete until nothing cannot be deleted?
- Would a newcomer read this without help?
- Did I solve the real problem?
- Did I followed YAGNI (You ain't gonna need it)?
- Does it match its neighbours?

Trust your discomfort. If something feels ugly, find out why and fix it. Check what means not ugly:

* **Is simple.** Remove parts until nothing more can go. As few concepts and branches as possible.
* **Solves the right problem.** Check the problem first. A neat answer to the wrong question is waste.
* **Looks easy.** Hard work hides behind a clear result. Keep refining until it 
* **Is timeless.** Prefer designs that will still make sense in ten years over fashion.
* **Uses symmetry.** Make similar things look and behave alike. Break the pattern only for a reason.
* **Fits its parts to the whole.** Follow how the system already works. Do not fight it.
* **Is suggestive.** A good core makes the next feature obvious. Prefer general tools over special cases.
* **Is strong, not fussy.** Skip needless ornament, config, comment and abstration.
* **Is redesigned.** Expect the first draft/implementation to be wrong. Rewrite it when you see better.
* **Can borrow.** Copy good designs, then adopt. Imitation teaches taste.
* **May be odd.** A bold or unusual choice is fine when it is simpler or truer.
* **Keep stable, strong, and speedy.** API stability, rock solid testing and verification, and high performance are features.
* **Polishing.** Good code does not yield once at a time. They come after iterations until no extra can be simplified or trimmed.

These preferences do not authorize sending messages, publishing, or pushing
changes. Routine local implementation, testing, and documentation are authorized
by the task and do not require repeated confirmation.

## Workflow

* Commit as often as you can when you made progress.
* Docs, TLA+ Models, Tests, Examples should not break and benchmarks has no degradation before push or release or releasing a new version unless user accepts.
* Commit message should come with a brief of test or bench results.
* Before tagging and pushing a release, re-measure `docs/benchmark.md` at the
  release commit (`bench/compare.py`, `make bench`) and commit the report.
  Stop at any bench issue (a regression against the last report, a wrong or
  failing program, an engine that cannot be measured) and report it instead
  of tagging.
