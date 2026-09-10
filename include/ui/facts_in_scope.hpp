#pragma once

struct Facts;

const Facts *factsInScope();

class OfferingFacts
{
public:
    explicit OfferingFacts(const Facts &facts);
    ~OfferingFacts();

    OfferingFacts(const OfferingFacts &) = delete;
    OfferingFacts &operator=(const OfferingFacts &) = delete;

private:
    const Facts *before = nullptr;
};
