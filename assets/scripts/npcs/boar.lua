return {
    onNoise = function(boar, kind, at)
        if kind == 'landing' and boar:onSameSurfaceAs(at) then
            boar:event('heard', true)
        end
    end,
    onTick = function(boar)
        local threat = boar:threatFeet()
        boar:fact('near', threat ~= nil and boar:onSameSurfaceAs(threat)
            and boar:distanceTo(threat) <= boar:tuning('range'))
    end,
}