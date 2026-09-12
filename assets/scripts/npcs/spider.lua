local function sense(spider)
    local threat = spider:threatFeet()
    spider:fact("threatOnMySurface", threat ~= nil and spider:onSameSurfaceAs(threat))
    spider:fact("threatClose", threat ~= nil and spider:distanceTo(threat) <= spider:tuning("close"))
    spider:fact("threatInReach", threat ~= nil and spider:distanceTo(threat) <= spider:tuning("reach"))
end

return {
    onTick = function(spider, dt)
        sense(spider)
    end,

    onHurt = function(spider)
        camera:startShake(0.1, 1.2)
    end,

    onDeath = function(spider)
        camera:startShake(0.25, 2)
    end,
}
