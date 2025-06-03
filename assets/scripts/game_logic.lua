waitSeconds = coroutine.yield

function onRespawn()
    startCoroutine(respawnCoroutine)
end

function respawnCoroutine()
    game.isPaused = true
    camera:startShake(0.25, 4)
    waitSeconds(0.25)
    player:setPosition(tileMap:getPlayerStartWorldPosition())
    game.isPaused = false
end

function onLevelComplete()
    startCoroutine(levelCompleteCoroutine)
end

function levelCompleteCoroutine()
    game.isPaused = true
    waitSeconds(0.1)
    game:loadNextLevel()
    player:setPosition(tileMap:getPlayerStartWorldPosition())
    game.isPaused = false
end