waitSeconds = coroutine.yield

function onDeath()
    startCoroutine(deathCoroutine)
end

function deathCoroutine()
    game:pause()
    camera:startShake(0.25, 4)
    waitSeconds(0.25)
    player:reset()
    player:setPosition(tileMap:getPlayerStartWorldPosition())
    game:play()
end

function onLevelComplete()
    startCoroutine(levelCompleteCoroutine)
end

function levelCompleteCoroutine()
    game:pause()
    screenTransition:start(0.4, true)
    waitSeconds(0.0)
    game:loadNextLevel()
    player:reset()
    player:setPosition(tileMap:getPlayerStartWorldPosition())
    waitSeconds(0.4)
    game:play()
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