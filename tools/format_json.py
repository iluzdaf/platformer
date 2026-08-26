"""Write json assets in the format the game itself writes.

Structure goes on its own lines and leaves stay compact, with a tile grid
aligned into columns. TileMap::save produces the same bytes, so a level saved
from the in game editor and one written here are identical. The pre commit
hook runs --format over staged assets.

    python3 tools/format_json.py --format assets/game_data.json
    python3 tools/format_json.py --check

With no paths it applies to every json under assets. Note that no editor can
produce this format, so json formatting is off in .vscode/settings.json.

Editor and CMake config are left out: they are JSONC, carrying comments that
strict json formatting would destroy.
"""

import argparse
import json
import sys
from pathlib import Path

ASSETS_DIR = Path(__file__).resolve().parent.parent / "assets"
NESTING_ON_LINES = 2
INLINE_WIDTH_LIMIT = 100


def grid_of(data):
    if not isinstance(data, dict):
        return None

    if isinstance(data.get("indices"), list):
        return data["indices"]

    for value in data.values():
        found = grid_of(value)
        if found is not None:
            return found

    return None


def with_padded_grid(compact, grid):
    key = '"indices":['
    start = compact.find(key)
    if start == -1:
        return compact

    cursor = start + len(key)
    while cursor < len(compact) and compact[cursor] == "[":
        cursor = compact.index("]", cursor) + 1
        if cursor < len(compact) and compact[cursor] == ",":
            cursor += 1

    width = max(len(str(cell)) for row in grid for cell in row)
    rows = ["[" + ",".join(str(cell).rjust(width) for cell in row) + "]" for row in grid]

    return compact[:start + len(key)] + ",".join(rows) + compact[cursor:]


def span_of(text, opening):
    """Returns (holds a container, length of this container in the compact text)."""
    holds_container = False
    depth = 0
    in_string = False
    escaped = False

    for at in range(opening, len(text)):
        character = text[at]

        if escaped:
            escaped = False
        elif in_string:
            if character == "\\":
                escaped = True
            elif character == '"':
                in_string = False
        elif character == '"':
            in_string = True
        elif character in "{[":
            depth += 1
            if depth > 1:
                holds_container = True
        elif character in "}]":
            depth -= 1
            if depth == 0:
                return holds_container, at - opening + 1

    return holds_container, 0


def with_structure_on_lines(text):
    """Newlines for containers that hold containers, near the top of the tree."""
    out = []
    expanded = []
    depth = 0
    in_string = False
    escaped = False

    at = 0
    while at < len(text):
        character = text[at]

        if escaped:
            out.append(character)
            escaped = False
        elif in_string:
            out.append(character)
            if character == "\\":
                escaped = True
            elif character == '"':
                in_string = False
        elif character == '"':
            out.append(character)
            in_string = True
        elif character in "{[":
            closing = "}" if character == "{" else "]"
            if at + 1 < len(text) and text[at + 1] == closing:
                out.append(character + closing)
                at += 2
                continue

            written = "".join(out)
            line_start = written.rfind("\n")
            column = len(written) - line_start - 1 if line_start != -1 else len(written)

            holds, length = span_of(text, at)
            near_top = holds and depth + 1 <= NESTING_ON_LINES
            too_long = column + length > INLINE_WIDTH_LIMIT
            scalar_array = character == "[" and not holds

            on_lines = not scalar_array and (near_top or too_long)
            out.append(character)
            depth += 1
            expanded.append(on_lines)

            if on_lines:
                out.append("\n" + " " * (4 * depth))
        elif character in "}]":
            on_lines = bool(expanded) and expanded[-1]
            if expanded:
                expanded.pop()

            depth -= 1
            if on_lines:
                out.append("\n" + " " * (4 * depth))

            out.append(character)
        elif character == ",":
            out.append(character)
            if expanded and expanded[-1]:
                out.append("\n" + " " * (4 * depth))
        else:
            out.append(character)

        at += 1

    return "".join(out)


def formatted(data):
    """The storage format: structure on its own lines, leaves kept compact.

    Matches what TileMap::save writes, so a level saved from the in game
    editor and a level written here are byte identical.
    """
    compact = json.dumps(data, separators=(",", ":"))

    grid = grid_of(data)
    if isinstance(grid, list) and grid:
        compact = with_padded_grid(compact, grid)

    return with_structure_on_lines(compact)


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    action = parser.add_mutually_exclusive_group(required=True)
    action.add_argument("--format", action="store_true", help="write the format the game writes")
    action.add_argument("--check", action="store_true", help="fail if anything is not in that format")
    parser.add_argument("paths", nargs="*", type=Path, help="json files, defaults to every json under assets")
    args = parser.parse_args()

    paths = args.paths or sorted(ASSETS_DIR.rglob("*.json"))
    if not paths:
        print(f"no json found under {ASSETS_DIR}", file=sys.stderr)
        return 1

    unformatted = []
    for path in paths:
        original = path.read_text()
        try:
            data = json.loads(original)
        except json.JSONDecodeError as error:
            if "//" in original or "/*" in original:
                print(f"{path.name}: looks like JSONC, formatting it would drop its comments", file=sys.stderr)
            else:
                print(f"{path.name}: invalid json, {error}", file=sys.stderr)
            return 1

        if args.check:
            if original != formatted(data):
                unformatted.append(str(path))
            continue

        wanted = formatted(data)

        if json.loads(wanted) != data:
            print(f"{path.name}: would change the data, refusing", file=sys.stderr)
            return 1

        if original == wanted:
            continue

        path.write_text(wanted)
        print(f"{path.name}: {len(original.splitlines())} -> {len(wanted.splitlines())} lines")

    if unformatted:
        print("not in the game's format: " + ", ".join(unformatted), file=sys.stderr)
        print("run: python3 tools/format_json.py --format", file=sys.stderr)
        return 1

    return 0


if __name__ == "__main__":
    sys.exit(main())
