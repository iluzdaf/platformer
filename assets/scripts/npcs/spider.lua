return {
    onHurt = function(spider)
        camera:startShake(0.1, 1.2)
    end,

    onDied = function(spider)
        camera:startShake(0.25, 2)
    end,
}
