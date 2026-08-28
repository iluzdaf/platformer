#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include <glaze/glaze.hpp>

template <> struct glz::meta<glm::ivec2>
{
    using T = glm::ivec2;
    // NOLINTNEXTLINE(readability-identifier-naming) glaze requires this name
    static constexpr auto value = glz::array(&T::x, &T::y);
};
