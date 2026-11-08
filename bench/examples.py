"""Check tlaplus/Examples' models with tpp and TLC; write docs/fullbench.md.

usage: examples.py ENGINE --tlc "TLC CMD" --examples DIR [--library DIR]...
                   [--only SUBSTRING] [--max-runtime HH:MM:SS] [--timeout S] [--memory MB]

Every exhaustive-search model whose recorded runtime is at most
--max-runtime runs once with each engine and one worker, killed past
--timeout seconds or --memory MB resident. tpp must print what TLC prints.
Each library's modules are staged beside the spec unless the spec has one
of that name or tpp builds it in, as TLC finds them on its class path.
TLC's results are cached next to the Examples checkout: they change only
with the pins. The diffs of the models that differ go to fullbench.diff.
"""
import argparse
import difflib
import hashlib
import json
import math
import os
import platform
import re
import shutil
import signal
import subprocess
import sys
import tempfile
import threading
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "tests"))
import spec_runner  # noqa: E402

BUILT_IN = {"Naturals", "Integers", "Reals", "Sequences", "FiniteSets", "TLC", "Bags",
            "Randomization", "RealTime", "Toolbox", "TLCExt"}
COUNTS = re.compile(r"^(\d+) states generated, (\d+) distinct states found", re.M)
OUTPUT_LIMIT = 64 << 20


def finished(process, seconds):
    try:
        process.wait(seconds)
        return True
    except subprocess.TimeoutExpired:
        return False


def run(command, directory, timeout, memory):
    """The seconds command takes and its output, or why it was killed."""
    start = time.perf_counter()
    process = subprocess.Popen(command, cwd=directory, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                               start_new_session=True)
    chunks, size = [], [0]

    def pump():
        for chunk in iter(lambda: process.stdout.read(1 << 16), b""):
            size[0] += len(chunk)
            if size[0] <= OUTPUT_LIMIT:
                chunks.append(chunk)
    reader = threading.Thread(target=pump)
    reader.start()
    killed = None
    while not finished(process, 0.05):
        rss = subprocess.run(["ps", "-o", "rss=", "-g", str(process.pid)], capture_output=True, text=True).stdout
        if sum(map(int, rss.split())) > memory * 1024:
            killed = f"over {memory} MB"
        elif time.perf_counter() - start > timeout:
            killed = f"over {timeout} s"
        elif size[0] > OUTPUT_LIMIT:
            killed = f"over {OUTPUT_LIMIT >> 20} MB of output"
        if killed:
            os.killpg(process.pid, signal.SIGKILL)
            break
    process.wait()
    reader.join()
    seconds = time.perf_counter() - start
    if killed:
        return seconds, None, killed
    output = spec_runner.normalize(b"".join(chunks).decode(errors="replace"), directory)
    return seconds, output + f"exit {process.returncode}\n", None


def stage(spec, libraries, directory):
    shutil.copytree(spec, directory)
    for library in libraries:
        for module in Path(library).glob("*.tla"):
            if module.stem not in BUILT_IN and not (directory / module.name).exists():
                shutil.copy(module, directory)


def models(examples, only, max_runtime):
    for manifest in sorted(examples.glob("specifications/*/manifest.json")):
        for module in json.loads(manifest.read_text())["modules"]:
            for model in module["models"]:
                if (model["mode"] == "exhaustive search" and model["runtime"] != "unknown"
                        and model["runtime"] <= max_runtime and only in model["path"]):
                    yield module, model


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("engine")
    parser.add_argument("--tlc", required=True)
    parser.add_argument("--examples", required=True, type=Path)
    parser.add_argument("--library", action="append", default=[])
    parser.add_argument("--only", default="")
    parser.add_argument("--max-runtime", default="00:10:00")
    parser.add_argument("--timeout", type=float, default=900)
    parser.add_argument("--memory", type=int, default=4000)
    args = parser.parse_args()
    engine = str(Path(args.engine).resolve())
    examples = args.examples.resolve()
    key = hashlib.sha1(f"{args.tlc} {examples} {args.library}".encode()).hexdigest()[:12]
    cache_path = examples.with_name(f"fullbench-tlc-{key}.json")
    cache = json.loads(cache_path.read_text()) if cache_path.exists() else {}
    diffs = open(examples.with_name("fullbench.diff"), "w")
    rows, tallies, speedups = [], {}, []
    for module, model in models(examples, args.only, args.max_runtime):
        path = model["path"]
        name = path.removeprefix("specifications/").removesuffix(".cfg")
        command = ["-config", Path(path).name, Path(module["path"]).stem]
        with tempfile.TemporaryDirectory() as tmp:
            directory = Path(tmp).resolve() / "spec"
            stage(examples / Path(path).parent, args.library, directory)
            if path not in cache:
                tlc = args.tlc.split()
                if "action composition" in module["features"]:
                    tlc.insert(tlc.index("tlc2.TLC"), "-Dtlc2.tool.impl.Tool.cdot=true")
                tlc += ["-workers", "1", "-metadir", str(Path(tmp) / "meta")]
                seconds, output, killed = run(tlc + command, directory, args.timeout, args.memory)
                cache[path] = {"seconds": seconds, "output": output, "killed": killed}
                cache_path.write_text(json.dumps(cache, indent=1))
            theirs = cache[path]
            ours_seconds, ours, killed = run([engine] + command, directory, args.timeout, args.memory)
        if theirs["killed"]:
            verdict = f"TLC {theirs['killed']}"
        elif killed:
            verdict = f"tpp {killed}"
        elif ours != theirs["output"]:
            verdict = "differs"
            diffs.write(f"=== {name}\n" + "".join(difflib.unified_diff(
                theirs["output"].splitlines(True), ours.splitlines(True), "TLC", "tpp", n=2)))
            diffs.flush()
        else:
            verdict = "same"
            speedups.append(theirs["seconds"] / ours_seconds)
        tallies[verdict.split(" over")[0]] = tallies.get(verdict.split(" over")[0], 0) + 1
        counts = COUNTS.search(theirs["output"] or "")
        distinct = counts.group(2) if counts else ""
        tpp_time = "" if killed else f"{ours_seconds:.2f}"
        speedup = f"{theirs['seconds'] / ours_seconds:.1f}x" if verdict == "same" else ""
        rows.append(f"| {name} | {distinct} | {theirs['seconds']:.2f} | {tpp_time} | {speedup} | {verdict} |")
        print(rows[-1], flush=True)
    mean = math.exp(sum(map(math.log, speedups)) / len(speedups)) if speedups else 0
    summary = ", ".join(f"{n} {verdict}" for verdict, n in sorted(tallies.items(), key=lambda t: -t[1]))
    report = ROOT / "docs/fullbench.md"
    head = report.read_text().split("<!-- results -->")[0] if report.exists() else "# Full benchmark\n\n"
    report.write_text(head + "<!-- results -->\n\n"
                      f"tlaplus/Examples at {examples.name.removeprefix('Examples-')[:12]}, the exhaustive-search "
                      f"models recorded to run in at most {args.max_runtime}. Measured on {platform.machine()} "
                      f"{platform.system()} {platform.release()}, one run, wall-clock seconds, one worker.\n\n"
                      f"{len(rows)} models: {summary}. Where tpp prints what TLC prints, "
                      f"it is {mean:.1f}x as fast (geometric mean).\n\n"
                      "| model | distinct states | TLC | tpp | speedup | result |\n"
                      "|---|---:|---:|---:|---:|---|\n" + "\n".join(rows) + "\n")
    return 0 if tallies.get("same", 0) + tallies.get("TLC", 0) == len(rows) else 1


if __name__ == "__main__":
    sys.exit(main())
