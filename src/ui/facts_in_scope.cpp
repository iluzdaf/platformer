#include "ui/facts_in_scope.hpp"

namespace
{
    const FactsData *offered = nullptr;
}

const FactsData *factsInScope()
{
    return offered;
}

OfferingFacts::OfferingFacts(const FactsData &facts) : before(offered)
{
    offered = &facts;
}

OfferingFacts::~OfferingFacts()
{
    offered = before;
}
