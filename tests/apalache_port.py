"""Port Apalache's integration tests of `apalache-mc check` to tests/symbolic/apalache,
and of `apalache-mc typecheck` to tests/typecheck/apalache.

usage: apalache_port.py APALACHE_SOURCE COMMUNITY_MODULES

APALACHE_SOURCE is a checkout of apalache-mc/apalache, COMMUNITY_MODULES
an unpacked release of tlaplus/CommunityModules. Each `apalache-mc
check` of test/tla/cli-integration-tests.md that checks a .tla file for
invariants, temporal properties and deadlock, with the options -symbolic has, and whose
outcome is no error or a violation, becomes a test: the module, the model
configuration its options make, Apalache's defaults Init and Next, and
an options line; the modules it uses in NAME.files, from beside it, from
Apalache's library (Variants, Option) or from the Community Modules,
whose definitions Apalache's rewired copies stand for. A module checked more
than once is renamed for each further check. The expected outcomes are
then Apalache's own: make bless-symbolic.

Each `apalache-mc typecheck` of a .tla file becomes a test of the module
alone, with the modules it uses; its outcome is Apalache's own too: make
bless-typecheck.
"""
import re
import shlex
import shutil
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
OUT = ROOT / "tests/symbolic/apalache"
TYPECHECK = ROOT / "tests/typecheck/apalache"
# Options that do not change the outcome, and those -symbolic has.
IGNORED = {"--out-dir", "--run-dir", "--debug", "--write-intermediate", "--discard-disabled", "--smt-solver",
           "--profiling", "--smtprof", "--output-traces"}
KNOWN = {"--inv", "--temporal", "--length", "--init", "--next", "--config", "--cinit", "--no-deadlock"}
# The modules manyan builds in, Apalache's among them.
BUILT_IN = {"Naturals", "Integers", "Reals", "Sequences", "FiniteSets", "TLC", "Bags", "Apalache"}
# The modules Apalache reads in place of a module of that name, as its
# StandardLibrary.wiredModules; manyan builds in TLC, Sequences and Bags.
WIRED = {"BagsExt": "__rewire_bags_ext_in_apalache", "Functions": "__rewire_functions_in_apalache",
         "FiniteSetsExt": "__rewire_finite_sets_ext_in_apalache", "SequencesExt": "__rewire_sequences_ext_in_apalache",
         "Folds": "__rewire_folds_in_apalache"}
# The tests ported by name that -symbolic does not take, and why.
SKIP = {
    "TestFunctions": "Apalache runs out of 4 GB of heap",
    "InfDomFun": "Apalache reports f[0] = 42 violated, where f[0] is 42", "FunInInfiniteSubset": "a function over an infinite domain",
    "SafeMath": "integers of more than 64 bits", "SafeMath_2": "integers of more than 64 bits",
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


def typechecks(md):
    """The modules `apalache-mc typecheck` checks, each once."""
    for block in re.findall(r"```sh\n(.*?)```", md, flags=re.S):
        command = re.match(r"\$ apalache-mc typecheck (?:--output=\S+ )?(\w+)\.tla", block.split("\n")[0])
        if command:
            yield command.group(1)


def clear(directory):
    """Empty directory, but for the outcomes Apalache gave."""
    directory.mkdir(parents=True, exist_ok=True)
    for old in directory.iterdir():
        if old.suffix != ".out":
            shutil.rmtree(old) if old.is_dir() else old.unlink()


def copy(module, directory, directories, wired):
    """Write module to directory with the modules it uses in a sibling MODULE.files,
    those Apalache rewires from wired."""
    seen = {}
    used(module, directories, seen, wired)
    shutil.copy(seen.pop(module), directory)
    if seen:
        (directory / f"{module}.files").mkdir(exist_ok=True)
        for name, other in seen.items():
            shutil.copy(other, directory / f"{module}.files" / f"{name}.tla")


def prune(directory):
    for out in directory.glob("*.out"):
        if not out.with_suffix(".tla").exists():
            out.unlink()
    print(len(list(directory.glob("*.tla"))), "tests in", directory.relative_to(ROOT))


def used(module, directories, seen, wired=None):
    """The modules that module extends or instantiates, itself too, by name:
    each from the first of directories that has it, and what a library's
    module uses from that library on; from wired, the modules Apalache
    reads in place of others."""
    if module in seen or module in BUILT_IN:
        return
    if wired and module in WIRED:
        path, directories = wired / f"{WIRED[module]}.tla", [wired]
    else:
        at = next((i for i, d in enumerate(directories) if (d / f"{module}.tla").exists()), None)
        if at is None:
            return
        path = directories[at] / f"{module}.tla"
        directories = directories[at:] if at else directories
    seen[module] = path
    text = re.sub(r"\(\*.*?\*\)|\\\*[^\n]*", "", path.read_text(), flags=re.S)
    for names in re.findall(r"\bEXTENDS\b([\w\s,]+)", text):
        for name in re.findall(r"\w+", names):
            used(name, directories, seen, wired)
    for name in re.findall(r"\bINSTANCE\s+(\w+)", text):
        used(name, directories, seen, wired)


def main():
    source = Path(sys.argv[1]) / "test/tla"
    libraries = [Path(sys.argv[1]) / "src/tla", Path(sys.argv[2])]
    md = (source / "cli-integration-tests.md").read_text()
    clear(TYPECHECK)
    for module in dict.fromkeys(typechecks(md)):
        if (source / f"{module}.tla").exists():
            copy(module, TYPECHECK, [source] + libraries, libraries[0])
    prune(TYPECHECK)
    clear(OUT)
    names, done = {}, set()
    for words, status in checks(md):
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
        parts += [("PROPERTY", " " + p) for p in options.get("--temporal", "").split(",") if p]
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
    prune(OUT)


if __name__ == "__main__":
    main()
