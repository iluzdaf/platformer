local function endOfTheBeat(facts, walker, from, second)
    local beat = facts.beat
    if beat ~= nil then
        local place = walker:placeOnThePath(second and beat.to or beat.from)
        if place ~= nil then
            return place.feet, walker:endOfThePathBeyond(place, facts.feet)
        end
    end

    local farthest = from
    for _, node in ipairs(walker:walkableFrom(from)) do
        local here, best = walker:feetOf(node).x, walker:feetOf(farthest).x
        if (second and here > best) or (not second and here < best) then
            farthest = node
        end
    end

    return walker:feetOf(farthest), farthest
end

return {
    enter = function(self)
        self.headingForTheSecond = false
    end,

    decide = function(self, npc, facts, walker, dt)
        local from = walker:currentNode()
        if from ~= nil and walker:finished() then
            local stop, via = endOfTheBeat(facts, walker, from, self.headingForTheSecond)
            if walker:standsAt(stop) then
                self.headingForTheSecond = not self.headingForTheSecond
                stop, via = endOfTheBeat(facts, walker, from, self.headingForTheSecond)
            end

            walker:routeTo(via, stop)
        end

        return walker:follow(dt)
    end,
}
