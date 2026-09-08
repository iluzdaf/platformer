local function deathCoroutine()
    waitSeconds(0.25)
    world:respawnPlayer()
    playback:play()
end

local function levelCompleteCoroutine()
    waitSeconds(0)
    playback:pause()
    world:loadLevel(level:getNextLevel())
    screenTransition:start(0.5, true)
    waitSeconds(0.5)
    playback:play()
end

return {
    onDeath = function(player)
        playback:pause()
        camera:startShake(0.25, 4)
        startCoroutine(deathCoroutine)
    end,

    onHurt = function(player)
        camera:startShake(0.15, 2)
    end,

    onLevelComplete = function(player)
        startCoroutine(levelCompleteCoroutine)
    end,

    onWallJump = function(player)
        camera:startShake(0.1, 1)
    end,

    onDash = function(player)
        camera:startShake(0.25, 2)
    end,

    onAttack = function(player)
        camera:startShake(0.1, 0.8)
    end,

    onWallSliding = function(player)
        camera:startShake(0.1, 0.3)
    end,

    onFallFromHeight = function(player)
        camera:startShake(0.25, 2)
    end,

    onHitCeiling = function(player)
        camera:startShake(0.1, 0.5)
    end,
}
