#pragma once

#include <glaze/glaze.hpp>

template <>
struct glz::meta<glm::vec2>
{
    using T = glm::vec2;
    // NOLINTNEXTLINE(readability-identifier-naming) glaze requires this name
    static constexpr auto value = glz::array(&T::x, &T::y);
};
