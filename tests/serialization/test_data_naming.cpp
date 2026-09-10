#include <cstddef>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <map>
#include <optional>
#include <vector>
#include <variant>
#include <catch2/catch_test_macros.hpp>
#include <glaze/glaze.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "game/game_data.hpp"
#include "game/level_data.hpp"
#include "serialization/data_shapes.hpp"

namespace
{
    template <class T>
    concept MapLike = requires {
        typename T::key_type;
        typename T::mapped_type;
    };

    template <class T> constexpr bool aLeaf()
    {
        return std::is_arithmetic_v<T> || std::is_enum_v<T> || std::is_same_v<T, std::string> ||
               glz::name_v<T>.starts_with("glm::");
    }

    template <class T> constexpr bool ours()
    {
        return !glz::name_v<T>.starts_with("std::");
    }

    template <class T> constexpr std::string_view notNamedAsDataIn();

    template <class Variant, std::size_t... I>
    constexpr std::string_view notNamedAsDataInAny(std::index_sequence<I...>)
    {
        std::string_view found;
        ((found =
              found.empty() ? notNamedAsDataIn<std::variant_alternative_t<I, Variant>>() : found),
         ...);
        return found;
    }

    template <class T, std::size_t... I>
    constexpr std::string_view notNamedAsDataInFields(std::index_sequence<I...>)
    {
        std::string_view found;
        ((found = found.empty()
                      ? notNamedAsDataIn<
                            std::remove_cvref_t<typename glz::reflect<T>::template type<I>>>()
                      : found),
         ...);
        return found;
    }

    template <class T> constexpr std::string_view notNamedAsDataIn()
    {
        if constexpr (aLeaf<T>())
            return {};
        else if constexpr (shapes::IsOptional<T>::value || shapes::IsVector<T>::value)
            return notNamedAsDataIn<typename T::value_type>();
        else if constexpr (shapes::IsVariant<T>::value)
            return notNamedAsDataInAny<T>(std::make_index_sequence<std::variant_size_v<T>>{});
        else if constexpr (MapLike<T>)
        {
            if (ours<T>() && !glz::name_v<T>.ends_with("Data"))
                return glz::name_v<T>;

            return notNamedAsDataIn<typename T::mapped_type>();
        }
        else if constexpr (glz::reflectable<T>)
        {
            if (!glz::name_v<T>.ends_with("Data"))
                return glz::name_v<T>;

            return notNamedAsDataInFields<T>(std::make_index_sequence<glz::reflect<T>::size>{});
        }
        else if constexpr (ours<T>() && !glz::name_v<T>.ends_with("Data"))
            return glz::name_v<T>;
        else
            return {};
    }
}

TEST_CASE("Every type that reaches a data file is named as data", "[DataNaming]")
{
    constexpr std::string_view InTheGame = notNamedAsDataIn<GameData>();
    INFO("\"" << InTheGame << "\" reaches a data file and is not named as data");
    REQUIRE(InTheGame.empty());

    constexpr std::string_view InALevel = notNamedAsDataIn<LevelData>();
    INFO("\"" << InALevel << "\" reaches a level file and is not named as data");
    REQUIRE(InALevel.empty());
}

namespace naming
{
    struct Plainly
    {
        int count = 0;
    };

    struct WrapperData
    {
        std::vector<std::optional<Plainly>> held;
    };

    struct HoldingData
    {
        std::variant<int, WrapperData> either;
    };

    struct Named : std::map<std::string, int>
    {
    };

    struct KeepingData
    {
        std::map<std::string, Named> named;
    };

    struct FineData
    {
        std::optional<std::map<int, std::vector<glm::vec2>>> shapes;
        std::string name;
        float size = 0.0f;
    };
}

TEST_CASE("The walk finds a badly named type however deep it hides", "[DataNaming]")
{
    STATIC_REQUIRE(notNamedAsDataIn<naming::HoldingData>() == "naming::Plainly");
    STATIC_REQUIRE(notNamedAsDataIn<naming::KeepingData>() == "naming::Named");
    STATIC_REQUIRE(notNamedAsDataIn<naming::FineData>().empty());
    STATIC_REQUIRE(notNamedAsDataIn<naming::Plainly>() == "naming::Plainly");
}
