"""Name the functions this project defines that neither binary keeps.

The compiler already refuses an unused function with internal linkage, which is
how -Werror catches a helper nobody calls. It cannot say the same about anything
externally visible, because a translation unit has no way to know whether another
one calls it. Only the whole program knows, and after dead stripping the linker
has an opinion: a symbol our objects define that survives in neither the game nor
the tests is reached from nothing.

    cmake -B build/dead -G Ninja -DDEAD_CODE=ON -DCMAKE_BUILD_TYPE=Debug ...
    cmake --build build/dead
    python3 tools/dead_code.py build/dead

Alive in either binary counts. A function only the tests call is tested rather
than dead, and one only the game calls is shipped rather than untested.

This reports and does not fail, because the linker cannot tell an unused function
from an inlined one. Both leave no symbol behind. Actor::Actor is called by two
constructors and CameraShake::CameraShake by a member of Camera2D, and both are
listed here anyway. Read the list, do not trust it: it says nothing survived,
which is a fact about symbols and only sometimes a fact about the code.

It is blind to virtuals as well. A vtable keeps every override alive whether or
not anything calls it, so an unused one never appears.
"""

import argparse
import re
import shutil
import subprocess
import sys
from pathlib import Path

# Names that belong to somebody else, or that the linker discards for its own
# reasons rather than because nothing wants them. The profile runtime appears
# when coverage is on in the same build, which is how CI asks both questions at
# once.
NOT_OURS = re.compile(
    r"^_?GLOBAL__|^ltmp|llvm_profile|"
    r"\b(std|__cxx|__gnu|glm|ImGui|ImVec|sol|glz|Catch|efsw|lua|stbi)\b|"
    r"operator (new|delete)")


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


def run(command):
    finished = subprocess.run(command, capture_output=True, text=True)
    if finished.returncode != 0:
        raise SystemExit(f"{command[0]} failed:\n{finished.stderr.strip()}")

    return finished.stdout


def symbols(nm, paths):
    named = set()
    for line in run([nm, "--defined-only", *[str(p) for p in paths]]).splitlines():
        parts = line.split()
        if len(parts) >= 3 and parts[1] in ("T", "t"):
            named.add(parts[2])

    return named


def unreached(buildDirectory):
    nm = llvmTool("llvm-nm")
    build = Path(buildDirectory)

    objects = sorted(build.glob("CMakeFiles/platformer_lib.dir/**/*.o"))
    if not objects:
        raise SystemExit(f"no platformer_lib objects under {build}: build it first")

    binaries = [build / name for name in ("platformer", "tests")]
    missing = [str(b) for b in binaries if not b.exists()]
    if missing:
        raise SystemExit(f"{', '.join(missing)} not found: build them first")

    defined = symbols(nm, objects)
    alive = symbols(nm, binaries)

    return sorted(defined - alive)


def readable(names):
    demangled = run([llvmTool("llvm-cxxfilt"), *names]).splitlines() if names else []

    return sorted(name for name in demangled if not NOT_OURS.search(name))


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("build", help="a build directory configured with -DDEAD_CODE=ON")
    arguments = parser.parse_args()

    names = readable(unreached(arguments.build))
    if not names:
        print("every function this project defines is reached from the game or the tests")
        return 0

    print(f"{len(names)} functions survive in neither binary, so nothing the linker")
    print("can see calls them. An inlined function looks the same, so read before deleting.\n")
    for name in names:
        print(f"  {name}")

    return 0


if __name__ == "__main__":
    sys.exit(main())
