#include "GameCamera.h"

#include <algorithm>
#include <cmath>

namespace
{
constexpr float kCameraTraumaDecay = 1.85f;
constexpr float kCameraImpactDamping = 11.5f;
constexpr float kCameraOffsetDamping = 14.0f;
constexpr float kCameraZoomRecovery = 4.8f;
constexpr float kCameraMaxZoomPunch = 0.13f;
constexpr sf::Vector2f kCameraShakeAmplitude = {28.f, 18.f};

sf::Vector2f clampVectorMagnitude(sf::Vector2f value, sf::Vector2f maxMagnitude)
{
    value.x = std::clamp(value.x, -maxMagnitude.x, maxMagnitude.x);
    value.y = std::clamp(value.y, -maxMagnitude.y, maxMagnitude.y);
    return value;
}

sf::Vector2f normalizeOrFallback(sf::Vector2f value, sf::Vector2f fallback = {0.f, -1.f})
{
    const float length = std::sqrt(value.x * value.x + value.y * value.y);
    if (length <= 0.0001f)
    {
        return fallback;
    }

    return value / length;
}

float expSmoothingFactor(float sharpness, float deltaTime)
{
    return 1.f - std::exp(-sharpness * deltaTime);
}
}

void GameCamera::movementUpdate(float deltatime, const sf::FloatRect& cameraBounds)
{
    targetPos = chasePlayer
        ? calculateFollowTarget(deltatime, cameraBounds)
        : clampToLevelBounds(targetPos, cameraBounds);

    const float snapDistanceX = getScreenViewSize().x * BASE_CAMERA_SNAP_DISTANCE_FACTOR;
    const float snapDistanceY = getScreenViewSize().y * BASE_CAMERA_SNAP_DISTANCE_FACTOR;

    if (std::abs(targetPos.x - cameraPos.x) > snapDistanceX || std::abs(targetPos.y - cameraPos.y) > snapDistanceY)
    {
        cameraPos = targetPos;
        resetMotionState();
        return;
    }

    cameraPos.x = smoothDamp(
        cameraPos.x,
        targetPos.x,
        speed.x,
        BASE_CAMERA_SMOOTH_TIME.x,
        maxSpeed.x,
        deltatime
    );

    cameraPos.y = smoothDamp(
        cameraPos.y,
        targetPos.y,
        speed.y,
        BASE_CAMERA_SMOOTH_TIME.y,
        maxSpeed.y,
        deltatime
    );

    cameraPos = clampToLevelBounds(cameraPos, cameraBounds);
}

sf::Vector2f GameCamera::clampToLevelBounds(sf::Vector2f pos, const sf::FloatRect& cameraBounds) const
{
    const sf::Vector2f viewSize = getScreenViewSize();
    const float halfWidth = viewSize.x / 2.f;
    const float halfHeight = viewSize.y / 2.f;

    const float left = cameraBounds.position.x;
    const float top = cameraBounds.position.y;
    const float right = cameraBounds.position.x + cameraBounds.size.x;
    const float bottom = cameraBounds.position.y + cameraBounds.size.y;

    const float minX = left + halfWidth;
    const float maxX = right - halfWidth;
    if (maxX < minX)
    {
        pos.x = left + cameraBounds.size.x * 0.5f;
    }
    else
    {
        pos.x = std::clamp(pos.x, minX, maxX);
    }

    const float minY = top + halfHeight;
    const float maxY = bottom - halfHeight;
    if (maxY < minY)
    {
        pos.y = top + cameraBounds.size.y * 0.5f;
    }
    else
    {
        pos.y = std::clamp(pos.y, minY, maxY);
    }

    return pos;
}

sf::Vector2f GameCamera::calculateFollowTarget(float deltatime, const sf::FloatRect& cameraBounds)
{
    const sf::Vector2f playerPos = player->getCenterPosition();

    if (!hasLastPlayerPos)
    {
        lastPlayerPos = playerPos;
        hasLastPlayerPos = true;
    }

    const float safeDeltaTime = std::max(deltatime, 0.0001f);
    const sf::Vector2f rawPlayerVelocity = (playerPos - lastPlayerPos) / safeDeltaTime;
    lastPlayerPos = playerPos;

    smoothedPlayerVelocity += (rawPlayerVelocity - smoothedPlayerVelocity)
        * expSmoothingFactor(BASE_CAMERA_LOOK_AHEAD_SHARPNESS, safeDeltaTime);

    const sf::Vector2f lookAheadOffset = clampVectorMagnitude(
        smoothedPlayerVelocity * BASE_CAMERA_LOOK_AHEAD_TIME,
        BASE_CAMERA_LOOK_AHEAD_MAX
    );

    return clampToLevelBounds(playerPos + lookAheadOffset, cameraBounds);
}

sf::Vector2f GameCamera::calculateShakeOffset() const
{
    const float shakeStrength = shakeTrauma * shakeTrauma;
    return {
        std::sin(shakeTime * 47.f) * kCameraShakeAmplitude.x * shakeStrength,
        std::cos(shakeTime * 63.f + 0.8f) * kCameraShakeAmplitude.y * shakeStrength
    };
}

void GameCamera::updateScreenEffects(float deltaTime, const sf::FloatRect& cameraBounds)
{
    shakeTime += deltaTime;
    shakeTrauma = std::max(0.f, shakeTrauma - kCameraTraumaDecay * deltaTime);
    zoomPunch = std::max(0.f, zoomPunch - kCameraZoomRecovery * deltaTime);

    impactVelocity *= std::exp(-kCameraImpactDamping * deltaTime);
    impactOffset += impactVelocity * deltaTime;
    impactOffset *= std::exp(-kCameraOffsetDamping * deltaTime);

    const sf::Vector2f finalCenter = clampToLevelBounds(
        cameraPos + impactOffset + calculateShakeOffset(),
        cameraBounds
    );
    const float zoomFactor = std::clamp(1.f - zoomPunch, 0.82f, 1.06f);

    view->setSize({WINDOW_WIDTH * ZOOM_SCALE * zoomFactor, WINDOW_HEIGHT * ZOOM_SCALE * zoomFactor});
    view->setCenter(finalCenter);
}

float GameCamera::smoothDamp(
    float current,
    float target,
    float& currentVelocity,
    float smoothTime,
    float maxSpeedValue,
    float deltaTime
)
{
    smoothTime = std::max(0.0001f, smoothTime);

    const float omega = 2.f / smoothTime;
    const float x = omega * deltaTime;
    const float exp = 1.f / (1.f + x + 0.48f * x * x + 0.235f * x * x * x);

    float change = current - target;
    const float originalTarget = target;

    const float maxChange = maxSpeedValue * smoothTime;
    change = std::clamp(change, -maxChange, maxChange);
    target = current - change;

    const float temp = (currentVelocity + omega * change) * deltaTime;
    currentVelocity = (currentVelocity - omega * temp) * exp;

    float output = target + (change + temp) * exp;

    if ((originalTarget - current > 0.f) == (output > originalTarget))
    {
        output = originalTarget;
        currentVelocity = 0.f;
    }

    return output;
}

void GameCamera::resetMotionState()
{
    speed = {0.f, 0.f};
    smoothedPlayerVelocity = {0.f, 0.f};
}

GameCamera::GameCamera(sf::View& view)
{
    this->view = &view;
    this->view->setSize({WINDOW_WIDTH * ZOOM_SCALE, WINDOW_HEIGHT * ZOOM_SCALE});
    this->view->setCenter(cameraPos);
}

GameCamera::~GameCamera()
{
}

void GameCamera::update()
{
    static sf::Clock clock;
    const float dt = std::clamp(clock.restart().asSeconds(), 0.0001f, 0.05f);

    if (!player || !levelManager || !view)
    {
        return;
    }

    const sf::Vector2i levelSize = levelManager->getCurrentLevelSize();
    mapBorders = {static_cast<float>(levelSize.x), static_cast<float>(levelSize.y)};

    if (!isConditionSuccessed)
    {
        bool shouldReleaseTarget = false;

        if (useTimedPointTarget && pointTargetClock.getElapsedTime() >= pointTargetDuration)
        {
            shouldReleaseTarget = true;
        }

        if (releaseCondition && releaseCondition())
        {
            shouldReleaseTarget = true;
        }

        if (shouldReleaseTarget)
        {
            isConditionSuccessed = true;
            chasePlayer = true;
            useTimedPointTarget = false;
            pointTargetDuration = sf::Time::Zero;
            releaseCondition = {};
        }
    }

    const sf::FloatRect cameraBounds = levelManager->getCurrentCameraBoundsForPosition(player->getCenterPosition());

    movementUpdate(dt, cameraBounds);

    updateScreenEffects(dt, cameraBounds);
}

void GameCamera::pointCameraAt(sf::Vector2f pos, std::function<bool()> condition)
{
    chasePlayer = false;
    isConditionSuccessed = false;
    targetPos = pos;
    releaseCondition = std::move(condition);
    useTimedPointTarget = false;
    pointTargetDuration = sf::Time::Zero;
    pointTargetClock.restart();
}

void GameCamera::pointCameraAt(sf::Vector2f pos, unsigned int time)
{
    chasePlayer = false;
    isConditionSuccessed = false;
    targetPos = pos;
    releaseCondition = {};
    useTimedPointTarget = true;
    pointTargetDuration = sf::milliseconds(time);
    pointTargetClock.restart();
}

void GameCamera::setMoveSpeed(sf::Vector2f pos)
{
    maxSpeed = {
        std::max(1.f, std::abs(pos.x)),
        std::max(1.f, std::abs(pos.y))
    };
}

void GameCamera::attachGameLevelManager(GameLevelManager &m)
{
    this->levelManager = &m;
}

void GameCamera::attachPlayer(Player &player)
{
    this->player = &player;
    lastPlayerPos = player.getCenterPosition();
    hasLastPlayerPos = true;
}

void GameCamera::setCenterPosition(sf::Vector2f pos)
{
    if (levelManager)
    {
        const sf::Vector2i levelSize = levelManager->getCurrentLevelSize();
        mapBorders = {static_cast<float>(levelSize.x), static_cast<float>(levelSize.y)};
        pos = clampToLevelBounds(pos, levelManager->getCurrentCameraBoundsForPosition(pos));
    }

    cameraPos = pos;
    targetPos = pos;
    resetMotionState();

    if (player)
    {
        lastPlayerPos = player->getCenterPosition();
        hasLastPlayerPos = true;
    }

    if (view)
    {
        view->setSize({WINDOW_WIDTH * ZOOM_SCALE, WINDOW_HEIGHT * ZOOM_SCALE});
        view->setCenter(cameraPos);
    }
}

void GameCamera::addImpact(const sf::Vector2f& direction, float impulseStrength, float trauma, float zoomPunchAmount)
{
    const sf::Vector2f normalizedDirection = normalizeOrFallback(direction);
    impactVelocity += normalizedDirection * impulseStrength;
    impactOffset += normalizedDirection * std::min(impulseStrength * 0.08f, 18.f);
    shakeTrauma = std::clamp(shakeTrauma + trauma, 0.f, 1.f);
    zoomPunch = std::clamp(std::max(zoomPunch, zoomPunchAmount), 0.f, kCameraMaxZoomPunch);
}

void GameCamera::clearEffects()
{
    impactOffset = {0.f, 0.f};
    impactVelocity = {0.f, 0.f};
    shakeTrauma = 0.f;
    zoomPunch = 0.f;
    shakeTime = 0.f;

    if (view)
    {
        view->setSize({WINDOW_WIDTH * ZOOM_SCALE, WINDOW_HEIGHT * ZOOM_SCALE});
        view->setCenter(cameraPos);
    }
}

float GameCamera::getZoom() const
{
    return ZOOM_SCALE;
}

sf::Vector2f GameCamera::getScreenViewSize() const
{
    if (view)
    {
        return view->getSize();
    }

    return {WINDOW_WIDTH * ZOOM_SCALE, WINDOW_HEIGHT * ZOOM_SCALE};
}

sf::Vector2f GameCamera::getScreenViewPos() const
{
    return {
        view->getCenter().x - getScreenViewSize().x / 2.f,
        view->getCenter().y - getScreenViewSize().y / 2.f
    };
}

sf::Vector2f GameCamera::getSpeed() const
{
    return this->speed;
}

sf::Vector2f GameCamera::getCameraCenter() const
{
    return view->getCenter();
}

sf::Vector2f GameCamera::getCameraCenterPos() const
{
    return this->cameraPos;
}
