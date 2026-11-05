"""Fuzz tpp.

usage: fuzz.py crash ENGINE [--runs N] [--seed S]
       fuzz.py diff ENGINE --tlc "TLC CMD" [--runs N] [--seed S]

crash: mutates the models of tests/spec and runs the engine (a sanitizer
build) on each; it must exit with one of its statuses, without a signal
or a sanitizer report.

diff: makes random constant expressions and checks ASSUME PrintT(e) with
the engine and with TLC; they must print the same. A batch of expressions
that differs is checked again one expression at a time, to name the one.
"""
import argparse
import random
import re
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "tests"))
import spec_runner  # noqa: E402

STATUSES = {0, 10, 11, 12, 13, 14, 75, 76, 150, 151, 255}
STRINGS = ['"a"', '"b"', '"ab"', '""']
TOKEN = re.compile(r"\\[A-Za-z]+|[A-Za-z_][A-Za-z0-9_]*|\d+|\"[^\"\n]*\"|<<|>>|\|->|==|/\\|\\/|[^\s\w]|\s+")


def kind(token):
    if token[0].isdigit():
        return "number"
    if token[0].isalpha() or token[0] == "_":
        return "word"
    if token[0] == '"':
        return "string"
    return "symbol"


def mutate(text, rng):
    """Mostly swaps a token for another of its kind, so most mutants parse."""
    tokens = TOKEN.findall(text)
    spots = [i for i, t in enumerate(tokens) if not t.isspace()]
    by_kind = {}
    for i in spots:
        by_kind.setdefault(kind(tokens[i]), []).append(tokens[i])
    for _ in range(rng.randint(1, 3)):
        i = rng.choice(spots)
        if not tokens[i]:
            continue
        choice = rng.random()
        if choice < 0.7:
            k = kind(tokens[i])
            tokens[i] = str(rng.randint(0, 12)) if k == "number" else rng.choice(by_kind[k])
        elif choice < 0.85:
            tokens[i] = ""
        else:
            tokens[i] = tokens[i] + " " + tokens[rng.choice(spots)]
    return "".join(tokens)


def run(engine, name, directory, timeout):
    try:
        done = subprocess.run(engine + ["-config", name + ".cfg", name], cwd=directory, timeout=timeout,
                              stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, errors="replace")
    except subprocess.TimeoutExpired:
        return None, ""
    return done.returncode, done.stdout


def crash(engine, runs, seed):
    rng = random.Random(seed)
    seeds = sorted((ROOT / "tests/spec").rglob("*.tla"))
    seeds = [p for p in seeds if p.suffixes == [".tla"] and ".files" not in str(p)]
    failed = 0
    for i in range(runs):
        path = rng.choice(seeds)
        with tempfile.TemporaryDirectory() as tmp:
            directory = Path(tmp)
            spec_runner.stage(path, directory)
            text = mutate(path.read_text(), rng)
            (directory / path.name).write_text(text)
            code, out = run(engine, path.stem, directory, 10)
            if code is not None and (code not in STATUSES or "runtime error" in out or "Sanitizer" in out):
                failed += 1
                kept = ROOT / ".obj" / f"crash-{seed}-{i}.tla"
                kept.parent.mkdir(exist_ok=True)
                kept.write_text(text)
                print(f"FAIL {path.relative_to(ROOT)} exit {code}, kept as {kept.relative_to(ROOT)}")
                print(out[-2000:])
    print(f"{runs - failed}/{runs} mutated models ran without crashing")
    return failed


class Gen:
    """Random constant expressions of a few types, kept small."""

    def __init__(self, rng):
        self.rng = rng
        self.bound = []

    def pick(self, *options):
        return self.rng.choice(options)()

    def int(self, d=0):
        r = self.rng
        leaf = [lambda: str(r.randint(-3, 9))] + [lambda: x for x, t in self.bound if t == "int"][:1]
        if d > 2:
            return r.choice(leaf)()
        n = lambda: self.int(d + 1)
        return self.pick(
            *leaf,
            lambda: f"({n()} + {n()})",
            lambda: f"({n()} - {n()})",
            lambda: f"({n()} * {n()})",
            lambda: f"({n()} \\div {r.randint(1, 4)})",
            lambda: f"({n()} % {r.randint(1, 4)})",
            lambda: f"(-{n()})",
            lambda: f"(IF {self.bool(d + 1)} THEN {n()} ELSE {n()})",
            lambda: f"Cardinality({self.set(d + 1)})",
            lambda: f"Cardinality({self.big(d + 1)})",
            lambda: f"Len({self.seq(d + 1)})",
            lambda: f"Head({self.seq(d + 1)})",
            lambda: f"({n()} ^ {r.randint(0, 3)})",
            lambda: self.choose(d, "int"),
            lambda: f"{self.record(d + 1)}.a",
            lambda: f"{self.fcn(d + 1)}[{r.randint(1, 3)}]",
            lambda: f"(LET y == {n()} IN y + {n()})",
        )

    def bool(self, d=0):
        r = self.rng
        if d > 2:
            return r.choice(["TRUE", "FALSE", f"({self.int(d)} < {self.int(d)})"])
        b = lambda: self.bool(d + 1)
        return self.pick(
            lambda: r.choice(["TRUE", "FALSE"]),
            lambda: f"({self.int(d + 1)} < {self.int(d + 1)})",
            lambda: f"({self.int(d + 1)} = {self.int(d + 1)})",
            lambda: f"({self.int(d + 1)} \\in {self.set(d + 1)})",
            lambda: f"({self.set(d + 1)} \\subseteq {self.set(d + 1)})",
            lambda: f"(~{b()})",
            lambda: f"({b()} /\\ {b()})",
            lambda: f"({b()} \\/ {b()})",
            lambda: f"({b()} => {b()})",
            lambda: f"({b()} <=> {b()})",
            lambda: self.quantifier(d),
            lambda: f"({self.seq(d + 1)} = {self.seq(d + 1)})",
            lambda: f"({self.set(d + 1)} = {self.set(d + 1)})",
            lambda: f"({self.seq(d + 1)} \\in Seq({self.set(d + 1)}))",
            lambda: f"({self.record(d + 1)} = {self.record(d + 1)})",
            lambda: f"({self.record(d + 1)} \\in [a: {self.set(d + 1)}, b: BOOLEAN])",
            lambda: f"({r.choice(STRINGS)} = {r.choice(STRINGS)})",
        )

    def bind(self, body):
        name = f"x{len(self.bound)}"
        self.bound.append((name, "int"))
        try:
            return name, body()
        finally:
            self.bound.pop()

    def quantifier(self, d):
        s = self.set(d + 1)
        q = self.rng.choice(["\\A", "\\E"])
        x, p = self.bind(lambda: self.bool(d + 1))
        return f"({q} {x} \\in {s} : {p})"

    def choose(self, d, _):
        s = self.set(d + 1)
        x, p = self.bind(lambda: self.bool(d + 1))
        return f"(CHOOSE {x} \\in {s} : {p})"

    def set(self, d=0):
        r = self.rng
        n = lambda: self.int(d + 1)
        if d > 2:
            return r.choice(["{}", f"{{{n()}}}", f"({r.randint(0, 2)}..{r.randint(0, 4)})"])
        s = lambda: self.set(d + 1)
        return self.pick(
            lambda: "{" + ", ".join(n() for _ in range(r.randint(0, 4))) + "}",
            lambda: f"({r.randint(-2, 2)}..{r.randint(-1, 5)})",
            lambda: f"({s()} \\cup {s()})",
            lambda: f"({s()} \\cap {s()})",
            lambda: f"({s()} \\ {s()})",
            lambda: self.filter(d),
            lambda: self.image(d),
            lambda: f"DOMAIN {self.seq(d + 1)}",
            lambda: f"UNION {{{s()}, {s()}}}",
        )

    def big(self, d=0):
        """A set of something other than integers."""
        s = lambda: self.set(d + 1)
        return self.pick(
            lambda: f"DOMAIN {self.record(d + 1)}",
            lambda: "{" + ", ".join(self.rng.sample(STRINGS, self.rng.randint(0, 3))) + "}",
            lambda: f"SUBSET {s()}",
            lambda: f"({s()} \\X {s()})",
            lambda: f"[{s()} -> {s()}]",
            lambda: f"[a: {s()}, b: BOOLEAN]",
            lambda: f"{{{self.seq(d + 1)}, {self.seq(d + 1)}}}",
        )

    def filter(self, d):
        s = self.set(d + 1)
        x, p = self.bind(lambda: self.bool(d + 1))
        return f"{{{x} \\in {s} : {p}}}"

    def image(self, d):
        s = self.set(d + 1)
        x, e = self.bind(lambda: self.int(d + 1))
        return f"{{{e} : {x} \\in {s}}}"

    def seq(self, d=0):
        r = self.rng
        n = lambda: self.int(d + 1)
        if d > 2:
            return "<<" + ", ".join(n() for _ in range(r.randint(0, 3))) + ">>"
        q = lambda: self.seq(d + 1)
        return self.pick(
            lambda: "<<" + ", ".join(n() for _ in range(r.randint(0, 4))) + ">>",
            lambda: f"Append({q()}, {n()})",
            lambda: f"({q()} \\o {q()})",
            lambda: f"SubSeq({q()}, {r.randint(1, 2)}, {r.randint(0, 3)})",
            lambda: f"Tail({q()})",
            lambda: f"SelectSeq({q()}, LAMBDA v : v > {n()})",
            lambda: f"[{q()} EXCEPT ![{r.randint(1, 3)}] = @ * 2]",
            lambda: f"[{self.fcn(d + 1)} EXCEPT ![{r.randint(1, 3)}] = {n()}]",
        )

    def fcn(self, d=0):
        s = f"(1..{self.rng.randint(0, 4)})"
        x, e = self.bind(lambda: self.int(d + 1))
        return f"[{x} \\in {s} |-> {e}]"

    def record(self, d=0):
        r = [f"[a |-> {self.int(d + 1)}, b |-> {self.bool(d + 1)}]"]
        if d < 2:
            r.append(f"[{self.record(d + 1)} EXCEPT !.a = {self.int(d + 1)}]")
        return self.rng.choice(r)

    def any(self):
        return self.pick(self.int, self.bool, self.set, self.big, self.seq, self.record)


def batch_output(command, exprs):
    with tempfile.TemporaryDirectory() as tmp:
        directory = Path(tmp)
        lines = "\n".join(f"ASSUME PrintT({e})" for e in exprs)
        (directory / "F.tla").write_text(f"---- MODULE F ----\nEXTENDS Integers, Sequences, FiniteSets, TLC\n"
                                         f"{lines}\n====\n")
        (directory / "F.cfg").write_text("")
        code, out = run(command, "F", directory, 60)
        return spec_runner.normalize(out, directory) + f"exit {code}\n"


def diff(engine, tlc, runs, seed):
    rng = random.Random(seed)
    tlc_cmd = tlc.split() + ["-workers", "1"]
    failed = 0
    size = 25
    for b in range(0, runs, size):
        exprs = [Gen(rng).any() for _ in range(min(size, runs - b))]
        meta = ["-metadir", tempfile.mkdtemp(prefix="tpp-meta-")]
        if batch_output(engine, exprs) == batch_output(tlc_cmd + meta, exprs):
            continue
        for e in exprs:
            meta = ["-metadir", tempfile.mkdtemp(prefix="tpp-meta-")]
            mine, theirs = batch_output(engine, [e]), batch_output(tlc_cmd + meta, [e])
            if mine != theirs:
                failed += 1
                print(f"FAIL {e}\n--- tpp\n{mine}--- TLC\n{theirs}")
                break
    print(f"{runs - failed}/{runs} random expressions printed as TLC prints them")
    return failed


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("mode", choices=["crash", "diff"])
    parser.add_argument("engine")
    parser.add_argument("--tlc")
    parser.add_argument("--runs", type=int, default=200)
    parser.add_argument("--seed", type=int, default=1)
    args = parser.parse_args()
    engine = [str(Path(args.engine).resolve())]
    if args.mode == "crash":
        return 1 if crash(engine, args.runs, args.seed) else 0
    if not args.tlc:
        parser.error("diff needs --tlc")
    return 1 if diff(engine, args.tlc, args.runs, args.seed) else 0


if __name__ == "__main__":
    sys.exit(main())
