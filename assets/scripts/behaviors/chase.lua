local function distance(from, to)
    return math.sqrt((to.x - from.x) ^ 2 + (to.y - from.y) ^ 2)
end

local function caughtUp(facts, walker)
    return walker:standsAt(facts.threatFeet, facts:tuning('standoff') or 0)
end

local function threatHasMoved(self, facts, walker)
    return self.lastSeenAt == nil or distance(facts.threatFeet, self.lastSeenAt) > walker:arrivesWithin()
end

local function whereToCloseIn(facts, walker)
    local from = walker:currentNode()
    if from == nil then
        return nil
    end

    local reachable = walker:roundTripFrom(from)
    local place = walker:placeOnThePath(facts.threatFeet)
    if place ~= nil then
        local beyond = walker:endOfThePathBeyond(place, facts.feet)
        for _, node in ipairs(reachable) do
            if node == beyond then
                return beyond
            end
        end
    end

    local nearest, nearestDistance = nil, 0
    for _, node in ipairs(reachable) do
        local away = distance(walker:feetOf(node), facts.threatFeet)
        if nearest == nil or away < nearestDistance then
            nearest, nearestDistance = node, away
        end
    end

    return nearest
end

return {
    decide = function(self, npc, facts, walker, dt)
        if facts.threatFeet == nil or caughtUp(facts, walker) then
            return nil
        end

        if walker:finished() or (facts.onGround and threatHasMoved(self, facts, walker)) then
            self.lastSeenAt = facts.threatFeet
            local quarry = whereToCloseIn(facts, walker)
            if quarry ~= nil then
                walker:routeTo(quarry, facts.threatFeet)
            end
        end

        return walker:follow(dt)
    end,
}
