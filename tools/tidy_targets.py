"""List the .cpp files clang-tidy needs to look at for a given change.

A header change is not confined to that header: every translation unit that
pulls it in, however deeply, can break on it. So this walks the include graph
backwards from whatever changed and prints the .cpp files that reach it.

    python3 tools/tidy_targets.py --since origin/master
    python3 tools/tidy_targets.py

With no --since it prints every .cpp. So does a change to the rules themselves,
since those judge every translation unit and none can be assumed to have been read
under the new ones. Registering a source file in CMakeLists does not count, being
a list rather than a rule. Only quoted includes are followed, so system and
external headers are ignored.
"""

import argparse
import re
import subprocess
import sys
from pathlib import Path

ROOTS = ("src", "include", "tests")

# Change any of these and every translation unit is judged by different rules,
# so none of them can be trusted to have been checked under the new ones.
RULES = (".clang-tidy", ".llvm-version", "CMakeLists.txt", "tools/tidy_targets.py")

# CMakeLists is on that list for the flags it sets, not for the file lists it
# keeps. Registering a new source changes how nothing else is compiled, and
# every branch that adds a file would otherwise sweep the tree.
SOURCE_LINE = re.compile(r"^[+-]\s*(src|tests|include)/\S+\.(cpp|hpp|c|h)\s*$")

INCLUDE = re.compile(r'^\s*#\s*include\s*"([^"]+)"', re.MULTILINE)


def sources():
    return [path for root in ROOTS for path in Path(root).rglob("*") if path.suffix in (".cpp", ".hpp")]


def includedBy(paths):
    """Map each file to the files that include it, resolved against the roots."""
    resolvable = {}
    for path in paths:
        resolvable.setdefault(path.name, []).append(path)
        for root in ROOTS:
            try:
                resolvable.setdefault(str(path.relative_to(root)), []).append(path)
            except ValueError:
                pass

    users = {}
    for path in paths:
        for quoted in INCLUDE.findall(path.read_text(encoding="utf-8", errors="ignore")):
            for target in resolvable.get(quoted, []):
                users.setdefault(target, set()).add(path)

    return users


def reaching(changed, users):
    """Every file that includes anything in `changed`, transitively."""
    found = set(changed)
    pending = list(changed)
    while pending:
        for user in users.get(pending.pop(), ()):
            if user not in found:
                found.add(user)
                pending.append(user)

    return found


def branchedFrom(ref):
    merged = subprocess.run(["git", "merge-base", ref, "HEAD"], capture_output=True, text=True)

    return merged.stdout.strip() if merged.returncode == 0 else ref


def changedSince(against):
    listed = subprocess.run(
        ["git", "diff", "--name-only", against, "--"] + list(ROOTS) + list(RULES),
        capture_output=True, text=True, check=True)

    # A file nobody has added yet is not in any diff, and locally that is exactly
    # what a new file is. On CI everything is committed and this finds nothing.
    untracked = subprocess.run(
        ["git", "ls-files", "--others", "--exclude-standard", "--"] + list(ROOTS),
        capture_output=True, text=True, check=True)

    return [Path(name) for name in listed.stdout.split() + untracked.stdout.split()]


def rulesChanged(changed, against):
    touched = sorted(str(path) for path in changed if str(path) in RULES)
    if not touched:
        return False

    if touched != ["CMakeLists.txt"]:
        return True

    written = subprocess.run(
        ["git", "diff", "-U0", against, "--", "CMakeLists.txt"],
        capture_output=True, text=True, check=True).stdout

    for line in written.splitlines():
        if line.startswith(("+++", "---")) or not line.startswith(("+", "-")):
            continue

        if not SOURCE_LINE.match(line):
            return True

    return False


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--since", help="git ref to compare against; omit to list everything")
    arguments = parser.parse_args()

    paths = sources()
    if not arguments.since:
        print("\n".join(str(path) for path in paths if path.suffix == ".cpp"))
        return 0

    against = branchedFrom(arguments.since)
    changed = changedSince(against)
    if rulesChanged(changed, against):
        print("\n".join(str(path) for path in paths if path.suffix == ".cpp"))
        return 0

    changed = [path for path in changed if path in set(paths)]
    if not changed:
        return 0

    affected = reaching(changed, includedBy(paths))
    print("\n".join(sorted(str(path) for path in affected if path.suffix == ".cpp")))
    return 0


if __name__ == "__main__":
    sys.exit(main())
