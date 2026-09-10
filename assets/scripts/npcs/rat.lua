local function sense(rat)
    local threat = rat:threatFeet()
    rat:fact("threatOnMySurface", threat ~= nil and rat:onSameSurfaceAs(threat))
    rat:fact("threatClose", threat ~= nil and rat:distanceTo(threat) <= rat:tuning("close"))
    rat:fact("threatInReach", threat ~= nil and rat:distanceTo(threat) <= rat:tuning("reach"))
    rat:fact("cornered", threat ~= nil and rat:corneredBy(threat))
end

return {
    onTick = function(rat, dt)
        sense(rat)
    end,

    onHurt = function(rat)
        camera:startShake(0.06, 0.6)
    end,

    onDied = function(rat)
        camera:startShake(0.12, 1)
    end,
}
