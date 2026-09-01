"""Say which of the project's own lines the tests never run.

A test suite that passes says nothing about the code it never enters. This runs
the suite under instrumentation and reports what was reached, grouped by subject
so that a gap reads as a place rather than a list of files.

    cmake -B build/coverage -G Ninja -DCOVERAGE=ON -DCMAKE_BUILD_TYPE=Debug ...
    cmake --build build/coverage
    python3 tools/coverage.py build/coverage

    python3 tools/coverage.py build/coverage --files
    python3 tools/coverage.py build/coverage --at-least 60

Only src and include count. External code, the tests themselves and the
standard library are somebody else's coverage, and counting them moves the
number without moving the truth. Headers with no executable lines are dropped
for the same reason: a struct cannot be untested.

--at-least fails when the total falls below a floor, for a build that should
notice going backwards. Without it this only ever reports.
"""

import argparse
import collections
import json
import os
import shutil
import subprocess
import sys
from pathlib import Path

OURS = ("src/", "include/")
NOT_OURS = "external/|/tests/|catch2|Cellar|/build/"


def llvmTool(name):
    found = shutil.which(name)
    if found:
        return found

    version = (Path(__file__).parent.parent / ".llvm-version").read_text().strip()
    for prefix in ("/opt/homebrew/opt/llvm/bin", f"/usr/lib/llvm-{version}/bin", "/usr/bin"):
        candidate = Path(prefix) / name
        if candidate.exists():
            return str(candidate)

    raise SystemExit(f"{name} not found: install llvm or put it on PATH")


def run(command, **kwargs):
    finished = subprocess.run(command, capture_output=True, text=True, **kwargs)
    if finished.returncode != 0:
        raise SystemExit(f"{command[0]} failed:\n{finished.stderr.strip()}")

    return finished.stdout


def measure(buildDirectory):
    tests = Path(buildDirectory) / "tests"
    if not tests.exists():
        raise SystemExit(f"{tests} not found: configure with -DCOVERAGE=ON and build first")

    raw = Path(buildDirectory) / "coverage.profraw"
    merged = Path(buildDirectory) / "coverage.profdata"

    subprocess.run([str(tests)], env={**os.environ, "LLVM_PROFILE_FILE": str(raw)},
                   capture_output=True, text=True)
    if not raw.exists():
        raise SystemExit("no profile was written: was the build configured with -DCOVERAGE=ON?")

    run([llvmTool("llvm-profdata"), "merge", "-sparse", str(raw), "-o", str(merged)])
    exported = run([
        llvmTool("llvm-cov"), "export", str(tests),
        f"-instr-profile={merged}", "-summary-only",
        f"-ignore-filename-regex={NOT_OURS}"])

    return json.loads(exported)


def repositoryRoot():
    return Path(run(["git", "rev-parse", "--show-toplevel"]).strip())


def ours(exported, buildDirectory):
    root = repositoryRoot()
    seen = 0

    for entry in exported["data"][0]["files"]:
        seen += 1
        # llvm-cov reports whatever path the compiler was given, which is absolute
        # from one build directory and relative to it from another.
        where = Path(entry["filename"])
        if not where.is_absolute():
            where = (Path(buildDirectory) / where).resolve()

        try:
            name = str(where.relative_to(root))
        except ValueError:
            continue

        lines = entry["summary"]["lines"]
        if name.startswith(OURS) and lines["count"] > 0:
            yield name, lines["covered"], lines["count"]

    if not seen:
        raise SystemExit("llvm-cov reported no files: was the build configured with -DCOVERAGE=ON?")


def report(measured, showFiles):
    if not measured:
        raise SystemExit(
            "no file under src or include carried coverage data, so there is nothing to report")

    areas = collections.defaultdict(lambda: [0, 0])
    untouched = collections.defaultdict(list)

    for name, covered, total in measured:
        area = name.split("/")[1]
        areas[area][0] += covered
        areas[area][1] += total
        if covered == 0:
            untouched[area].append((total, name))

    print(f"{'cover':>7}  {'lines':>6}  {'never run':>9}  area")
    for area, (covered, total) in sorted(areas.items(), key=lambda kv: kv[1][0] / kv[1][1]):
        missed = sum(lines for lines, _ in untouched[area])
        print(f"{100 * covered / total:6.1f}%  {total:6d}  {missed:9d}  {area}")

    if showFiles:
        print("\nfiles no test enters, most lines first")
        for area in sorted(untouched, key=lambda a: -sum(l for l, _ in untouched[a])):
            for lines, name in sorted(untouched[area], reverse=True):
                print(f"{lines:6d}  {name}")

    covered = sum(area[0] for area in areas.values())
    total = sum(area[1] for area in areas.values())

    return 100 * covered / total, total - covered


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("build", help="a build directory configured with -DCOVERAGE=ON")
    parser.add_argument("--files", action="store_true", help="also name the files no test enters")
    parser.add_argument("--at-least", type=float, help="fail below this percentage")
    arguments = parser.parse_args()

    measured = list(ours(measure(arguments.build), arguments.build))
    percent, missed = report(measured, arguments.files)
    print(f"\n{percent:.1f}% of src and include, {missed} lines never run")

    if arguments.at_least is not None and percent < arguments.at_least:
        print(f"below the floor of {arguments.at_least}%", file=sys.stderr)
        return 1

    return 0


if __name__ == "__main__":
    sys.exit(main())
