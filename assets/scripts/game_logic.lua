waitSeconds = coroutine.yield

function onDeath()
    playback:pause()
    camera:startShake(0.25, 4)
    startCoroutine(deathCoroutine)
end

function deathCoroutine()
    waitSeconds(0.25)
    world:respawnPlayer()
    playback:play()
end

function onLevelComplete()    
    startCoroutine(levelCompleteCoroutine)
end

function levelCompleteCoroutine()    
    waitSeconds(0);
    playback:pause()
    world:loadLevel(level:getNextLevel())
    screenTransition:start(0.5, true)
    waitSeconds(0.5)
    playback:play()
end

function onWallJump()
    camera:startShake(0.1, 1)
end

function onDoubleJump()
    camera:startShake(0.1, 1)
end

function onDash()
    camera:startShake(0.25, 2)
end

function onFallFromHeight()
    camera:startShake(0.25, 2)
end

function onHitCeiling()
    camera:startShake(0.1, 0.5)
end

function onWallSliding()
    camera:startShake(0.1, 0.3)
end

function onGameLoaded()
    screenTransition:start(0.5, true)
end