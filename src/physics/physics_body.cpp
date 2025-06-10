#include "physics/physics_body.hpp"
#include "game/tile_map/tile_map.hpp"

void PhysicsBody::setPosition(glm::vec2 position)
{
    this->position = position;
    this->nextPosition = position;
}

glm::vec2 PhysicsBody::getPosition() const
{
    return position;
}

void PhysicsBody::setVelocity(glm::vec2 velocity)
{
    this->velocity = velocity;
}

glm::vec2 PhysicsBody::getVelocity() const
{
    return velocity;
}

void PhysicsBody::setColliderSize(glm::vec2 size)
{
    this->colliderSize = size;
}

glm::vec2 PhysicsBody::getColliderSize() const
{
    return colliderSize;
}

void PhysicsBody::setGravity(float gravity)
{
    this->gravity = gravity;
}

float PhysicsBody::getGravity() const
{
    return gravity;
}

AABB PhysicsBody::getAABB() const
{
    return AABB(position + colliderOffset, colliderSize);
}

void PhysicsBody::resolveCollision(const TileMap &tileMap)
{
    collisionAABBX = AABB();
    collisionAABBY = AABB();

    glm::vec2 positionWithOffset = position + colliderOffset;
    glm::vec2 nextPositionWithOffset = nextPosition + colliderOffset;

    glm::vec2 proposedPosX = {nextPositionWithOffset.x, positionWithOffset.y};
    AABB proposedAABBX(proposedPosX, colliderSize);
    AABB solidAABBX = tileMap.getSolidAABBAt(proposedPosX, colliderSize);
    if (solidAABBX.intersects(proposedAABBX))
    {
        collisionAABBX = solidAABBX;
        float deltaX = (proposedAABBX.center().x - solidAABBX.center().x);
        float overlapX = (solidAABBX.size.x + proposedAABBX.size.x) * 0.5f - std::abs(deltaX);
        if (deltaX > 0)
            nextPositionWithOffset.x += overlapX;
        else
            nextPositionWithOffset.x -= overlapX;
        velocity.x = 0.0f;
    }

    glm::vec2 proposedPosY = nextPositionWithOffset;
    AABB proposedAABBY(proposedPosY, colliderSize);
    AABB solidAABBY = tileMap.getSolidAABBAt(proposedPosY, colliderSize);
    if (solidAABBY.intersects(proposedAABBY))
    {
        collisionAABBY = solidAABBY;
        float deltaY = (proposedAABBY.center().y - solidAABBY.center().y);
        float overlapY = (solidAABBY.size.y + proposedAABBY.size.y) * 0.5f - std::abs(deltaY);
        if (deltaY > 0)
            nextPositionWithOffset.y += overlapY;
        else
        {
            nextPositionWithOffset.y -= overlapY;
        }
        velocity.y = 0.0f;
    }

    nextPosition = nextPositionWithOffset - colliderOffset;
}

void PhysicsBody::clampToTileMapBounds(const TileMap &tileMap)
{
    const int mapWidth = tileMap.getWorldWidth();
    const int mapHeight = tileMap.getWorldHeight();

    glm::vec2 newPosition = nextPosition + colliderOffset;
    glm::vec2 newVelocity = velocity;
    bool clamped = false;

    AABB playerAABB(newPosition, colliderSize);

    if (playerAABB.left() < 0.0f)
    {
        newPosition.x = 0.0f;
        newVelocity.x = 0.0f;
        clamped = true;
    }
    else if (playerAABB.right() > mapWidth)
    {
        newPosition.x = mapWidth - colliderSize.x;
        newVelocity.x = 0.0f;
        clamped = true;
    }

    if (playerAABB.top() < 0.0f)
    {
        newPosition.y = 0.0f;
        newVelocity.y = 0.0f;
        clamped = true;
    }
    else if (playerAABB.bottom() > mapHeight)
    {
        newPosition.y = mapHeight - colliderSize.y;
        newVelocity.y = 0.0f;
        clamped = true;
    }

    if (clamped)
    {
        nextPosition = newPosition - colliderOffset;
        velocity = newVelocity;
    }
}

bool PhysicsBody::contactInDirection(const TileMap &tileMap, glm::vec2 direction) const
{
    glm::vec2 probePos = position + colliderOffset + direction;
    AABB probeAABB(probePos, colliderSize);
    AABB solidAABB = tileMap.getSolidAABBAt(probePos, colliderSize);
    return solidAABB.intersects(probeAABB);
}

bool PhysicsBody::contactWithLeftWall(const TileMap &tileMap)
{
    return contactInDirection(tileMap, glm::vec2(-0.1f, 0.0f));
}

bool PhysicsBody::contactWithRightWall(const TileMap &tileMap)
{
    return contactInDirection(tileMap, glm::vec2(0.1f, 0.0f));
}

bool PhysicsBody::contactWithGround(const TileMap &tileMap)
{
    return contactInDirection(tileMap, glm::vec2(0.0f, 0.1f)) ||
           AABB(position + colliderOffset, colliderSize).bottom() >= tileMap.getWorldHeight();
}

bool PhysicsBody::contactWithCeiling(const TileMap &tileMap) const
{
    return contactInDirection(tileMap, glm::vec2(0.0f, -0.1f));
}

void PhysicsBody::applyGravity(float deltaTime)
{
    velocity.y += gravity * deltaTime;
}

void PhysicsBody::stepPhysics(float deltaTime, const TileMap &tileMap)
{
    nextPosition += velocity * deltaTime;

    resolveCollision(tileMap);
    
    clampToTileMapBounds(tileMap);

    position = nextPosition;
}

void PhysicsBody::setColliderOffset(glm::vec2 offset)
{
    this->colliderOffset = offset;
}

glm::vec2 PhysicsBody::getColliderOffset() const
{
    return colliderOffset;
}

AABB PhysicsBody::getCollisionAABBX() const
{
    return collisionAABBX;
}

AABB PhysicsBody::getCollisionAABBY() const
{
    return collisionAABBY;
}

void PhysicsBody::reset()
{
    position = glm::vec2(0, 0);
    nextPosition = glm::vec2(0, 0);
    velocity = glm::vec2(0);
    collisionAABBX = AABB();
    collisionAABBY = AABB();
}