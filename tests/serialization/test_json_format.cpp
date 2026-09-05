#include <catch2/catch_test_macros.hpp>
#include <string>
#include "serialization/json_format.hpp"

TEST_CASE("A quote escaped inside a string does not end it", "[JsonFormat]")
{
    std::string json = R"({"name":"a\"b","rows":[[1,2],[3,4]]})";

    REQUIRE(withStructureOnLines(json) == R"({
    "name":"a\"b",
    "rows":[
        [1,2],
        [3,4]
    ]
})");
}

TEST_CASE("A backslash escaped inside a string is left as it is", "[JsonFormat]")
{
    std::string json = R"({"path":"c:\\dir","list":[1]})";

    REQUIRE(withStructureOnLines(json) == R"({
    "path":"c:\\dir",
    "list":[1]
})");
}

TEST_CASE("A container never closed is laid out to where it ends", "[JsonFormat]")
{
    REQUIRE(withStructureOnLines("[[1,2") == "[\n    [1,2");
}

TEST_CASE("A grid is padded so every cell is as wide as the widest", "[JsonFormat]")
{
    REQUIRE(
        withPaddedGrid(R"({"indices":[[0,100],[7,0]],"tilePalette":"cave"})") ==
        R"({"indices":[[  0,100],[  7,  0]],"tilePalette":"cave"})");
}

TEST_CASE("Padding stops at the grid and leaves the rest alone", "[JsonFormat]")
{
    REQUIRE(
        withPaddedGrid(R"({"indices":[[0,10]],"playerStart":[8,128],"npcs":[[1,2]]})") ==
        R"({"indices":[[ 0,10]],"playerStart":[8,128],"npcs":[[1,2]]})");
}

TEST_CASE("Text with no grid passes through untouched", "[JsonFormat]")
{
    const std::string settings = R"({"debug":true,"scoreIcon":{"frame":0}})";

    REQUIRE(withPaddedGrid(settings) == settings);
}
