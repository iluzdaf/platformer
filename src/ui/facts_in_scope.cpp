#include "ui/facts_in_scope.hpp"

namespace
{
    const Facts *offered = nullptr;
}

const Facts *factsInScope()
{
    return offered;
}

OfferingFacts::OfferingFacts(const Facts &facts) : before(offered)
{
    offered = &facts;
}

OfferingFacts::~OfferingFacts()
{
    offered = before;
}
