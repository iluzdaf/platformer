local function distance(from, to)
    return math.sqrt((to.x - from.x) ^ 2 + (to.y - from.y) ^ 2)
end

local function refugeFrom(facts, walker)
    local from = walker:currentNode()
    if from == nil or facts.threatFeet == nil then
        return nil
    end

    local away = facts.feet.x < facts.threatFeet.x and -1 or 1
    return walker:furthestRefugeFrom(from, facts.threatFeet, away)
end

local function fleeingTowardsTheThreat(facts, walker)
    local refuge = walker:targetNode()
    if refuge == nil or facts.threatFeet == nil or not facts.onGround then
        return false
    end

    return distance(walker:feetOf(refuge), facts.threatFeet) < distance(facts.feet, facts.threatFeet)
end

return {
    onHurt = function(rat)
        camera:startShake(0.06, 0.6)
    end,

    onDeath = function(rat)
        camera:startShake(0.12, 1)
    end,

    states = {
        patrol = include('scripts/steering/patrol.lua'),

        flee = {
            decide = function(self, rat, facts, walker, dt)
                if walker:finished() or fleeingTowardsTheThreat(facts, walker) then
                    local refuge = refugeFrom(facts, walker)
                    if refuge ~= nil then
                        walker:routeTo(refuge)
                    end
                end

                return walker:follow(dt)
            end,
        },
    },
}
