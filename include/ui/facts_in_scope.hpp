#pragma once

struct FactsData;

const FactsData *factsInScope();

class OfferingFacts
{
public:
    explicit OfferingFacts(const FactsData &facts);
    ~OfferingFacts();

    OfferingFacts(const OfferingFacts &) = delete;
    OfferingFacts &operator=(const OfferingFacts &) = delete;

private:
    const FactsData *before = nullptr;
};
