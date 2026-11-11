"""Port Apalache's integration tests of `apalache-mc check` to tests/symbolic/apalache.

usage: apalache_port.py APALACHE_SOURCE COMMUNITY_MODULES

APALACHE_SOURCE is a checkout of apalache-mc/apalache, COMMUNITY_MODULES
an unpacked release of tlaplus/CommunityModules. Each `apalache-mc
check` of test/tla/cli-integration-tests.md that checks a .tla file for
invariants and deadlock, with the options -symbolic has, and whose
outcome is no error or a violation, becomes a test: the module, the model
configuration its options make, Apalache's defaults Init and Next, and
an options line; the modules it uses in NAME.files, from beside it, from
Apalache's library (Variants, Option) or from the Community Modules,
whose definitions Apalache's rewired copies stand for. A module checked more
than once is renamed for each further check. The expected outcomes are
then Apalache's own: make bless-symbolic.
"""
import re
import shlex
import shutil
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
OUT = ROOT / "tests/symbolic/apalache"
# Options that do not change the outcome, and those -symbolic has.
IGNORED = {"--out-dir", "--run-dir", "--debug", "--write-intermediate", "--discard-disabled", "--smt-solver",
           "--profiling", "--smtprof", "--output-traces"}
KNOWN = {"--inv", "--length", "--init", "--next", "--config", "--cinit", "--no-deadlock"}
# The modules manyan builds in, Apalache's among them.
BUILT_IN = {"Naturals", "Integers", "Reals", "Sequences", "FiniteSets", "TLC", "Bags", "Apalache"}
# The tests ported by name that -symbolic does not take, and why.
SKIP = {
    "TestFunctions": "Apalache runs out of 4 GB of heap",
    "Bug2973": "Gen needs Apalache's types", "Bug2973_2": "Gen needs Apalache's types",
    "Bug3158": "Gen needs Apalache's types",
    "InfDomFun": "a function over an infinite domain", "FunInInfiniteSubset": "a function over an infinite domain",
    "Invariants_5": "a trace invariant", "Invariants_6": "a trace invariant",
    "SafeMath": "integers of more than 64 bits", "SafeMath_2": "integers of more than 64 bits",
    "Slicer5": "primes in the initial predicate", "UnchangedExpr471": "primes in the initial predicate",
    "HandshakeWithTypes_2": "the evaluator replays \\E over Int", "HandshakeWithTypes_5": "the evaluator replays \\E over Int",
    "FoldSetInInit": "ApaFoldSet over SUBSET (1..10) takes more than 1 GB",
}


KEYWORDS = r"CONSTANTS?|INIT|NEXT|SPECIFICATION|INVARIANTS?|PROPERTY|PROPERTIES|CONSTRAINTS?|ACTION_CONSTRAINTS?|SYMMETRY|VIEW|CHECK_DEADLOCK|POSTCONDITION|ALIAS"


def sections(config):
    """A configuration's statements: each keyword with the text up to the next."""
    config = re.sub(r"\\\*[^\n]*|\(\*.*?\*\)", "", config, flags=re.S)
    parts = re.split(rf"\b({KEYWORDS})\b", config)
    return [(parts[i], parts[i + 1]) for i in range(1, len(parts), 2)]


def checks(md):
    for block in re.findall(r"```sh\n(.*?)```", md, flags=re.S):
        command = re.match(r"\$ apalache-mc check (.*?)(?:\s*\||\s*2>&1|\s*$)", block.split("\n")[0])
        status = re.search(r"EXITCODE: (OK|ERROR \(12\))", block)
        if command and status:
            yield shlex.split(command.group(1)), status.group(1)


def used(module, directories, seen):
    """The modules that module extends or instantiates, itself too, by name:
    each from the first of directories that has it, and what a library's
    module uses from that library on."""
    at = next((i for i, d in enumerate(directories) if (d / f"{module}.tla").exists()), None)
    if module in seen or module in BUILT_IN or at is None:
        return
    path = seen[module] = directories[at] / f"{module}.tla"
    directories = directories[at:] if at else directories
    text = re.sub(r"\(\*.*?\*\)|\\\*[^\n]*", "", path.read_text(), flags=re.S)
    for names in re.findall(r"\bEXTENDS\b([\w\s,]+)", text):
        for name in re.findall(r"\w+", names):
            used(name, directories, seen)
    for name in re.findall(r"\bINSTANCE\s+(\w+)", text):
        used(name, directories, seen)


def main():
    source = Path(sys.argv[1]) / "test/tla"
    libraries = [Path(sys.argv[1]) / "src/tla", Path(sys.argv[2])]
    OUT.mkdir(parents=True, exist_ok=True)
    for old in OUT.iterdir():   # the outcomes Apalache gave stay
        if old.suffix != ".out":
            shutil.rmtree(old) if old.is_dir() else old.unlink()
    names, done = {}, set()
    for words, status in checks((source / "cli-integration-tests.md").read_text()):
        options, files = {}, [w for w in words if not w.startswith("--")]
        flags = [w.split("=", 1) for w in words if w.startswith("--")]
        if len(files) != 1 or not files[0].endswith(".tla") or any(f[0] not in KNOWN | IGNORED for f in flags):
            continue
        for flag in flags:
            if flag[0] in KNOWN:
                options[flag[0]] = flag[1] if len(flag) > 1 else ""
        path = source / files[0]
        key = (files[0], tuple(sorted(options.items())))
        if not path.exists() or key in done:
            continue
        done.add(key)
        parts = sections((path.parent / options["--config"]).read_text()) if "--config" in options else []
        if any(k in ("PROPERTY", "PROPERTIES") for k, _ in parts):
            continue
        module = path.stem
        names[module] = names.get(module, 0) + 1
        name = module if names[module] == 1 else f"{module}_{names[module]}"
        text = path.read_text()
        if name in SKIP:
            continue
        if name != module:
            text = re.sub(rf"(MODULE\s+){module}\b", rf"\g<1>{name}", text, count=1)
        given = {k for k, _ in parts}
        if "--init" in options or not given & {"INIT", "SPECIFICATION"}:
            parts = [p for p in parts if p[0] not in ("INIT", "SPECIFICATION")] + [("INIT", " " + options.get("--init", "Init"))]
        if "--next" in options or not given & {"NEXT", "SPECIFICATION"}:
            parts = [p for p in parts if p[0] not in ("NEXT", "SPECIFICATION")] + [("NEXT", " " + options.get("--next", "Next"))]
        parts += [("INVARIANT", " " + inv) for inv in options.get("--inv", "").split(",") if inv]
        config = "".join(f"{k} {v.strip()}\n" for k, v in parts)
        line = f"-length {options.get('--length', '10')}"
        line += " -deadlock" if "--no-deadlock" in options else ""
        line += f" -cinit {options['--cinit']}" if "--cinit" in options else ""
        (OUT / f"{name}.tla").write_text(f"\\* options: {line}\n\\* apalache-mc check {' '.join(words)}: {status}\n"
                                        + text.rstrip("\n") + "\n" + config)
        seen = {}
        used(module, [path.parent] + libraries, seen)
        del seen[module]
        if seen:
            (OUT / f"{name}.files").mkdir(exist_ok=True)
            for other in seen.values():
                shutil.copy(other, OUT / f"{name}.files")
    for out in OUT.glob("*.out"):
        if not out.with_suffix(".tla").exists():
            out.unlink()
    print(len(list(OUT.glob("*.tla"))), "tests")


if __name__ == "__main__":
    main()
