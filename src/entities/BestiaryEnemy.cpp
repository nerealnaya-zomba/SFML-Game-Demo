#include "BestiaryEnemy.h"

#include <Bullet.h>
#include <EnemyManager.h>
#include <GameLevel.h>
#include <Ground.h>
#include <Platform.h>
#include <Player.h>

#include <algorithm>
#include <cmath>
#include <cctype>

using namespace gameUtils;

namespace
{
constexpr float kPi = 3.14159265f;

float vectorLength(const sf::Vector2f& value)
{
    return std::sqrt(value.x * value.x + value.y * value.y);
}

sf::Vector2f normalizeOrZero(const sf::Vector2f& value)
{
    const float length = vectorLength(value);
    if (length <= 0.0001f)
    {
        return {0.f, 0.f};
    }

    return value / length;
}

float approachValue(float current, float target, float maxDelta)
{
    if (current < target)
    {
        return std::min(current + maxDelta, target);
    }

    return std::max(current - maxDelta, target);
}

float clampAbs(float value, float maxAbs)
{
    return std::clamp(value, -maxAbs, maxAbs);
}

std::string toLowerCopy(std::string value)
{
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char symbol) {
        return static_cast<char>(std::tolower(symbol));
    });
    return value;
}
}

BestiaryEnemy::BestiaryEnemy(GameData& gameData,
                             EnemyManager& enemyManager,
                             GameLevel& gameLevel,
                             sf::RenderWindow& window,
                             Ground& ground,
                             Platform& platform,
                             Player& player,
                             const std::string& type,
                             sf::Vector2f position)
    : Enemy(gameData)
{
    window_ = &window;
    ground_ = &ground;
    platform_ = &platform;
    player_ = &player;
    gameLevel_ = &gameLevel;
    gameData_ = &gameData;
    enemyManager_ = &enemyManager;
    spawnPosition_ = position;

    const std::string normalizedType = toLowerCopy(type);
    if (normalizedType == "wraithbat" || normalizedType == "wraith-bat" || normalizedType == "wraith_bat")
    {
        kind_ = Kind::WraithBat;
        typeKey_ = "wraith-bat";
    }
    else if (normalizedType == "voidslime" || normalizedType == "void-slime" || normalizedType == "void_slime")
    {
        kind_ = Kind::VoidSlime;
        typeKey_ = "void-slime";
    }
    else
    {
        kind_ = Kind::DreadScorpion;
        typeKey_ = "dread-scorpion";
    }

    loadData();
    attachTextures();

    sprite_ = std::make_unique<sf::Sprite>(idleTextures_->at(0));
    setSpriteOriginToMiddle(*sprite_);
    sprite_->setScale(enemyScale_);

    rect_ = std::make_unique<sf::RectangleShape>(hitboxSize_);
    rect_->setFillColor(sf::Color::Transparent);
    rect_->setPosition(position);

    portal_ = std::make_unique<enemyPortal>(gameData, position);
    portal_->setPosition(getCenterPosition());
    syncSpriteToRect();

    healthBar_ = std::make_unique<HealthBar>(
        rect_.get(),
        window,
        sf::Color(48, 14, 18, 210),
        kind_ == Kind::VoidSlime ? sf::Color(90, 62, 198, 220) : sf::Color(186, 88, 70, 220),
        sf::Vector2f{46.f, 5.f},
        HP_,
        sf::Vector2f{0.f, -38.f}
    );

    hoverPhase_ = random(0.f, kPi * 2.f);
    patrolDir_ = random(0, 1) == 0 ? -1.f : 1.f;

    stateClock_.restart();
    attackCooldownClock_.restart();
    aggroClock_.restart();
    auraClock_.restart();
    patrolEffectClock_.restart();
    hopClock_.restart();
    deathSmokeClock_.restart();
}

BestiaryEnemy::~BestiaryEnemy() = default;

void BestiaryEnemy::loadData()
{
    const nlohmann::json& settings = gameData_->getEnemySettings();
    const nlohmann::json& entry = settings[typeKey_];

    enemyScale_ = {entry["scaleX"], entry["scaleY"]};
    hitboxSize_ = {entry["hitboxWidth"], entry["hitboxHeight"]};
    spriteOffsetY_ = entry["spriteOffsetY"];
    HP_ = entry["HP"];
    maxHP_ = HP_;
    DMG_ = entry["DMG"];
    knockback_ = {entry["KnockbackX"], entry["KnockbackY"]};
    maxSpeed_ = entry["maxSpeed"];
    acceleration_ = entry["acceleration"];
    friction_ = entry["friction"];
    gravity_ = entry["gravity"];
    alertDistance_ = entry["alertDistance"];
    attackDistance_ = entry["attackDistance"];
    loseDistance_ = entry["loseDistance"];
    verticalTolerance_ = entry["verticalTolerance"];
    patrolRange_ = entry["patrolRange"];
    hoverAmplitude_ = entry.value("hoverAmplitude", hoverAmplitude_);
    hoverFrequency_ = entry.value("hoverFrequency", hoverFrequency_);
    chargeSpeed_ = entry.value("chargeSpeed", chargeSpeed_);
    groundHopX_ = entry.value("groundHopX", groundHopX_);
    groundHopY_ = entry.value("groundHopY", groundHopY_);
    leapAttackX_ = entry.value("leapAttackX", leapAttackX_);
    leapAttackY_ = entry.value("leapAttackY", leapAttackY_);
    attackCooldownMs_ = entry["attackCooldownMs"];
    attackDurationMs_ = entry["attackDurationMs"];
    windupDurationMs_ = entry["windupDurationMs"];
    recoverDurationMs_ = entry["recoverDurationMs"];
    hurtDurationMs_ = entry["hurtDurationMs"];
    deathDurationMs_ = entry["deathDurationMs"];
    attackImpactFrame_ = entry.value("attackImpactFrame", attackImpactFrame_);

    if (kind_ == Kind::WraithBat)
    {
        baseTint_ = sf::Color(186, 136, 226, 255);
    }
    else if (kind_ == Kind::VoidSlime)
    {
        baseTint_ = sf::Color(120, 74, 206, 255);
    }
    else
    {
        baseTint_ = sf::Color(188, 214, 154, 255);
    }
}

void BestiaryEnemy::attachTextures()
{
    switch (kind_)
    {
        case Kind::WraithBat:
            attachTexture(gameData_->wraithBat_flyTextures, idleTextures_, gameData_->wraithBat_fly_helper, idleHelper_);
            attachTexture(gameData_->wraithBat_flyTextures, moveTextures_, gameData_->wraithBat_fly_helper, moveHelper_);
            attachTexture(gameData_->wraithBat_attackTextures, attackTextures_, gameData_->wraithBat_attack_helper, attackHelper_);
            break;

        case Kind::VoidSlime:
            attachTexture(gameData_->voidSlime_idleTextures, idleTextures_, gameData_->voidSlime_idle_helper, idleHelper_);
            attachTexture(gameData_->voidSlime_leapTextures, moveTextures_, gameData_->voidSlime_leap_helper, moveHelper_);
            attachTexture(gameData_->voidSlime_leapTextures, attackTextures_, gameData_->voidSlime_leap_helper, attackHelper_);
            break;

        case Kind::DreadScorpion:
            attachTexture(gameData_->dreadScorpion_walkTextures, idleTextures_, gameData_->dreadScorpion_walk_helper, idleHelper_);
            attachTexture(gameData_->dreadScorpion_walkTextures, moveTextures_, gameData_->dreadScorpion_walk_helper, moveHelper_);
            attachTexture(gameData_->dreadScorpion_stabTextures, attackTextures_, gameData_->dreadScorpion_stab_helper, attackHelper_);
            moveHelper_.iterationsTillSwitch = 12;
            attackHelper_.iterationsTillSwitch = 16;
            break;
    }
}

void BestiaryEnemy::resetAnimationHelpers()
{
    idleHelper_.ptrToTexture = 0;
    idleHelper_.iterationCounter = 0;
    moveHelper_.ptrToTexture = 0;
    moveHelper_.iterationCounter = 0;
    attackHelper_.ptrToTexture = 0;
    attackHelper_.iterationCounter = 0;
}

void BestiaryEnemy::setState(State newState)
{
    if (state_ == newState)
    {
        return;
    }

    state_ = newState;
    stateClock_.restart();
    attackDamageApplied_ = false;
    resetAnimationHelpers();
    if (state_ != State::Die)
    {
        scorpionDeathPulseMidPlayed_ = false;
        scorpionDeathPulseLatePlayed_ = false;
    }

    if (state_ == State::Windup)
    {
        spawnNoticeEffect();
    }
    else if (state_ == State::Attack)
    {
        spawnAttackEffect();
    }
    else if (state_ == State::Die && !deathEffectPlayed_)
    {
        hitFlashActive_ = false;
        spawnDeathEffect();
        deathEffectPlayed_ = true;
    }
}

sf::Vector2f BestiaryEnemy::getCenterPosition() const
{
    return rect_ ? rect_->getGlobalBounds().getCenter() : spawnPosition_;
}

float BestiaryEnemy::getFacingSign() const
{
    return facingRight_ ? 1.f : -1.f;
}

void BestiaryEnemy::updateAI()
{
    if (!isAlive || !portal_->getIsHalfPassed())
    {
        return;
    }

    if (HP_ <= 0 && state_ != State::Die)
    {
        setState(State::Die);
    }

    if (state_ == State::Die)
    {
        return;
    }

    const sf::Vector2f center = getCenterPosition();
    const sf::Vector2f playerCenter = player_->playerRectangle_->getGlobalBounds().getCenter();

    if (state_ == State::Hurt)
    {
        if (stateClock_.getElapsedTime().asMilliseconds() >= hurtDurationMs_)
        {
            setState(playerDetected_ ? State::Chase : State::Patrol);
        }
        return;
    }

    switch (kind_)
    {
        case Kind::WraithBat:
            updateBatAI(center, playerCenter);
            break;
        case Kind::VoidSlime:
            updateSlimeAI(center, playerCenter);
            break;
        case Kind::DreadScorpion:
            updateScorpionAI(center, playerCenter);
            break;
    }
}

void BestiaryEnemy::updateBatAI(const sf::Vector2f& center, const sf::Vector2f& playerCenter)
{
    const float distanceX = std::abs(center.x - playerCenter.x);
    const float distanceY = std::abs(center.y - playerCenter.y);
    const bool seesPlayer = distanceX <= alertDistance_ && distanceY <= verticalTolerance_;
    const bool hadPlayer = playerDetected_;

    if (seesPlayer)
    {
        playerDetected_ = true;
        aggroClock_.restart();
    }
    else if (aggroClock_.getElapsedTime().asMilliseconds() >= 1600)
    {
        playerDetected_ = false;
    }

    if (!hadPlayer && playerDetected_)
    {
        pushRing(center, sf::Color(186, 126, 230, 190), 10.f, 44.f, 3.f, 2.f, 190.f);
    }

    if (state_ == State::Windup)
    {
        if (stateClock_.getElapsedTime().asMilliseconds() >= windupDurationMs_)
        {
            setState(State::Attack);
            attackVelocity_ = normalizeOrZero(playerCenter - center) * chargeSpeed_;
            if (std::abs(attackVelocity_.x) > 0.18f)
            {
                facingRight_ = attackVelocity_.x >= 0.f;
            }
            attackCooldownClock_.restart();
        }
        return;
    }

    if (state_ == State::Attack)
    {
        damagePlayerOnContact();
        if (stateClock_.getElapsedTime().asMilliseconds() >= attackDurationMs_)
        {
            setState(State::Recover);
        }
        return;
    }

    if (state_ == State::Recover)
    {
        if (stateClock_.getElapsedTime().asMilliseconds() >= recoverDurationMs_)
        {
            setState(playerDetected_ ? State::Chase : State::Patrol);
        }
        return;
    }

    if (playerDetected_)
    {
        if (distanceX <= attackDistance_ &&
            distanceY <= verticalTolerance_ * 0.9f &&
            attackCooldownClock_.getElapsedTime().asMilliseconds() >= attackCooldownMs_)
        {
            setState(State::Windup);
        }
        else
        {
            setState(State::Chase);
        }
    }
    else
    {
        setState(State::Patrol);
    }
}

void BestiaryEnemy::updateSlimeAI(const sf::Vector2f& center, const sf::Vector2f& playerCenter)
{
    const float distanceX = std::abs(center.x - playerCenter.x);
    const float distanceY = std::abs(center.y - playerCenter.y);
    const bool seesPlayer = distanceX <= alertDistance_ && distanceY <= verticalTolerance_;
    const bool hadPlayer = playerDetected_;

    if (seesPlayer)
    {
        playerDetected_ = true;
        aggroClock_.restart();
    }
    else if (aggroClock_.getElapsedTime().asMilliseconds() >= 1800)
    {
        playerDetected_ = false;
    }

    if (!hadPlayer && playerDetected_)
    {
        pushRing(center, sf::Color(128, 90, 214, 175), 12.f, 40.f, 2.8f, 2.f, 175.f);
    }

    facingRight_ = playerCenter.x >= center.x;

    if (state_ == State::Windup)
    {
        if (isOnGround_ && stateClock_.getElapsedTime().asMilliseconds() >= windupDurationMs_)
        {
            setState(State::Attack);
            velocityY_ = -leapAttackY_;
            velocityX_ = getFacingSign() * leapAttackX_;
            attackCooldownClock_.restart();
        }
        return;
    }

    if (state_ == State::Attack)
    {
        damagePlayerOnContact();
        if (isOnGround_ && stateClock_.getElapsedTime().asMilliseconds() >= 180.f)
        {
            spawnLandingEffect();
            setState(State::Recover);
        }
        return;
    }

    if (state_ == State::Recover)
    {
        if (stateClock_.getElapsedTime().asMilliseconds() >= recoverDurationMs_)
        {
            setState(playerDetected_ ? State::Chase : State::Patrol);
        }
        return;
    }

    if (playerDetected_)
    {
        if (distanceX <= attackDistance_ &&
            distanceY <= verticalTolerance_ &&
            isOnGround_ &&
            attackCooldownClock_.getElapsedTime().asMilliseconds() >= attackCooldownMs_)
        {
            setState(State::Windup);
        }
        else
        {
            setState(State::Chase);
        }
    }
    else
    {
        setState(State::Patrol);
    }
}

void BestiaryEnemy::updateScorpionAI(const sf::Vector2f& center, const sf::Vector2f& playerCenter)
{
    const float distanceX = std::abs(center.x - playerCenter.x);
    const float distanceY = std::abs(center.y - playerCenter.y);
    const bool seesPlayer = distanceX <= alertDistance_ && distanceY <= verticalTolerance_;
    const bool hadPlayer = playerDetected_;

    if (seesPlayer)
    {
        playerDetected_ = true;
        aggroClock_.restart();
    }
    else if (aggroClock_.getElapsedTime().asMilliseconds() >= 1700)
    {
        playerDetected_ = false;
    }

    if (!hadPlayer && playerDetected_)
    {
        pushRing(center, sf::Color(182, 214, 132, 170), 12.f, 38.f, 2.5f, 2.f, 170.f);
    }

    if (state_ == State::Windup)
    {
        if (stateClock_.getElapsedTime().asMilliseconds() >= windupDurationMs_)
        {
            setState(State::Attack);
            attackCooldownClock_.restart();
        }
        return;
    }

    if (state_ == State::Attack)
    {
        return;
    }

    if (state_ == State::Recover)
    {
        if (stateClock_.getElapsedTime().asMilliseconds() >= recoverDurationMs_)
        {
            setState(playerDetected_ ? State::Chase : State::Patrol);
        }
        return;
    }

    if (playerDetected_)
    {
        if (distanceX <= attackDistance_ &&
            distanceY <= verticalTolerance_ * 0.8f &&
            attackCooldownClock_.getElapsedTime().asMilliseconds() >= attackCooldownMs_)
        {
            if (std::abs(playerCenter.x - center.x) > 10.f)
            {
                facingRight_ = playerCenter.x >= center.x;
            }
            lockedAttackFacingRight_ = facingRight_;
            setState(State::Windup);
        }
        else
        {
            setState(State::Chase);
        }
    }
    else
    {
        setState(State::Patrol);
    }
}

void BestiaryEnemy::updateControl()
{
    if (!isAlive || !portal_->getIsHalfPassed())
    {
        return;
    }

    const sf::Vector2f center = getCenterPosition();
    const sf::Vector2f playerCenter = player_->playerRectangle_->getGlobalBounds().getCenter();

    switch (kind_)
    {
        case Kind::WraithBat:
            updateBatControl(center, playerCenter);
            break;
        case Kind::VoidSlime:
            updateSlimeControl(center, playerCenter);
            break;
        case Kind::DreadScorpion:
            updateScorpionControl(center, playerCenter);
            break;
    }
}

void BestiaryEnemy::updateBatControl(const sf::Vector2f& center, const sf::Vector2f& playerCenter)
{
    const float time = auraClock_.getElapsedTime().asSeconds() + hoverPhase_;
    constexpr float kBatAimThreshold = 42.f;
    constexpr float kBatVelocityTurnThreshold = 0.7f;

    if (state_ == State::Die)
    {
        velocityX_ *= 0.96f;
        velocityY_ *= 0.96f;
        return;
    }

    if (state_ == State::Attack)
    {
        velocityX_ = attackVelocity_.x;
        velocityY_ = attackVelocity_.y;
        return;
    }

    sf::Vector2f desiredPoint = spawnPosition_;
    float speedLimit = maxSpeed_;

    if (state_ == State::Patrol)
    {
        desiredPoint.x += std::sin(time * 0.9f) * patrolRange_;
        desiredPoint.y += std::cos(time * hoverFrequency_) * hoverAmplitude_;
    }
    else if (state_ == State::Chase)
    {
        const float desiredFacing = (playerCenter.x - center.x) >= 0.f ? 1.f : -1.f;
        desiredPoint = {
            playerCenter.x - desiredFacing * 72.f,
            playerCenter.y - 30.f + std::sin(time * 3.2f) * 18.f
        };
        speedLimit = maxSpeed_ * 1.22f;
    }
    else if (state_ == State::Windup)
    {
        const float desiredFacing = (playerCenter.x - center.x) >= 0.f ? 1.f : -1.f;
        desiredPoint = {
            center.x - desiredFacing * 26.f,
            playerCenter.y - 16.f
        };
        speedLimit = maxSpeed_ * 0.75f;
    }
    else if (state_ == State::Recover)
    {
        desiredPoint = {
            spawnPosition_.x + std::sin(time * 1.4f) * (patrolRange_ * 0.35f),
            spawnPosition_.y + std::cos(time * hoverFrequency_) * (hoverAmplitude_ * 0.75f)
        };
        speedLimit = maxSpeed_ * 1.1f;
    }

    const sf::Vector2f toTarget = desiredPoint - center;
    const sf::Vector2f desiredVelocity = normalizeOrZero(toTarget) * std::min(speedLimit, vectorLength(toTarget) * 0.08f + 0.4f);
    velocityX_ = approachValue(velocityX_, desiredVelocity.x, acceleration_);
    velocityY_ = approachValue(velocityY_, desiredVelocity.y, acceleration_ * 0.9f);
    velocityX_ = clampAbs(velocityX_, speedLimit);
    velocityY_ = clampAbs(velocityY_, speedLimit);

    if (state_ == State::Windup && std::abs(playerCenter.x - center.x) > kBatAimThreshold)
    {
        facingRight_ = playerCenter.x >= center.x;
    }
    else if (std::abs(velocityX_) > kBatVelocityTurnThreshold)
    {
        facingRight_ = velocityX_ >= 0.f;
    }

    if (patrolEffectClock_.getElapsedTime().asMilliseconds() >= 120.f)
    {
        patrolEffectClock_.restart();
        spawnPatrolEffect();
    }
}

void BestiaryEnemy::updateSlimeControl(const sf::Vector2f&, const sf::Vector2f& playerCenter)
{
    if (collidedHorizontally_ && state_ == State::Patrol)
    {
        patrolDir_ *= -1.f;
    }

    if (state_ == State::Patrol)
    {
        applyGroundFriction();
        if (isOnGround_ && hopClock_.getElapsedTime().asMilliseconds() >= 900.f)
        {
            velocityY_ = -groundHopY_;
            velocityX_ = patrolDir_ * groundHopX_;
            hopClock_.restart();
            spawnPatrolEffect();
        }
    }
    else if (state_ == State::Chase)
    {
        facingRight_ = playerCenter.x >= getCenterPosition().x;
        if (isOnGround_ && hopClock_.getElapsedTime().asMilliseconds() >= 620.f)
        {
            velocityY_ = -groundHopY_ * 1.05f;
            velocityX_ = getFacingSign() * groundHopX_ * 1.15f;
            hopClock_.restart();
            spawnPatrolEffect();
        }
    }
    else if (state_ == State::Windup || state_ == State::Recover || state_ == State::Hurt || state_ == State::Die)
    {
        applyGroundFriction();
    }
}

void BestiaryEnemy::updateScorpionControl(const sf::Vector2f& center, const sf::Vector2f& playerCenter)
{
    float desiredSpeed = 0.f;

    if (state_ == State::Patrol)
    {
        if (collidedHorizontally_ || (isOnGround_ && !hasGroundAhead(patrolDir_)))
        {
            patrolDir_ *= -1.f;
            pushRing({center.x, center.y + 8.f}, sf::Color(132, 156, 118, 110), 9.f, 28.f, 2.2f, 2.f, 110.f);
        }

        facingRight_ = patrolDir_ > 0.f;
        desiredSpeed = patrolDir_ * maxSpeed_ * 0.72f;
    }
    else if (state_ == State::Chase)
    {
        const float deltaX = playerCenter.x - center.x;
        const float chaseDir = deltaX >= 0.f ? 1.f : -1.f;
        if (std::abs(deltaX) > 12.f)
        {
            facingRight_ = chaseDir > 0.f;
        }
        if (isOnGround_ && hasGroundAhead(chaseDir))
        {
            desiredSpeed = chaseDir * maxSpeed_;
        }
    }

    if (state_ == State::Windup || state_ == State::Attack || state_ == State::Recover || state_ == State::Hurt || state_ == State::Die)
    {
        desiredSpeed = 0.f;
    }

    velocityX_ = approachValue(velocityX_, desiredSpeed, acceleration_);

    if (state_ == State::Patrol || state_ == State::Chase)
    {
        if (patrolEffectClock_.getElapsedTime().asMilliseconds() >= 170.f)
        {
            patrolEffectClock_.restart();
            spawnPatrolEffect();
        }
    }
}

void BestiaryEnemy::applyGroundFriction()
{
    if (velocityX_ > 0.f)
    {
        velocityX_ = std::max(0.f, velocityX_ - friction_);
    }
    else if (velocityX_ < 0.f)
    {
        velocityX_ = std::min(0.f, velocityX_ + friction_);
    }
}

void BestiaryEnemy::updatePhysics()
{
    if (!isAlive)
    {
        return;
    }

    portal_->update();
    updateVisualEffects();

    if (portal_->getIsHalfPassed())
    {
        if (kind_ == Kind::WraithBat)
        {
            updateFlyingPhysics();
        }
        else
        {
            updateGroundPhysics();
        }

        if (state_ != State::Die)
        {
            checkBulletCollision();
        }

        if (healthBar_)
        {
            healthBar_->update(std::max(0, HP_));
        }
    }
}

void BestiaryEnemy::updateFlyingPhysics()
{
    collidedHorizontally_ = false;
    rect_->move({velocityX_, velocityY_});

    const sf::Vector2f levelBounds = static_cast<sf::Vector2f>(gameLevel_->getLevelSize());
    sf::Vector2f position = rect_->getPosition();

    if (position.x <= 0.f)
    {
        position.x = 0.f;
        velocityX_ = std::abs(velocityX_) * 0.6f;
        collidedHorizontally_ = true;
    }
    else if (position.x + rect_->getSize().x >= levelBounds.x)
    {
        position.x = levelBounds.x - rect_->getSize().x;
        velocityX_ = -std::abs(velocityX_) * 0.6f;
        collidedHorizontally_ = true;
    }

    const float minY = 48.f;
    const float maxY = std::max(minY + 40.f, ground_->getRect().getPosition().y - rect_->getSize().y - 42.f);
    if (position.y <= minY)
    {
        position.y = minY;
        velocityY_ = std::abs(velocityY_) * 0.6f;
    }
    else if (position.y >= maxY)
    {
        position.y = maxY;
        velocityY_ = -std::abs(velocityY_) * 0.6f;
    }

    rect_->setPosition(position);

    if (state_ == State::Die)
    {
        velocityX_ *= 0.95f;
        velocityY_ *= 0.95f;
    }

    if (state_ == State::Die && deathSmokeClock_.getElapsedTime().asMilliseconds() >= 95.f)
    {
        deathSmokeClock_.restart();
        spawnDeathSmoke();
    }

    if (state_ == State::Die && stateClock_.getElapsedTime().asMilliseconds() >= deathDurationMs_)
    {
        if (!hasDroppedGold_ && enemyManager_)
        {
            enemyManager_->dropGold(getCenterPosition(), getDropKey());
            hasDroppedGold_ = true;
        }
        isAlive = false;
    }

    syncSpriteToRect();
}

void BestiaryEnemy::updateGroundPhysics()
{
    collidedHorizontally_ = false;
    const bool wasOnGround = isOnGround_;
    isOnGround_ = false;

    velocityY_ += gravity_;
    rect_->move({0.f, velocityY_});
    checkGroundCollision();
    checkPlatformCollision();

    rect_->move({velocityX_, 0.f});
    checkPlatformCollision();

    const sf::Vector2f levelBounds = static_cast<sf::Vector2f>(gameLevel_->getLevelSize());
    sf::Vector2f position = rect_->getPosition();

    if (position.x <= 0.f)
    {
        position.x = 0.f;
        velocityX_ = std::abs(velocityX_) * 0.2f;
        collidedHorizontally_ = true;
    }
    else if (position.x + rect_->getSize().x >= levelBounds.x)
    {
        position.x = levelBounds.x - rect_->getSize().x;
        velocityX_ = -std::abs(velocityX_) * 0.2f;
        collidedHorizontally_ = true;
    }

    rect_->setPosition(position);

    if (!wasOnGround && isOnGround_ && kind_ == Kind::VoidSlime && state_ != State::Attack)
    {
        spawnPatrolEffect();
    }

    if (state_ == State::Die && deathSmokeClock_.getElapsedTime().asMilliseconds() >= 110.f)
    {
        deathSmokeClock_.restart();
        spawnDeathSmoke();
    }

    if (state_ == State::Die && stateClock_.getElapsedTime().asMilliseconds() >= deathDurationMs_)
    {
        if (!hasDroppedGold_ && enemyManager_)
        {
            enemyManager_->dropGold(getCenterPosition(), getDropKey());
            hasDroppedGold_ = true;
        }
        isAlive = false;
    }

    syncSpriteToRect();
}

void BestiaryEnemy::checkGroundCollision()
{
    const float bottom = rect_->getPosition().y + rect_->getSize().y;
    const float groundTop = ground_->getRect().getPosition().y;

    if (bottom >= groundTop)
    {
        rect_->setPosition({rect_->getPosition().x, groundTop - rect_->getSize().y});
        velocityY_ = 0.f;
        isOnGround_ = true;
    }
}

void BestiaryEnemy::checkPlatformCollision()
{
    for (auto& rectPtr : platform_->getRects())
    {
        const sf::FloatRect enemyBounds = rect_->getGlobalBounds();
        const sf::FloatRect platformBounds = rectPtr->getGlobalBounds();

        if (!enemyBounds.findIntersection(platformBounds))
        {
            continue;
        }

        const float overlapLeft = enemyBounds.position.x + enemyBounds.size.x - platformBounds.position.x;
        const float overlapRight = platformBounds.position.x + platformBounds.size.x - enemyBounds.position.x;
        const float overlapTop = enemyBounds.position.y + enemyBounds.size.y - platformBounds.position.y;
        const float overlapBottom = platformBounds.position.y + platformBounds.size.y - enemyBounds.position.y;

        const bool fromLeft = overlapLeft < overlapRight;
        const bool fromTop = overlapTop < overlapBottom;
        const float minXOverlap = fromLeft ? overlapLeft : overlapRight;
        const float minYOverlap = fromTop ? overlapTop : overlapBottom;

        if (minXOverlap < minYOverlap)
        {
            if (fromLeft)
            {
                rect_->setPosition({platformBounds.position.x - enemyBounds.size.x, enemyBounds.position.y});
            }
            else
            {
                rect_->setPosition({platformBounds.position.x + platformBounds.size.x, enemyBounds.position.y});
            }

            velocityX_ = 0.f;
            collidedHorizontally_ = true;
        }
        else
        {
            if (fromTop)
            {
                rect_->setPosition({enemyBounds.position.x, platformBounds.position.y - enemyBounds.size.y});
                velocityY_ = 0.f;
                isOnGround_ = true;
            }
            else
            {
                rect_->setPosition({enemyBounds.position.x, platformBounds.position.y + platformBounds.size.y});
                velocityY_ = std::max(0.f, velocityY_);
            }
        }
    }
}

bool BestiaryEnemy::hasGroundAhead(float direction) const
{
    if (!rect_ || !ground_ || !platform_)
    {
        return false;
    }

    const sf::FloatRect enemyBounds = rect_->getGlobalBounds();
    const float sampleX = direction > 0.f
        ? enemyBounds.position.x + enemyBounds.size.x + 10.f
        : enemyBounds.position.x - 10.f;
    const float footY = enemyBounds.position.y + enemyBounds.size.y + 4.f;

    const sf::FloatRect groundBounds = ground_->getRect().getGlobalBounds();
    if (sampleX >= groundBounds.position.x &&
        sampleX <= groundBounds.position.x + groundBounds.size.x &&
        footY >= groundBounds.position.y - 6.f)
    {
        return true;
    }

    for (auto& platformRect : platform_->getRects())
    {
        const sf::FloatRect platformBounds = platformRect->getGlobalBounds();
        const bool horizontallyInside = sampleX >= platformBounds.position.x + 4.f &&
            sampleX <= platformBounds.position.x + platformBounds.size.x - 4.f;
        const float bottom = enemyBounds.position.y + enemyBounds.size.y;
        const bool sameHeightBand = std::abs(bottom - platformBounds.position.y) <= 22.f;
        if (horizontallyInside && sameHeightBand)
        {
            return true;
        }
    }

    return false;
}

void BestiaryEnemy::checkBulletCollision()
{
    for (auto bulletIt = player_->bullets.begin(); bulletIt != player_->bullets.end(); ++bulletIt)
    {
        Bullet& bullet = *(*bulletIt);
        if (!bullet.isSheduledToBeDestroyed &&
            bullet.canHitTarget(this) &&
            bullet.getBulletRect().getGlobalBounds().findIntersection(rect_->getGlobalBounds()))
        {
            bullet.registerHitTarget(this);
            receiveBulletHit(bullet, false);

            if (bullet.getSplashRadius() > 0.f && enemyManager_)
            {
                enemyManager_->applySplashDamage(
                    bullet.getCenterPosition(),
                    bullet.getSplashRadius(),
                    bullet,
                    this
                );
            }
        }
    }
}

void BestiaryEnemy::receiveBulletHit(const Bullet& bullet, bool splashHit)
{
    if (!isAlive || state_ == State::Die)
    {
        return;
    }

    const int damage = splashHit
        ? std::max(1, static_cast<int>(std::round(static_cast<float>(bullet.getDamage()) * 0.72f)))
        : bullet.getDamage();
    HP_ -= damage;

    const sf::Color impactColor = bullet.getConfig().impactColor;
    spawnParticleBurst(
        getCenterPosition(),
        splashHit ? sf::Color(impactColor.r, impactColor.g, impactColor.b, 160) : impactColor,
        splashHit ? 5 : 8,
        splashHit ? 36.f : 52.f,
        splashHit ? 92.f : 146.f,
        splashHit ? 2.2f : 2.8f,
        splashHit ? 18.f : 28.f,
        splashHit ? 0.36f : 0.48f
    );
    pushRing(
        getCenterPosition(),
        sf::Color(impactColor.r, impactColor.g, impactColor.b, splashHit ? 120 : 170),
        splashHit ? 10.f : 12.f,
        splashHit ? 30.f : 40.f,
        splashHit ? 2.4f : 3.f,
        splashHit ? 2.f : 2.6f,
        splashHit ? 120.f : 170.f
    );
    spawnHitEffect(impactColor, splashHit);

    if (HP_ <= 0)
    {
        HP_ = 0;
        setState(State::Die);
        return;
    }

    if (kind_ == Kind::DreadScorpion)
    {
        playerDetected_ = true;
        aggroClock_.restart();
        return;
    }

    setState(State::Hurt);
    if (kind_ != Kind::WraithBat)
    {
        velocityX_ = 0.f;
    }
}

void BestiaryEnemy::damagePlayerOnContact()
{
    if (attackDamageApplied_ || !player_)
    {
        return;
    }

    sf::FloatRect hitBounds = rect_->getGlobalBounds();
    if (kind_ == Kind::DreadScorpion && (state_ == State::Windup || state_ == State::Attack))
    {
        const bool strikeRight = lockedAttackFacingRight_;
        const sf::Vector2f center = getCenterPosition();
        const float strikeWidth = rect_->getSize().x + 12.f;
        const float strikeHeight = rect_->getSize().y + 18.f;
        hitBounds.size = {strikeWidth, strikeHeight};
        hitBounds.position.y = rect_->getPosition().y - 6.f;
        hitBounds.position.x = strikeRight
            ? center.x + 4.f
            : center.x - strikeWidth - 4.f;
    }
    else if (kind_ == Kind::WraithBat && state_ == State::Attack)
    {
        hitBounds.position.x -= 8.f;
        hitBounds.position.y -= 8.f;
        hitBounds.size.x += 16.f;
        hitBounds.size.y += 16.f;
    }
    else if (kind_ == Kind::VoidSlime && state_ == State::Attack)
    {
        hitBounds.position.x -= 4.f;
        hitBounds.size.x += 8.f;
        hitBounds.size.y += 6.f;
    }

    if (!hitBounds.findIntersection(player_->playerRectangle_->getGlobalBounds()))
    {
        return;
    }

    const bool hitFromRight = getCenterPosition().x > player_->playerRectangle_->getGlobalBounds().getCenter().x;
    if (player_->takeDMG(DMG_, knockback_, hitFromRight))
    {
        attackDamageApplied_ = true;
        spawnImpactEffect();
    }
}

void BestiaryEnemy::updateTextures()
{
    if (!isAlive)
    {
        return;
    }

    portal_->updateTextures();
    if (!portal_->getIsHalfPassed())
    {
        return;
    }

    sprite_->setColor(baseTint_);

    if (kind_ == Kind::DreadScorpion && hitFlashActive_)
    {
        const float progress = static_cast<float>(hitFlashClock_.getElapsedTime().asMilliseconds()) / std::max(1.f, hitFlashDurationMs_);
        if (progress >= 1.f)
        {
            hitFlashActive_ = false;
        }
        else
        {
            const float flicker = 0.8f + std::sin(progress * kPi * 8.f) * 0.2f;
            const float intensity = std::clamp((1.f - progress) * flicker, 0.f, 1.f);
            const sf::Color flashColor(232, 255, 136, 255);
            const sf::Color baseColor = sprite_->getColor();
            sprite_->setColor(sf::Color(
                static_cast<std::uint8_t>(baseColor.r + (flashColor.r - baseColor.r) * intensity),
                static_cast<std::uint8_t>(baseColor.g + (flashColor.g - baseColor.g) * intensity),
                static_cast<std::uint8_t>(baseColor.b + (flashColor.b - baseColor.b) * intensity),
                baseColor.a
            ));
        }
    }

    if (state_ == State::Die)
    {
        pulseSprite(*sprite_, sf::Color(255, 120, 120, 255), 2.1f, sf::seconds(0.35f));
        if (kind_ == Kind::DreadScorpion)
        {
            switchToNextSprite(sprite_.get(), *attackTextures_, attackHelper_, switchSprite_SwitchOption::Loop);
        }
        else
        {
            switchToNextSprite(sprite_.get(), *moveTextures_, moveHelper_, switchSprite_SwitchOption::Loop);
        }

        const float fade = std::clamp(
            1.f - static_cast<float>(stateClock_.getElapsedTime().asMilliseconds()) / std::max(1.f, deathDurationMs_),
            0.f,
            1.f
        );
        const sf::Color tinted = sprite_->getColor();
        sprite_->setColor(sf::Color(tinted.r, tinted.g, tinted.b, static_cast<std::uint8_t>(fade * 255.f)));
        syncSpriteToRect();
        return;
    }

    if (state_ == State::Hurt)
    {
        pulseSprite(*sprite_, sf::Color(255, 86, 92, 255), 2.4f, sf::seconds(0.2f));
    }

    if (kind_ == Kind::WraithBat)
    {
        if (state_ == State::Windup || state_ == State::Attack || state_ == State::Recover)
        {
            switchToNextSprite(sprite_.get(), *attackTextures_, attackHelper_, switchSprite_SwitchOption::Loop);
        }
        else
        {
            switchToNextSprite(sprite_.get(), *moveTextures_, moveHelper_, switchSprite_SwitchOption::Loop);
        }
    }
    else if (kind_ == Kind::VoidSlime)
    {
        if (state_ == State::Patrol && isOnGround_)
        {
            switchToNextSprite(sprite_.get(), *idleTextures_, idleHelper_, switchSprite_SwitchOption::Loop);
        }
        else
        {
            switchToNextSprite(sprite_.get(), *attackTextures_, attackHelper_, switchSprite_SwitchOption::Loop);
        }
    }
    else
    {
        if (state_ == State::Windup)
        {
            sprite_->setTexture(attackTextures_->front());
        }
        else if (state_ == State::Attack)
        {
            const bool continues = switchToNextSprite(sprite_.get(), *attackTextures_, attackHelper_, switchSprite_SwitchOption::Single);
            const bool isStrikeFrame = &sprite_->getTexture() == &attackTextures_->back();
            if (!attackDamageApplied_ && isStrikeFrame)
            {
                damagePlayerOnContact();
            }
            if (!continues)
            {
                setState(State::Recover);
            }
        }
        else
        {
            switchToNextSprite(sprite_.get(), *moveTextures_, moveHelper_, switchSprite_SwitchOption::Loop);
        }
    }

    const float visualScaleY = (kind_ == Kind::VoidSlime && state_ == State::Windup)
        ? enemyScale_.y * 0.84f
        : enemyScale_.y;
    float visualScaleX = getFacingSign() * enemyScale_.x;
    if (kind_ == Kind::DreadScorpion)
    {
        visualScaleX *= -1.f;
    }
    sprite_->setScale({visualScaleX, visualScaleY});
    syncSpriteToRect();
}

void BestiaryEnemy::syncSpriteToRect()
{
    if (!sprite_ || !rect_)
    {
        return;
    }

    setSpriteOriginToMiddle(*sprite_);
    const sf::Vector2f center = rect_->getGlobalBounds().getCenter();
    float targetY = center.y + spriteOffsetY_;
    if (kind_ != Kind::WraithBat)
    {
        const sf::Texture& texture = sprite_->getTexture();
        const float scaledHeight = static_cast<float>(texture.getSize().y) * std::abs(sprite_->getScale().y);
        targetY = rect_->getPosition().y + rect_->getSize().y - scaledHeight * 0.5f - spriteOffsetY_;
    }

    sprite_->setPosition({center.x, targetY});
}

void BestiaryEnemy::updateVisualEffects()
{
    if (kind_ == Kind::DreadScorpion && state_ == State::Die)
    {
        triggerScorpionDeathBursts();
    }

    for (auto& particle : effectParticles_)
    {
        particle.update();
    }

    effectParticles_.erase(
        std::remove_if(effectParticles_.begin(), effectParticles_.end(), [](const Particle& particle) {
            return !particle.getIsAlive();
        }),
        effectParticles_.end()
    );

    for (auto& ring : effectRings_)
    {
        ring.radius += ring.growth;
        ring.alpha = std::max(0.f, ring.alpha - ring.growth * 3.2f);
    }

    effectRings_.erase(
        std::remove_if(effectRings_.begin(), effectRings_.end(), [](const VisualRing& ring) {
            return ring.radius >= ring.maxRadius || ring.alpha <= 1.f;
        }),
        effectRings_.end()
    );
}

void BestiaryEnemy::drawVisualEffects()
{
    const sf::Vector2f center = getCenterPosition();
    if (state_ != State::Die && kind_ == Kind::DreadScorpion)
    {
        sf::CircleShape shadow(22.f);
        shadow.setOrigin({shadow.getRadius(), shadow.getRadius()});
        shadow.setScale({1.45f, 0.34f});
        shadow.setPosition({center.x, rect_->getPosition().y + rect_->getSize().y - 11.f});
        shadow.setFillColor(sf::Color(46, 56, 34, 22));
        window_->draw(shadow);
    }

    if (kind_ == Kind::DreadScorpion && state_ == State::Die)
    {
        drawScorpionDeathAura();
    }
    else if (kind_ == Kind::DreadScorpion && hitFlashActive_)
    {
        drawScorpionHitAura();
    }

    for (const auto& ring : effectRings_)
    {
        sf::CircleShape circle(ring.radius);
        circle.setOrigin({circle.getRadius(), circle.getRadius()});
        circle.setPosition(ring.position);
        circle.setFillColor(sf::Color::Transparent);
        circle.setOutlineThickness(ring.thickness);
        circle.setOutlineColor(sf::Color(ring.color.r, ring.color.g, ring.color.b, static_cast<std::uint8_t>(ring.alpha)));
        window_->draw(circle);
    }

    for (const auto& particle : effectParticles_)
    {
        particle.draw(*window_);
    }
}

void BestiaryEnemy::drawScorpionHitAura()
{
    const float progress = static_cast<float>(hitFlashClock_.getElapsedTime().asMilliseconds()) / std::max(1.f, hitFlashDurationMs_);
    if (progress >= 1.f)
    {
        return;
    }

    const float intensity = std::clamp(1.f - progress, 0.f, 1.f);
    const sf::Vector2f center = getCenterPosition();
    const float direction = getFacingSign();
    const float shimmer = std::sin(progress * kPi * 9.f);
    const sf::Vector2f shellPulsePos = {
        center.x - direction * (14.f - intensity * 5.f),
        center.y - 18.f + shimmer * 2.5f
    };
    const sf::Vector2f stingPulsePos = {
        center.x + direction * (rect_->getSize().x * 0.4f + intensity * 12.f),
        center.y - 26.f + shimmer * 1.8f
    };

    sf::CircleShape shellBloom(18.f + intensity * 14.f);
    shellBloom.setOrigin({shellBloom.getRadius(), shellBloom.getRadius()});
    shellBloom.setScale({1.55f, 0.74f});
    shellBloom.setPosition(shellPulsePos);
    shellBloom.setFillColor(sf::Color(218, 255, 118, static_cast<std::uint8_t>(38.f + intensity * 108.f)));
    window_->draw(shellBloom);

    sf::CircleShape shellCore(10.f + intensity * 7.f);
    shellCore.setOrigin({shellCore.getRadius(), shellCore.getRadius()});
    shellCore.setScale({1.25f, 0.7f});
    shellCore.setPosition({shellPulsePos.x - direction * 4.f, shellPulsePos.y - 1.f});
    shellCore.setFillColor(sf::Color(244, 255, 182, static_cast<std::uint8_t>(64.f + intensity * 156.f)));
    window_->draw(shellCore);

    sf::CircleShape stingGlow(9.f + intensity * 9.f);
    stingGlow.setOrigin({stingGlow.getRadius(), stingGlow.getRadius()});
    stingGlow.setScale({1.2f, 1.05f});
    stingGlow.setPosition(stingPulsePos);
    stingGlow.setFillColor(sf::Color(186, 255, 96, static_cast<std::uint8_t>(46.f + intensity * 146.f)));
    window_->draw(stingGlow);

    sf::RectangleShape stingSpark({22.f + intensity * 18.f, 5.f + intensity * 5.f});
    stingSpark.setOrigin({stingSpark.getSize().x * 0.12f, stingSpark.getSize().y * 0.5f});
    stingSpark.setPosition(stingPulsePos);
    stingSpark.setRotation(sf::degrees(direction > 0.f ? -18.f + shimmer * 6.f : 198.f - shimmer * 6.f));
    stingSpark.setFillColor(sf::Color(235, 255, 170, static_cast<std::uint8_t>(58.f + intensity * 168.f)));
    window_->draw(stingSpark);

    sf::CircleShape acidWake(28.f + intensity * 16.f);
    acidWake.setOrigin({acidWake.getRadius(), acidWake.getRadius()});
    acidWake.setScale({1.22f, 0.42f});
    acidWake.setPosition({center.x - direction * 6.f, rect_->getPosition().y + rect_->getSize().y - 7.f});
    acidWake.setFillColor(sf::Color::Transparent);
    acidWake.setOutlineThickness(2.f + intensity * 1.4f);
    acidWake.setOutlineColor(sf::Color(164, 255, 90, static_cast<std::uint8_t>(54.f + intensity * 134.f)));
    window_->draw(acidWake);
}

void BestiaryEnemy::drawScorpionDeathAura()
{
    const float progress = std::clamp(
        static_cast<float>(stateClock_.getElapsedTime().asMilliseconds()) / std::max(1.f, deathDurationMs_),
        0.f,
        1.f
    );
    const float fade = 1.f - progress;
    const sf::Vector2f center = getCenterPosition();
    const float direction = getFacingSign();
    const float oscillation = std::sin(progress * kPi * 6.f);

    sf::CircleShape acidPool(30.f + progress * 28.f);
    acidPool.setOrigin({acidPool.getRadius(), acidPool.getRadius()});
    acidPool.setScale({1.36f, 0.46f});
    acidPool.setPosition({center.x - direction * 4.f, rect_->getPosition().y + rect_->getSize().y - 7.f});
    acidPool.setFillColor(sf::Color(80, 255, 92, static_cast<std::uint8_t>(24.f + fade * 80.f)));
    window_->draw(acidPool);

    sf::CircleShape acidRing(40.f + progress * 36.f);
    acidRing.setOrigin({acidRing.getRadius(), acidRing.getRadius()});
    acidRing.setScale({1.22f, 0.42f});
    acidRing.setPosition({center.x - direction * 2.f, rect_->getPosition().y + rect_->getSize().y - 7.f});
    acidRing.setFillColor(sf::Color::Transparent);
    acidRing.setOutlineThickness(2.8f + fade * 1.4f);
    acidRing.setOutlineColor(sf::Color(170, 255, 108, static_cast<std::uint8_t>(92.f + fade * 126.f)));
    window_->draw(acidRing);

    sf::CircleShape backHalo(20.f + progress * 18.f);
    backHalo.setOrigin({backHalo.getRadius(), backHalo.getRadius()});
    backHalo.setScale({1.58f, 0.7f});
    backHalo.setPosition({center.x - direction * 16.f, center.y - 20.f + oscillation * 2.f});
    backHalo.setFillColor(sf::Color(226, 255, 138, static_cast<std::uint8_t>(34.f + fade * 120.f)));
    window_->draw(backHalo);

    sf::CircleShape stingNova(12.f + progress * 14.f);
    stingNova.setOrigin({stingNova.getRadius(), stingNova.getRadius()});
    stingNova.setScale({1.28f, 1.f});
    stingNova.setPosition({center.x + direction * (rect_->getSize().x * 0.44f + progress * 14.f), center.y - 26.f + oscillation * 1.5f});
    stingNova.setFillColor(sf::Color(196, 255, 104, static_cast<std::uint8_t>(56.f + fade * 156.f)));
    window_->draw(stingNova);

    sf::RectangleShape stingFlare({26.f + progress * 34.f, 7.f + progress * 6.f});
    stingFlare.setOrigin({stingFlare.getSize().x * 0.08f, stingFlare.getSize().y * 0.5f});
    stingFlare.setPosition({center.x + direction * (rect_->getSize().x * 0.38f), center.y - 22.f});
    stingFlare.setRotation(sf::degrees(direction > 0.f ? -16.f + oscillation * 7.f : 196.f - oscillation * 7.f));
    stingFlare.setFillColor(sf::Color(240, 255, 182, static_cast<std::uint8_t>(38.f + fade * 138.f)));
    window_->draw(stingFlare);
}

void BestiaryEnemy::triggerScorpionDeathBursts()
{
    const float progress = std::clamp(
        static_cast<float>(stateClock_.getElapsedTime().asMilliseconds()) / std::max(1.f, deathDurationMs_),
        0.f,
        1.f
    );
    const sf::Vector2f center = getCenterPosition();

    if (!scorpionDeathPulseMidPlayed_ && progress >= 0.3f)
    {
        scorpionDeathPulseMidPlayed_ = true;
        pushRing({center.x - getFacingSign() * 12.f, center.y - 16.f}, sf::Color(238, 255, 188, 225), 12.f, 56.f, 4.f, 2.8f, 225.f);
        spawnParticleBurst({center.x - getFacingSign() * 10.f, center.y - 18.f}, sf::Color(206, 255, 124, 220), 10, 56.f, 164.f, 2.8f, 6.f, 0.58f);
        spawnParticleBurst({center.x, center.y + 2.f}, sf::Color(132, 212, 78, 180), 5, 18.f, 74.f, 2.6f, -10.f, 0.52f);
    }

    if (!scorpionDeathPulseLatePlayed_ && progress >= 0.66f)
    {
        scorpionDeathPulseLatePlayed_ = true;
        pushRing({center.x + getFacingSign() * 16.f, center.y - 24.f}, sf::Color(184, 255, 102, 205), 10.f, 48.f, 3.8f, 2.6f, 205.f);
        pushRing({center.x, rect_->getPosition().y + rect_->getSize().y - 6.f}, sf::Color(112, 255, 90, 185), 16.f, 64.f, 3.6f, 2.4f, 185.f);
        spawnParticleBurst({center.x + random(-6.f, 6.f), rect_->getPosition().y + rect_->getSize().y - 4.f}, sf::Color(96, 196, 78, 180), 8, 26.f, 94.f, 3.2f, -14.f, 0.68f);
        spawnParticleBurst({center.x + getFacingSign() * 18.f, center.y - 20.f}, sf::Color(230, 255, 170, 180), 6, 32.f, 96.f, 2.5f, 10.f, 0.48f);
    }
}

void BestiaryEnemy::spawnParticleBurst(const sf::Vector2f& origin,
                                       const sf::Color& color,
                                       int count,
                                       float minSpeed,
                                       float maxSpeed,
                                       float radius,
                                       float gravity,
                                       float lifetime)
{
    for (int index = 0; index < count; ++index)
    {
        const float angle = random(0.f, 360.f) * kPi / 180.f;
        const float speed = random(minSpeed, maxSpeed);
        const sf::Vector2f velocity = {
            std::cos(angle) * speed,
            std::sin(angle) * speed
        };

        effectParticles_.emplace_back(
            origin,
            velocity,
            sf::Vector2f{random(-35.f, 35.f), random(-24.f, 24.f)},
            color,
            radius,
            gravity,
            0.88f,
            lifetime
        );
    }
}

void BestiaryEnemy::pushRing(const sf::Vector2f& position,
                             const sf::Color& color,
                             float radius,
                             float maxRadius,
                             float growth,
                             float thickness,
                             float alpha)
{
    effectRings_.push_back(VisualRing{
        .position = position,
        .color = color,
        .radius = radius,
        .maxRadius = maxRadius,
        .growth = growth,
        .thickness = thickness,
        .alpha = alpha
    });
}

void BestiaryEnemy::spawnNoticeEffect()
{
    const sf::Color accent = kind_ == Kind::WraithBat
        ? sf::Color(186, 118, 228, 210)
        : (kind_ == Kind::VoidSlime ? sf::Color(130, 92, 218, 205) : sf::Color(182, 214, 132, 205));
    pushRing(getCenterPosition(), accent, 10.f, 44.f, 3.2f, 2.4f, 190.f);
    spawnParticleBurst(getCenterPosition(), accent, 8, 40.f, 120.f, 2.4f, 18.f, 0.46f);
}

void BestiaryEnemy::spawnHitEffect(const sf::Color& impactColor, bool splashHit)
{
    if (kind_ != Kind::DreadScorpion)
    {
        return;
    }

    hitFlashActive_ = true;
    hitFlashClock_.restart();

    const sf::Vector2f center = getCenterPosition();
    const sf::Color paleImpact(
        std::min(255, impactColor.r + 86),
        std::min(255, impactColor.g + 120),
        std::min(255, impactColor.b + 24),
        splashHit ? 192 : 236
    );
    const sf::Color toxicAccent(136, 255, 82, splashHit ? 160 : 214);
    const float direction = getFacingSign();
    const sf::Vector2f burstOrigin = {
        center.x - direction * (8.f - random(-3.f, 3.f)),
        center.y - random(8.f, 18.f)
    };
    const sf::Vector2f stingOrigin = {
        center.x + direction * (rect_->getSize().x * 0.38f),
        center.y - 22.f
    };

    pushRing(burstOrigin, paleImpact, splashHit ? 12.f : 16.f, splashHit ? 42.f : 60.f, 3.7f, 2.8f, splashHit ? 176.f : 224.f);
    pushRing(stingOrigin, toxicAccent, splashHit ? 10.f : 14.f, splashHit ? 32.f : 46.f, 3.f, 2.2f, splashHit ? 150.f : 196.f);
    spawnParticleBurst(burstOrigin, paleImpact, splashHit ? 7 : 12, 44.f, splashHit ? 128.f : 194.f, 2.6f, 18.f, 0.46f);
    spawnParticleBurst(stingOrigin, toxicAccent, splashHit ? 4 : 7, 28.f, splashHit ? 88.f : 138.f, 2.4f, 10.f, 0.42f);
    spawnParticleBurst({center.x, rect_->getPosition().y + rect_->getSize().y - 4.f}, sf::Color(110, 238, 84, splashHit ? 128 : 170), splashHit ? 3 : 6, 18.f, 72.f, 2.4f, -12.f, 0.42f);
}

void BestiaryEnemy::spawnAttackEffect()
{
    const sf::Vector2f center = getCenterPosition();
    if (kind_ == Kind::DreadScorpion)
    {
        const float direction = lockedAttackFacingRight_ ? 1.f : -1.f;
        const sf::Vector2f strikeOrigin = {
            center.x + direction * (rect_->getSize().x * 0.45f),
            center.y - 6.f
        };
        pushRing(strikeOrigin, sf::Color(196, 255, 110, 230), 12.f, 54.f, 3.8f, 2.8f, 230.f);
        pushRing({center.x - direction * 12.f, center.y - 10.f}, sf::Color(140, 242, 92, 165), 10.f, 28.f, 2.6f, 2.f, 165.f);
        spawnParticleBurst(strikeOrigin, sf::Color(228, 255, 170, 230), 11, 66.f, 198.f, 2.8f, 18.f, 0.54f);
        spawnParticleBurst({strikeOrigin.x, strikeOrigin.y + 10.f}, sf::Color(120, 220, 92, 180), 6, 28.f, 96.f, 2.4f, -6.f, 0.46f);
        return;
    }

    const sf::Color accent = kind_ == Kind::WraithBat
        ? sf::Color(226, 156, 255, 214)
        : (kind_ == Kind::VoidSlime ? sf::Color(156, 116, 255, 220) : sf::Color(192, 222, 142, 220));
    pushRing(center, accent, 10.f, kind_ == Kind::WraithBat ? 58.f : 42.f, 3.6f, 2.4f, 200.f);
    spawnParticleBurst(center, accent, kind_ == Kind::WraithBat ? 10 : 7, 52.f, 176.f, 2.6f, 22.f, 0.52f);
}

void BestiaryEnemy::spawnImpactEffect()
{
    const sf::Vector2f center = getCenterPosition();
    if (kind_ == Kind::DreadScorpion)
    {
        const float direction = lockedAttackFacingRight_ ? 1.f : -1.f;
        const sf::Vector2f impactPoint = {
            center.x + direction * (rect_->getSize().x * 0.52f),
            center.y - 4.f
        };
        pushRing(impactPoint, sf::Color(240, 255, 174, 235), 10.f, 46.f, 4.f, 2.6f, 220.f);
        pushRing({impactPoint.x, impactPoint.y + 10.f}, sf::Color(138, 246, 98, 188), 8.f, 30.f, 2.8f, 2.f, 188.f);
        spawnParticleBurst(impactPoint, sf::Color(238, 255, 190, 230), 12, 94.f, 226.f, 2.6f, 20.f, 0.46f);
        spawnParticleBurst({impactPoint.x, impactPoint.y + 8.f}, sf::Color(126, 218, 88, 190), 7, 32.f, 116.f, 2.2f, -4.f, 0.44f);
        return;
    }

    const sf::Vector2f impactPoint = {
        center.x + getFacingSign() * (rect_->getSize().x * 0.45f),
        center.y - 4.f
    };
    const sf::Color accent = kind_ == Kind::WraithBat
        ? sf::Color(255, 218, 250, 220)
        : (kind_ == Kind::VoidSlime ? sf::Color(204, 172, 255, 220) : sf::Color(224, 255, 190, 220));
    pushRing(impactPoint, accent, 8.f, 34.f, 3.4f, 2.f, 180.f);
    spawnParticleBurst(impactPoint, accent, 7, 74.f, 188.f, 2.3f, 20.f, 0.4f);
}

void BestiaryEnemy::spawnDeathEffect()
{
    if (kind_ == Kind::DreadScorpion)
    {
        const sf::Vector2f center = getCenterPosition();
        const float direction = getFacingSign();
        pushRing({center.x - direction * 10.f, center.y - 18.f}, sf::Color(222, 255, 140, 240), 16.f, 92.f, 5.f, 3.4f, 240.f);
        pushRing({center.x + direction * 18.f, center.y - 24.f}, sf::Color(184, 255, 98, 214), 12.f, 58.f, 4.f, 2.8f, 214.f);
        pushRing({center.x, rect_->getPosition().y + rect_->getSize().y - 6.f}, sf::Color(94, 248, 92, 188), 20.f, 74.f, 3.8f, 2.8f, 188.f);
        spawnParticleBurst({center.x - direction * 8.f, center.y - 12.f}, sf::Color(236, 255, 176, 228), 26, 108.f, 268.f, 3.3f, 18.f, 1.02f);
        spawnParticleBurst({center.x + direction * 16.f, center.y - 20.f}, sf::Color(148, 255, 96, 210), 18, 42.f, 154.f, 2.9f, -4.f, 0.9f);
        spawnParticleBurst({center.x, rect_->getPosition().y + rect_->getSize().y - 4.f}, sf::Color(96, 190, 80, 180), 12, 28.f, 96.f, 3.5f, -16.f, 0.94f);
        deathSmokeClock_.restart();
        return;
    }

    const sf::Color accent = kind_ == Kind::WraithBat
        ? sf::Color(178, 116, 228, 225)
        : (kind_ == Kind::VoidSlime ? sf::Color(110, 70, 202, 225) : sf::Color(174, 210, 128, 225));
    pushRing(getCenterPosition(), accent, 12.f, 68.f, 4.1f, 3.f, 220.f);
    spawnParticleBurst(getCenterPosition(), accent, 16, 80.f, 210.f, 3.2f, 24.f, 0.85f);
    deathSmokeClock_.restart();
}

void BestiaryEnemy::spawnPatrolEffect()
{
    const sf::Vector2f center = getCenterPosition();
    if (kind_ == Kind::WraithBat)
    {
        spawnParticleBurst(
            {center.x - getFacingSign() * 14.f, center.y + random(-8.f, 8.f)},
            sf::Color(152, 106, 210, 120),
            2,
            16.f,
            52.f,
            1.8f,
            -4.f,
            0.42f
        );
    }
    else if (kind_ == Kind::VoidSlime)
    {
        spawnParticleBurst(
            {center.x + random(-8.f, 8.f), rect_->getPosition().y + rect_->getSize().y - 2.f},
            sf::Color(104, 72, 188, 105),
            3,
            12.f,
            44.f,
            2.1f,
            -8.f,
            0.45f
        );
    }
    else
    {
        spawnParticleBurst(
            {center.x + random(-12.f, 12.f), rect_->getPosition().y + rect_->getSize().y - 1.f},
            sf::Color(142, 168, 114, 95),
            2,
            18.f,
            42.f,
            1.9f,
            -8.f,
            0.45f
        );
    }
}

void BestiaryEnemy::spawnLandingEffect()
{
    const sf::Vector2f center = getCenterPosition();
    pushRing({center.x, rect_->getPosition().y + rect_->getSize().y - 4.f}, sf::Color(144, 98, 228, 155), 8.f, 38.f, 2.8f, 2.2f, 155.f);
    spawnParticleBurst(
        {center.x, rect_->getPosition().y + rect_->getSize().y - 2.f},
        sf::Color(114, 80, 206, 135),
        6,
        22.f,
        96.f,
        2.2f,
        -12.f,
        0.5f
    );
}

void BestiaryEnemy::spawnDeathSmoke()
{
    const sf::Color smokeColor = kind_ == Kind::WraithBat
        ? sf::Color(88, 38, 122, 170)
        : (kind_ == Kind::VoidSlime ? sf::Color(68, 34, 120, 170) : sf::Color(78, 92, 52, 160));
    spawnParticleBurst(
        {getCenterPosition().x + random(-8.f, 8.f), getCenterPosition().y - random(4.f, 12.f)},
        smokeColor,
        kind_ == Kind::DreadScorpion ? 3 : 2,
        10.f,
        kind_ == Kind::DreadScorpion ? 54.f : 38.f,
        kind_ == Kind::DreadScorpion ? 4.2f : 3.6f,
        kind_ == Kind::DreadScorpion ? -6.f : -10.f,
        kind_ == Kind::DreadScorpion ? 0.82f : 0.7f
    );
}

void BestiaryEnemy::draw()
{
    if (portal_->getIsHalfPassed())
    {
        drawVisualEffects();
        window_->draw(*sprite_);
        healthBar_->draw(state_ != State::Die);
    }

    if (portal_->getIsExist())
    {
        portal_->draw(*window_);
    }
}

sf::RectangleShape& BestiaryEnemy::getRect()
{
    return *rect_;
}

sf::Vector2f BestiaryEnemy::getPosition() const
{
    return rect_ ? rect_->getPosition() : spawnPosition_;
}

void BestiaryEnemy::attachPlayer(Player& player)
{
    player_ = &player;
}

std::string BestiaryEnemy::getDropKey() const
{
    return typeKey_;
}
