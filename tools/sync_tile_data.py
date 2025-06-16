import json
import os
from pathlib import Path

base_path = Path(__file__).resolve().parent
tile_map_dir = base_path.parent / "assets" / "tile_maps"

with open(tile_map_dir / "test.json") as f:
    test_data = json.load(f)
    tile_data_to_copy = test_data["tileData"]

for file_path in tile_map_dir.glob("*.json"):
    if file_path.name == "test.json":
        continue

    with open(file_path) as f:
        level_data = json.load(f)

    level_data["tileData"] = tile_data_to_copy

    with open(file_path, "w") as f:
        json.dump(level_data, f, separators=(',', ':'))
    
    print(f"Updated tileData in {file_path.name}")