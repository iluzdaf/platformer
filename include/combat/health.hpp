#pragma once

#include <optional>
#include "combat/health_data.hpp"
#include "combat/hit.hpp"

class Health
{
public:
    explicit Health(const HealthData &data);
    bool takeHit(const Hit &hit);
    void update(float deltaTime);
    int points() const;
    int maximum() const;
    bool alive() const;
    bool invulnerable() const;
    const std::optional<Hit> &lastHit() const;

private:
    HealthData data;
    int left;
    float invulnerableLeft = 0.0f;
    std::optional<Hit> taken;
};
