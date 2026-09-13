return {
    onHurt = function(spider)
        camera:startShake(0.1, 1.2)
    end,

    onDeath = function(spider)
        camera:startShake(0.25, 2)
    end,

    states = {
        patrol = include('scripts/behaviors/patrol.lua'),
        chase = include('scripts/behaviors/chase.lua'),
    },
}
