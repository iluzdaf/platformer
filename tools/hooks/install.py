#!/usr/bin/env python3
"""Links this repository's hooks into .git/hooks, leaving the LFS hooks alone."""

import os
import shutil
import subprocess
import sys
from pathlib import Path


def main():
    root = Path(
        subprocess.run(
            ["git", "rev-parse", "--show-toplevel"],
            capture_output=True, text=True, check=True,
        ).stdout.strip()
    )

    source = root / "tools" / "hooks"
    destination = root / ".git" / "hooks"
    destination.mkdir(parents=True, exist_ok=True)

    for hook in sorted(source.iterdir()):
        if hook.name == Path(__file__).name:
            continue

        link = destination / hook.name
        if link.exists() or link.is_symlink():
            link.unlink()

        try:
            link.symlink_to(Path("..") / ".." / "tools" / "hooks" / hook.name)
        except OSError:
            # Windows without developer mode cannot symlink.
            shutil.copyfile(hook, link)

        os.chmod(link, 0o755)
        print(f"installed {hook.name}")

    return 0


if __name__ == "__main__":
    sys.exit(main())
