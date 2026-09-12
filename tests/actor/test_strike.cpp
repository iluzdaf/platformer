#include <optional>
#include <catch2/catch_test_macros.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "actor/hit.hpp"
#include "actor/hurting.hpp"
#include "actor/strike.hpp"
#include "physics/aabb.hpp"

namespace
{
    constexpr glm::vec2 Size{10.0f, 10.0f};

    AABB aBoxAt(float left)
    {
        return AABB{glm::vec2(left, 0.0f), Size};
    }

    glm::vec2 feetUnder(const AABB &box)
    {
        return box.bottomCenter();
    }

    Hurting threatening(const AABB &box, int damage = 1, float direction = 0.0f)
    {
        return Hurting{box, damage, direction};
    }

    std::optional<Hit> hitOn(const AABB &target, const Hurting &hurting, glm::vec2 attackerFeet)
    {
        return hitFrom(hurting, attackerFeet, target, feetUnder(target));
    }
}

TEST_CASE("A threat that reaches the target hits it for the threat's damage", "[Strike]")
{
    AABB attacker = aBoxAt(0.0f);

    std::optional<Hit> hit = hitOn(aBoxAt(5.0f), threatening(attacker, 3), feetUnder(attacker));

    REQUIRE(hit.has_value());
    REQUIRE(hit->damage == 3);
    REQUIRE_FALSE(hit->lethal);
}

TEST_CASE("A threat that does not reach the target, or only touches it, misses", "[Strike]")
{
    AABB attacker = aBoxAt(0.0f);
    Hurting hurting = threatening(attacker);

    REQUIRE_FALSE(hitOn(aBoxAt(Size.x + 5.0f), hurting, feetUnder(attacker)));
    REQUIRE_FALSE(hitOn(aBoxAt(Size.x), hurting, feetUnder(attacker)));
}

TEST_CASE("A threat that has a way pushes that way, whichever side the target is on", "[Strike]")
{
    AABB attacker = aBoxAt(0.0f);
    Hurting pushingLeft = threatening(attacker, 1, -1.0f);

    std::optional<Hit> hit = hitOn(aBoxAt(5.0f), pushingLeft, feetUnder(attacker));

    REQUIRE(hit->direction == glm::vec2(-1.0f, 0.0f));
}

TEST_CASE("A threat with no way of its own pushes away from whoever threatens", "[Strike]")
{
    AABB attacker = aBoxAt(0.0f);
    Hurting biting = threatening(attacker);

    REQUIRE(hitOn(aBoxAt(5.0f), biting, feetUnder(attacker))->direction == glm::vec2(1.0f, 0.0f));
    REQUIRE(hitOn(aBoxAt(-5.0f), biting, feetUnder(attacker))->direction == glm::vec2(-1.0f, 0.0f));
}
