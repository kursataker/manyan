"""Check tlaplus/Examples' symbolic models with manyan -symbolic and Apalache; write docs/symbolic-bench.md.

usage: symbolic.py ENGINE --apalache "APALACHE CMD" --examples DIR [--library DIR]...
                   [--only SUBSTRING] [--timeout S] [--memory MB]

Every model the manifests check with Apalache runs once with each engine,
to the length 5 the Examples' CI checks, killed past --timeout seconds or
--memory MB resident. manyan must give Apalache's outcome: no error, or
the invariant violated, or a deadlock, in as many states, with its exit
status. Apalache's results are cached next to the Examples checkout.
"""
import argparse
import hashlib
import json
import math
import platform
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "tests"))
sys.path.insert(0, str(ROOT / "bench"))
from examples import run, stage  # noqa: E402
from symbolic_runner import apalache_outcome, manyan_outcome  # noqa: E402

LENGTH = "5"


def models(examples, only):
    for manifest in sorted(examples.glob("specifications/*/manifest.json")):
        for module in json.loads(manifest.read_text())["modules"]:
            for model in module["models"]:
                if model["mode"] == "symbolic" and only in model["path"]:
                    yield module, model


def outcome(output):
    """The outcome and exit status of run's normalized output."""
    body, _, code = output.rpartition("exit ")
    return body, int(code)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("engine")
    parser.add_argument("--apalache", required=True)
    parser.add_argument("--examples", required=True, type=Path)
    parser.add_argument("--library", action="append", default=[])
    parser.add_argument("--only", default="")
    parser.add_argument("--timeout", type=float, default=600)
    parser.add_argument("--memory", type=int, default=4000)
    args = parser.parse_args()
    engine = str(Path(args.engine).resolve())
    examples = args.examples.resolve()
    key = hashlib.sha1(f"{args.apalache} {examples} {args.library}".encode()).hexdigest()[:12]
    cache_path = examples.with_name(f"symbolicbench-apalache-{key}.json")
    cache = json.loads(cache_path.read_text()) if cache_path.exists() else {}
    rows, tallies, speedups = [], {}, []
    for module, model in models(examples, args.only):
        path = model["path"]
        name = path.removeprefix("specifications/").removesuffix(".cfg")
        cfg = Path(path).name
        with tempfile.TemporaryDirectory() as tmp:
            directory = Path(tmp).resolve() / "spec"
            stage(examples / Path(path).parent, args.library, directory)
            if path not in cache:
                command = args.apalache.split() + ["check", f"--length={LENGTH}", f"--config={cfg}",
                                                   f"--out-dir={Path(tmp).resolve()}/out", Path(module["path"]).name]
                seconds, output, killed = run(command, directory, args.timeout, args.memory)
                result = None
                if not killed:
                    log, code = outcome(output)
                    result = apalache_outcome(log, code, Path(tmp) / "out", (directory / cfg).read_text())
                cache[path] = {"seconds": seconds, "outcome": result, "killed": killed}
                cache_path.write_text(json.dumps(cache, indent=1))
            theirs = cache[path]
            command = [engine, "-symbolic", "-length", LENGTH, "-config", cfg, Path(module["path"]).stem]
            ours_seconds, output, killed = run(command, directory, args.timeout, args.memory)
        ours = None if killed else manyan_outcome(*outcome(output))
        if theirs["killed"]:
            verdict = f"Apalache {theirs['killed']}"
        elif killed:
            verdict = f"manyan {killed}"
        elif ours != theirs["outcome"]:
            verdict = "differs: " + ours.replace("\n", " ").strip()
        else:
            verdict = "same"
            speedups.append(theirs["seconds"] / ours_seconds)
        tallies[verdict.split(" over")[0].split(":")[0]] = tallies.get(verdict.split(" over")[0].split(":")[0], 0) + 1
        apalache = (theirs["outcome"] or "").split("\n")[0]
        manyan_time = "" if killed else f"{ours_seconds:.2f}"
        speedup = f"{theirs['seconds'] / ours_seconds:.1f}x" if verdict == "same" else ""
        rows.append(f"| {name} | {apalache} | {theirs['seconds']:.2f} | {manyan_time} | {speedup} | {verdict} |")
        print(rows[-1], flush=True)
    mean = math.exp(sum(map(math.log, speedups)) / len(speedups)) if speedups else 0
    summary = ", ".join(f"{n} {verdict}" for verdict, n in sorted(tallies.items(), key=lambda t: -t[1]))
    report = ROOT / "docs/symbolic-bench.md"
    head = report.read_text().split("<!-- results -->")[0] if report.exists() else "# Symbolic benchmark\n\n"
    report.write_text(head + "<!-- results -->\n\n"
                      f"tlaplus/Examples at {examples.name.removeprefix('Examples-')[:12]}, the models its manifests "
                      f"check with Apalache, to length {LENGTH}. Measured on {platform.machine()} "
                      f"{platform.system()} {platform.release()}, one run, wall-clock seconds.\n\n"
                      f"{len(rows)} models: {summary}. Where manyan gives Apalache's outcome, "
                      f"it is {mean:.1f}x as fast (geometric mean).\n\n"
                      "| model | outcome | Apalache | Manyan | speedup | result |\n"
                      "|---|---|---:|---:|---:|---|\n" + "\n".join(rows) + "\n")
    return 0 if tallies.get("same", 0) == len(rows) else 1


if __name__ == "__main__":
    sys.exit(main())
