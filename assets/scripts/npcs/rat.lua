return {
    onHurt = function(rat)
        camera:startShake(0.06, 0.6)
    end,

    onDied = function(rat)
        camera:startShake(0.12, 1)
    end,
}
