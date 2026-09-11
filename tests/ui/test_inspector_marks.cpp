#include <catch2/catch_test_macros.hpp>
#include <algorithm>
#include <concepts>
#include <cstddef>
#include <stdexcept>
#include <functional>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <variant>
#include <vector>
#include <glaze/glaze.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "game/game_data.hpp"
#include "game/level_data.hpp"
#include "helpers/levels.hpp"
#include "helpers/npc_fixtures.hpp"
#include "assets/texture_path_data.hpp"
#include "conditions/asked.hpp"
#include "game/level_path_data.hpp"
#include "npc/npc_data.hpp"
#include "scripting/script_path_data.hpp"
#include "pickups/pickup_data.hpp"
#include "actor/actor_animation_data.hpp"
#include "animations/animator_data.hpp"
#include "animations/frame_animation_data.hpp"
#include "helpers/actors.hpp"
#include "helpers/palettes.hpp"
#include "serialization/only_what_differs.hpp"
#include "ui/data_inspector.hpp"
#include "ui/saved_in_scope.hpp"
#include "ui/types_ui.hpp"
#include "ui/type_shown.hpp"
#include "ui/editor_commands.hpp"
#include "rendering/texture_cache.hpp"
#include "helpers/headless_imgui.hpp"
#include <imgui.h>

namespace
{
    template <class T> bool nudge(T &value)
    {
        if constexpr (std::is_same_v<T, bool>)
            value = !value;
        else if constexpr (std::is_same_v<T, int>)
            value += 1;
        else if constexpr (std::is_same_v<T, float>)
            value += 1.0f;
        else if constexpr (std::is_same_v<T, std::string>)
            value += "x";
        else if constexpr (std::is_same_v<T, glm::vec2>)
            value.x += 1.0f;
        else if constexpr (std::is_same_v<T, glm::ivec2>)
            value.x += 1;
        else
            return false;

        return true;
    }

    template <class T> bool nudge(std::optional<T> &value)
    {
        return value && nudge(*value);
    }

    bool nudge(Asked &value)
    {
        return std::visit([](auto &held) { return nudge(held); }, value);
    }

    bool nudge(TexturePathData &value)
    {
        value.path += "x";
        return true;
    }

    bool nudge(ScriptPathData &value)
    {
        value.path += "x";
        return true;
    }

    bool nudge(LevelPathData &value)
    {
        value.path += "x";
        return true;
    }

    struct Fold
    {
        std::string name;
        std::function<std::string()> asJsonNow;
    };

    struct Asking
    {
        std::vector<std::string> quiet;
        std::vector<std::string> nudged;
    };

    template <class T>
    constexpr bool MapLike =
        inspector::IsMap<T>::value || std::derived_from<T, std::map<std::string, Asked>>;

    bool everyFoldSaysSo(const std::vector<Fold> &folds)
    {
        std::vector<std::string> at;
        for (std::size_t depth = 0; depth + 1 < folds.size(); ++depth)
        {
            at.push_back(folds[depth].name);
            if (!inspector::changedAt(at, folds[depth].asJsonNow()))
                return false;
        }

        return true;
    }

    template <class T> void nudgingEachField(T &value, std::vector<Fold> &folds, Asking &asking);

    template <class T>
    void nudgingUnder(std::string_view name, T &value, std::vector<Fold> &folds, Asking &asking)
    {
        inspector::InField here(name);
        folds.push_back(Fold{std::string(name), [&value] { return differs::compact(value); }});

        T asItWas = value;
        if (nudge(value))
        {
            asking.nudged.push_back(inspector::pathHere());
            if (!inspector::changedHere(value) || !everyFoldSaysSo(folds))
                asking.quiet.push_back(inspector::pathHere());

            value = asItWas;
            if (inspector::changedHere(value))
                asking.quiet.push_back(inspector::pathHere() + " (put back)");
        }

        nudgingEachField(value, folds, asking);
        folds.pop_back();
    }

    template <class T> void nudgingEachField(T &value, std::vector<Fold> &folds, Asking &asking)
    {
        if constexpr (inspector::IsOptional<T>::value)
        {
            if (value)
                nudgingEachField(*value, folds, asking);
        }
        else if constexpr (inspector::IsVector<T>::value)
        {
            for (std::size_t at = 0; at < value.size(); ++at)
                nudgingUnder(std::to_string(at), value[at], folds, asking);
        }
        else if constexpr (MapLike<T>)
        {
            for (auto &[key, held] : value)
                nudgingUnder(inspector::keyLabel(key), held, folds, asking);
        }
        else if constexpr (inspector::IsVariant<T>::value && glz::tagged<T>)
        {
            std::visit(
                [&folds, &asking](auto &held)
                {
                    inspector::forEachNamedField(
                        held,
                        [&folds, &asking](std::string_view fieldName, auto &field)
                        { nudgingUnder(fieldName, field, folds, asking); });
                },
                value);
        }
        else if constexpr (glz::reflectable<T>)
        {
            inspector::forEachNamedField(
                value,
                [&folds, &asking](std::string_view fieldName, auto &field)
                { nudgingUnder(fieldName, field, folds, asking); });
        }
    }

    void scalarsIn(const glz::json_t &json, const std::string &at, std::vector<std::string> &into)
    {
        if (json.is_object())
        {
            for (const auto &[key, held] : json.get_object())
                scalarsIn(held, at.empty() ? key : at + "." + key, into);
        }
        else if (json.is_array())
        {
            const glz::json_t::array_t &items = json.get_array();
            for (std::size_t index = 0; index < items.size(); ++index)
                scalarsIn(items[index], at + "." + std::to_string(index), into);
        }
        else if (!json.is_null())
            into.push_back(at);
    }

    std::vector<std::string> scalarsIn(const std::string &json)
    {
        glz::json_t read;
        if (glz::read_json(read, json))
            throw std::runtime_error("The saved copy is not json");

        std::vector<std::string> scalars;
        scalarsIn(read, "", scalars);
        return scalars;
    }

    bool startsWith(const std::string &path, const std::string &at)
    {
        return path == at || (path.starts_with(at) && path[at.size()] == '.');
    }

    struct Coverage
    {
        std::vector<std::string> neverNudged;
        std::vector<std::string> notInTheSavedCopy;
    };

    Coverage coveredBy(const std::string &savedJson, const std::vector<std::string> &nudged)
    {
        std::vector<std::string> scalars = scalarsIn(savedJson);

        Coverage coverage;
        for (const std::string &scalar : scalars)
            if (std::ranges::none_of(
                    nudged, [&scalar](const std::string &at) { return startsWith(scalar, at); }))
                coverage.neverNudged.push_back(scalar);

        for (const std::string &at : nudged)
            if (std::ranges::none_of(
                    scalars, [&at](const std::string &scalar) { return startsWith(scalar, at); }))
                coverage.notInTheSavedCopy.push_back(at);

        return coverage;
    }

    template <class T> void everyFieldOf(T &value)
    {
        std::string saved = differs::compact(value);

        SavedInScope was(saved);
        std::vector<Fold> folds;
        Asking asking;
        nudgingEachField(value, folds, asking);

        Coverage coverage = coveredBy(saved, asking.nudged);
        std::string report = "fields nudged: " + std::to_string(asking.nudged.size());
        for (const std::string &path : asking.quiet)
            report += "\nsaid nothing: " + path;
        for (const std::string &path : coverage.neverNudged)
            report += "\nnever nudged: " + path;
        for (const std::string &path : coverage.notInTheSavedCopy)
            report += "\nnudged but not in the saved copy: " + path;

        INFO(report);
        REQUIRE(asking.quiet.empty());
        REQUIRE(coverage.neverNudged.empty());
        REQUIRE(coverage.notInTheSavedCopy.empty());
    }

    ActorAnimationData someClipsAndARung()
    {
        ActorAnimationData animations;
        animations.clips["idle"] = FrameAnimationData{{0}, 0.2f};
        animations.clips["run"] = FrameAnimationData{{1, 2, 3}, 0.1f};

        AnimationTransitionData rung;
        rung.from = "idle";
        rung.to = "run";
        rung.when["moving"] = true;
        animations.ladder.transitions.push_back(rung);

        return animations;
    }

    GameData aGameWithSomethingOfEachKind()
    {
        GameData gameData;
        gameData.playerData = playerDataWithEveryAbility();
        gameData.playerData.actorData.animationData = someClipsAndARung();

        NpcData rat = setupNpcData();
        rat.actorData.animationData = someClipsAndARung();
        rat.facts["heard"] = false;
        rat.tuning["range"] = 200.0f;
        gameData.npcData = {{"rat", rat}};

        PickupData coin;
        coin.scoreDelta = 1;
        gameData.pickupData = {{"coin", coin}};
        gameData.tilePalettes = theOnlyPalette(aPaletteWithASolidTile());

        return gameData;
    }
}

TEST_CASE("Every field of the game data says when it is edited", "[InspectorMarks]")
{
    GameData gameData = aGameWithSomethingOfEachKind();

    everyFieldOf(gameData);
}

TEST_CASE("Every field of a level says when it is edited", "[InspectorMarks]")
{
    LevelData levelData = aFloorLevelPlacing({aVillagerAt(glm::ivec2(2, FloorLevelStanding))});

    everyFieldOf(levelData);
}

namespace
{
    std::vector<std::string> nothingReadsAsChangedWhile(
        HeadlessImGui &gui,
        const std::function<void()> &draw)
    {
        std::vector<ImGuiID> drawn;
        auto unfolding = [&drawn]
        {
            for (ImGuiID id : drawn)
                ImGui::GetStateStorage()->SetInt(id, 1);
        };

        for (int pass = 0; pass < 8; ++pass)
        {
            gui.frame(
                [&]
                {
                    unfolding();
                    draw();
                });
            drawn = gui.everythingDrawn();
        }

        std::vector<std::string> changed;
        gui.frame(
            [&]
            {
                unfolding();
                inspector::Watching watching;
                draw();
                changed = watching.saidChanged();
            });

        return changed;
    }
}

TEST_CASE("Nothing in the cast reads as changed until it is edited", "[InspectorMarks]")
{
    HeadlessImGui gui;
    GameData gameData = aGameWithSomethingOfEachKind();
    TextureCache textures;
    EditorCommands commands;

    auto nothingSaysItChanged = [&](const TypeShown &type)
    {
        TypesUi typesUi;
        REQUIRE_FALSE(typesUi.unsavedSince(gameData));
        typesUi.show(type);

        return nothingReadsAsChangedWhile(
            gui, [&] { typesUi.draw(gameData, textures, commands, nullptr); });
    };

    for (const TypeShown &type :
         {thePlayer(),
          TypeShown{TypeShown::What::Npc, "rat"},
          TypeShown{TypeShown::What::Pickup, "coin"}})
    {
        std::vector<std::string> changed = nothingSaysItChanged(type);
        INFO("looking at " << type.name);
        for (const std::string &path : changed)
            UNSCOPED_INFO("read as changed: " << path);

        REQUIRE(changed.empty());
    }
}
