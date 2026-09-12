#include <algorithm>
#include <optional>
#include <stdexcept>
#include "combat/health.hpp"
#include "combat/health_data.hpp"
#include "combat/hit.hpp"

Health::Health(const HealthData &data) : data(data), left(data.maximum)
{
    if (data.maximum < 1)
        throw std::runtime_error("Health needs a maximum above 0");

    if (data.invulnerableFor < 0.0f)
        throw std::runtime_error("Health needs an invulnerable window of 0 or longer");
}

bool Health::takeHit(const Hit &hit)
{
    if (!alive())
        return false;

    if (hit.lethal)
    {
        left = 0;
        taken = hit;
        return true;
    }

    if (invulnerable())
        return false;

    left = std::max(0, left - hit.damage);
    invulnerableLeft = data.invulnerableFor;
    taken = hit;
    return true;
}

void Health::update(float deltaTime)
{
    invulnerableLeft = std::max(0.0f, invulnerableLeft - deltaTime);
}

int Health::points() const
{
    return left;
}

int Health::maximum() const
{
    return data.maximum;
}

bool Health::alive() const
{
    return left > 0;
}

bool Health::invulnerable() const
{
    return invulnerableLeft > 0.0f;
}

const std::optional<Hit> &Health::lastHit() const
{
    return taken;
}
