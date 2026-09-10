seen = {ticks = 0}

return {
    onNoise = function(boar, kind, at)
        seen.kind = kind
        seen.state = boar:state()
        if kind == 'landing' and boar:onSameSurfaceAs(at) then
            boar:event('heard', true)
        end
    end,
    onTick = function(boar)
        seen.ticks = seen.ticks + 1
        local threat = boar:threatFeet()
        boar:fact('near', threat ~= nil and boar:onSameSurfaceAs(threat)
            and boar:distanceTo(threat) <= boar:tuning('range'))
    end,
}