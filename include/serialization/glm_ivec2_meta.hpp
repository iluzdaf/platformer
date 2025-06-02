#pragma once

#include <glaze/glaze.hpp>

template <>
struct glz::meta<glm::ivec2>
{
    using T = glm::ivec2;
    static constexpr auto value = glz::array(&T::x, &T::y);
};
