"""Run the symbolic checking tests (tests/symbolic) on Manyan, with Apalache as the oracle.

usage: symbolic_runner.py MANYAN [--oracle APALACHE] [--bless] [--only SUBSTRING] [--strict]

A test is NAME.tla, a module and then its model configuration, as in
tests/spec; a line `\\* options: -length N -deadlock` gives the length (10
by default) and turns deadlock checking off. NAME.out is the outcome
Apalache gives: NoError, Error with the violated invariant, or Deadlock,
then the number of states in the counterexample, then the exit status.
The traces themselves are the solver's choice and are not compared;
Manyan replays each of its own through its evaluator.

With --oracle, Apalache (its apalache-mc) must give NAME.out too, which
validates the test; --bless writes NAME.out from it.
"""
import argparse
import re
import shlex
import subprocess
import sys
import tempfile
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

from spec_runner import ROOT, stage

TESTS = ROOT / "tests/symbolic"


def options(path):
    found = re.search(r"^\\\* options: (.*)$", path.read_text(), flags=re.M)
    words = shlex.split(found.group(1)) if found else []
    length = words[words.index("-length") + 1] if "-length" in words else "10"
    return length, "-deadlock" in words


def invariants(config):
    """The configuration's invariants, in order, as Apalache numbers them."""
    names = []
    for m in re.finditer(r"^\s*INVARIANTS?\b(.*?)(?=^\s*[A-Z_]+\b|\Z)", config, flags=re.M | re.S):
        names += m.group(1).split()
    return names


def manyan_outcome(output, code):
    states = len(re.findall(r"^State \d+: ", output, flags=re.M))
    violated = re.search(r"^Error: Invariant (\S+) is violated\.$", output, flags=re.M)
    if violated:
        return f"Error {violated.group(1)} {states}\nexit {code}\n"
    if re.search(r"^Error: Deadlock reached\.$", output, flags=re.M):
        return f"Deadlock {states}\nexit {code}\n"
    if "No error has been found in behaviors" in output:
        return f"NoError\nexit {code}\n"
    if "No error has been found. No " in output:
        return f"ExecutionsTooShort\nexit {code}\n"
    error = re.search(r"^Error: (.*)$", output, flags=re.M)
    return f"Failure {error.group(1) if error else output.strip()[-200:]}\nexit {code}\n"


def apalache_outcome(output, code, directory, config):
    outcome = re.search(r"The outcome is: (\w+)", output)
    if not outcome:
        error = re.search(r"^.*(?:error|Error|ERROR).*$", output, flags=re.M)
        return f"Failure {error.group(0).strip() if error else 'no outcome'}\nexit {code}\n"
    outcome = outcome.group(1)
    if outcome in ("NoError", "ExecutionsTooShort"):
        return f"{outcome}\nexit {code}\n"
    traces = sorted(directory.rglob("violation*.tla"))
    states = len(re.findall(r"^State\d+ ==", traces[0].read_text(), flags=re.M)) if traces else 0
    if outcome == "Deadlock":
        return f"Deadlock {states}\nexit {code}\n"
    which = re.search(r"State \d+: state invariant (\d+) violated", output)
    names = invariants(config)
    name = names[int(which.group(1))] if which and int(which.group(1)) < len(names) else "?"
    return f"{outcome} {name} {states}\nexit {code}\n"


def run(command, path, oracle):
    length, no_deadlock = options(path)
    with tempfile.TemporaryDirectory() as tmp:
        directory = Path(tmp).resolve()
        stage(path, directory)
        cfg = path.stem + ".cfg"
        if oracle:
            args = command + ["check", f"--config={cfg}", f"--length={length}", f"--out-dir={directory}/out"]
            args += ["--no-deadlock"] if no_deadlock else []
            args += [path.name]
        else:
            args = command + ["-symbolic", "-length", length] + (["-deadlock"] if no_deadlock else []) + ["-config", cfg, path.stem]
        try:
            done = subprocess.run(args, cwd=directory, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, timeout=600)
            output, code = done.stdout, done.returncode
        except subprocess.TimeoutExpired:
            return "timeout\n", "timeout\n"
        if oracle:
            return apalache_outcome(output, code, directory, (directory / cfg).read_text()), output
        return manyan_outcome(output, code), output


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("engine")
    parser.add_argument("--oracle")
    parser.add_argument("--bless", action="store_true")
    parser.add_argument("--only", default="")
    parser.add_argument("--strict", action="store_true")
    parser.add_argument("--verbose", action="store_true")
    args = parser.parse_args()
    engine = shlex.split(args.engine)
    if "/" in engine[0]:
        engine[0] = str(Path(engine[0]).resolve())
    oracle = shlex.split(args.oracle) if args.oracle else None
    if oracle and "/" in oracle[0]:
        oracle[0] = str(Path(oracle[0]).resolve())
    tests = sorted(p for p in TESTS.glob("*.tla") if args.only in p.name)

    def check(path):
        expected_path = path.with_suffix(".out")
        notes = []
        if args.oracle:
            reference, log = run(oracle, path, True)
            if args.bless:
                expected_path.write_text(reference)
            elif not expected_path.exists() or reference != expected_path.read_text():
                notes.append("oracle disagrees with " + expected_path.name + ": " + reference.replace("\n", " "))
        if not expected_path.exists():
            return path, False, ["no " + expected_path.name], ""
        actual, output = run(engine, path, False)
        if actual != expected_path.read_text():
            notes.append("outcome differs: " + actual.replace("\n", " "))
        return path, not notes, notes, output

    failed = 0
    with ThreadPoolExecutor(max_workers=2 if args.oracle else 8) as pool:
        for path, ok, notes, output in pool.map(check, tests):
            if not ok:
                failed += 1
                print("FAIL", path.relative_to(ROOT), "-", "; ".join(notes))
                if args.verbose:
                    print(output)
    print(f"{'symbolic':24} {len(tests) - failed:4}/{len(tests)}")
    return 1 if failed and args.strict else 0


if __name__ == "__main__":
    sys.exit(main())
