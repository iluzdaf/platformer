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

void PhysicsBody::setSize(glm::vec2 size)
{
    this->size = size;
}

glm::vec2 PhysicsBody::getSize() const
{
    return size;
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
    return AABB(position + offset, size);
}

void PhysicsBody::resolveCollision(const TileMap &tileMap)
{
    glm::vec2 newPosition = nextPosition + offset;

    if (std::abs(velocity.x) > 0.001f)
    {
        glm::vec2 proposedPosX = {newPosition.x, position.y};
        AABB proposedAABBX(proposedPosX, size);
        AABB solidAABBX = tileMap.getSolidAABBAt(proposedPosX, size);
        if (solidAABBX.intersects(proposedAABBX))
        {
            float deltaX = (proposedAABBX.center().x - solidAABBX.center().x);
            float overlapX = (solidAABBX.size.x + proposedAABBX.size.x) * 0.5f - std::abs(deltaX);
            if (deltaX > 0)
                newPosition.x += overlapX;
            else
                newPosition.x -= overlapX;
            velocity.x = 0.0f;
        }
    }

    if (std::abs(velocity.y) > 0.001f)
    {
        glm::vec2 proposedPosY = newPosition;
        AABB proposedAABBY(proposedPosY, size);
        AABB solidAABBY = tileMap.getSolidAABBAt(proposedPosY, size);
        if (solidAABBY.intersects(proposedAABBY))
        {
            float deltaY = (proposedAABBY.center().y - solidAABBY.center().y);
            float overlapY = (solidAABBY.size.y + proposedAABBY.size.y) * 0.5f - std::abs(deltaY);
            if (deltaY > 0)
                newPosition.y += overlapY;
            else
            {
                newPosition.y -= overlapY;
            }
            velocity.y = 0.0f;
        }
    }

    nextPosition = newPosition - offset;
}

void PhysicsBody::clampToTileMapBounds(const TileMap &tileMap)
{
    const int mapWidth = tileMap.getWorldWidth();
    const int mapHeight = tileMap.getWorldHeight();

    glm::vec2 newPosition = nextPosition + offset;
    glm::vec2 newVelocity = velocity;
    bool clamped = false;

    AABB playerAABB(newPosition, size);

    if (playerAABB.left() < 0.0f)
    {
        newPosition.x = 0.0f;
        newVelocity.x = 0.0f;
        clamped = true;
    }
    else if (playerAABB.right() > mapWidth)
    {
        newPosition.x = mapWidth - size.x;
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
        newPosition.y = mapHeight - size.y;
        newVelocity.y = 0.0f;
        clamped = true;
    }

    if (clamped)
    {
        newPosition -= offset;
        AABB clampedAABB(newPosition, size);
        AABB solidAABB = tileMap.getSolidAABBAt(newPosition, size);
        if (solidAABB.intersects(clampedAABB))
        {
            throw std::runtime_error("Trying to clamp player into a solid tile");
        }

        nextPosition = newPosition;
        velocity = newVelocity;
    }
}

bool PhysicsBody::contactInDirection(const TileMap &tileMap, glm::vec2 direction) const
{
    glm::vec2 probePos = position + offset + direction;
    AABB probeAABB(probePos, size);
    AABB solidAABB = tileMap.getSolidAABBAt(probePos, size);
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
    return contactInDirection(tileMap, glm::vec2(0.0f, 0.1f));
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

void PhysicsBody::setOffset(glm::vec2 offset)
{
    this->offset = offset;
}

glm::vec2 PhysicsBody::getOffset() const
{
    return offset;
}