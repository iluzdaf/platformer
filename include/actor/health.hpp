#pragma once

#include "actor/health_data.hpp"

struct Hit;

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

private:
    HealthData data;
    int left;
    float invulnerableLeft = 0.0f;
};
