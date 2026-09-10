#include <string_view>
#include "ui/inspector_edited.hpp"
#include "ui/inspector_fields.hpp"
#include "ui/file_chooser.hpp"
#include "assets/asset_paths.hpp"
#include "assets/texture_path_data.hpp"
#include "game/level_path_data.hpp"
#include "scripting/script_path_data.hpp"

inspector::Edited drawCustomField(std::string_view name, TexturePathData &value)
{
    return drawFileChooser(name, value.path, assets::Textures, ".png");
}

inspector::Edited drawCustomField(std::string_view name, ScriptPathData &value)
{
    return drawFileChooser(name, value.path, assets::Scripts, ".lua");
}

inspector::Edited drawCustomField(std::string_view name, LevelPathData &value)
{
    return drawFileChooser(name, value.path, assets::Levels, ".json");
}
