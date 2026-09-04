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
