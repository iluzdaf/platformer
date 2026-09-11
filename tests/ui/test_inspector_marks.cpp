#include <catch2/catch_test_macros.hpp>
#include <cstddef>
#include <functional>
#include <map>
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
#include "npc/npc_data.hpp"
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

    struct Fold
    {
        std::string name;
        std::function<std::string()> asJsonNow;
    };

    struct Asking
    {
        std::vector<std::string> quiet;
        std::size_t nudged = 0;
    };

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
            ++asking.nudged;
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
        else if constexpr (inspector::IsMap<T>::value)
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

    template <class T> Asking nudgingEveryFieldOf(T &value)
    {
        SavedInScope was(differs::compact(value));
        std::vector<Fold> folds;
        Asking asking;
        nudgingEachField(value, folds, asking);

        return asking;
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

    Asking asking = nudgingEveryFieldOf(gameData);

    INFO("fields nudged: " << asking.nudged);
    for (const std::string &path : asking.quiet)
        UNSCOPED_INFO("said nothing: " << path);

    REQUIRE(asking.nudged > 50);
    REQUIRE(asking.quiet.empty());
}

TEST_CASE("Every field of a level says when it is edited", "[InspectorMarks]")
{
    LevelData levelData = aFloorLevelPlacing({aVillagerAt(glm::ivec2(2, FloorLevelStanding))});

    Asking asking = nudgingEveryFieldOf(levelData);

    INFO("fields nudged: " << asking.nudged);
    for (const std::string &path : asking.quiet)
        UNSCOPED_INFO("said nothing: " << path);

    REQUIRE(asking.nudged > 10);
    REQUIRE(asking.quiet.empty());
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
