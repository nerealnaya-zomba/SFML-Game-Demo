#include <Skeleton.h>
#include <CollisionUtils.h>
#include "Ground.h"    
#include "Platform.h"  
#include "Player.h"
#include <EnemyManager.h>
#include<GameLevel.h>

namespace
{
constexpr float kPi = 3.14159265f;
}

using namespace gameUtils;

// ========== ДВИЖЕНИЕ ==========
void Skeleton::walkLeft() {
    if (initialWalkSpeed <= -maxWalkSpeed || isPlayingHurtAnimation) return; 
    initialWalkSpeed -= speed;
}

void Skeleton::walkRight() {
    if (initialWalkSpeed >= maxWalkSpeed || isPlayingHurtAnimation) return;
    initialWalkSpeed += speed;
}

// ========== ПРЕСЛЕДОВАНИЕ ИГРОКА ==========
void Skeleton::chasePlayer(sf::Vector2f skeletonPos, sf::Vector2f playerPos) {
    if (std::abs(skeletonPos.x - playerPos.x) < 8.f) {
        action_ = IDLE;
    } else if (skeletonPos.x < playerPos.x) {
        action_ = WALKRIGHT;
    } else if (skeletonPos.x > playerPos.x) {
        action_ = WALKLEFT;
    }
}

// ========== ПАТРУЛИРОВАНИЕ ==========
void Skeleton::patrol() {
    currentSkeletonPos = skeletonRect->getGlobalBounds().getCenter().x;
    
    // Первый вход в патрулирование
    if (isFirstEnter) {
        isFirstEnter = false;
        leftExplored = false;
        rightExplored = false;
        
        // Случайный выбор начального направления
        if (rand() % 2 == 0) {
            explorationState = EXPLORE_LEFT;
            action_ = WALKLEFT;
        } else {
            explorationState = EXPLORE_RIGHT;
            action_ = WALKRIGHT;
        }
        
        exploreStartPos = currentSkeletonPos;
        deadEndCheckTimer.restart();
        return;
    }
    
    // Фаза исследования
    if (explorationState != PATROLLING) {
        // Проверка на тупик
        if (deadEndCheckTimer.getElapsedTime().asMilliseconds() >= TIME_TO_CHECK_DEADEND) {
            float distanceMoved = std::abs(currentSkeletonPos - exploreStartPos);
            
            if (distanceMoved < MIN_DISTANCE_FOR_DEADEND) {
                // Обнаружен тупик
                if (explorationState == EXPLORE_LEFT && !leftExplored) {
                    leftBound = currentSkeletonPos;
                    leftExplored = true;
                } else if (explorationState == EXPLORE_RIGHT && !rightExplored) {
                    rightBound = currentSkeletonPos;
                    rightExplored = true;
                }
                
                // Если обе стороны исследованы
                if (leftExplored && rightExplored) {
                    explorationState = PATROLLING;
                    // Выбор начального направления патрулирования
                    action_ = (currentSkeletonPos > (leftBound + rightBound) / 2.0f) ? WALKLEFT : WALKRIGHT;
                } else {
                    // Исследование другой стороны
                    explorationState = !leftExplored ? EXPLORE_LEFT : EXPLORE_RIGHT;
                    action_ = (explorationState == EXPLORE_LEFT) ? WALKLEFT : WALKRIGHT;
                    
                    exploreStartPos = currentSkeletonPos;
                    deadEndCheckTimer.restart();
                }
            } else {
                // Продолжаем движение
                exploreStartPos = currentSkeletonPos;
                deadEndCheckTimer.restart();
            }
        }
    } 
    // Фаза патрулирования между границами
    else {
        if (isPatrolPaused) {
            action_ = IDLE;
            if (patrolPauseTimer.getElapsedTime().asMilliseconds() >= PATROL_EDGE_PAUSE) {
                isPatrolPaused = false;
                action_ = pendingPatrolAction_;
                directionSwitchTimer.restart();
                recentlySwitchedDirection = true;
            }
            return;
        }

        if (!recentlySwitchedDirection) {
            // Проверка достижения границ
            if (action_ == WALKLEFT && currentSkeletonPos <= leftBound + DIRECTION_SWITCH_OFFSET) {
                pendingPatrolAction_ = WALKRIGHT;
                isPatrolPaused = true;
                patrolPauseTimer.restart();
                pushRing(
                    getCenterPosition(),
                    sf::Color(136, 162, 176, 110),
                    14.f,
                    42.f,
                    2.1f,
                    2.f,
                    110.f
                );
                return;
            } else if (action_ == WALKRIGHT && currentSkeletonPos >= rightBound - DIRECTION_SWITCH_OFFSET) {
                pendingPatrolAction_ = WALKLEFT;
                isPatrolPaused = true;
                patrolPauseTimer.restart();
                pushRing(
                    getCenterPosition(),
                    sf::Color(136, 162, 176, 110),
                    14.f,
                    42.f,
                    2.1f,
                    2.f,
                    110.f
                );
                return;
            }
        }
        
        // Сброс флага смены направления
        if (recentlySwitchedDirection && 
            directionSwitchTimer.getElapsedTime().asMilliseconds() >= DIRECTION_SWITCH_COOLDOWN) {
            recentlySwitchedDirection = false;
        }
    }
}

// ========== СБРОС ПЕРЕМЕННЫХ ПАТРУЛИРОВАНИЯ ==========
void Skeleton::makeRandomPatrolVariables() {
    if (makeRandomStart != isPlayerOutOfReach) {
        makeRandomStart = isPlayerOutOfReach;
        isFirstEnter = true;
        isPatrolPaused = false;
        explorationState = EXPLORE_NONE;
        leftExplored = false;
        rightExplored = false;
        leftBound = 0.0f;
        rightBound = 0.0f;
    }
}

void Skeleton::resetAllThatHeKnows()
{
    leftExplored               = false;
    rightExplored              = false;
    recentlySwitchedDirection  = false;
    isFirstEnter               = true;
    isPatrolPaused             = false;
    makeRandomStart            = false;
    leftBound                  = 0.0f;
    rightBound                 = 0.0f;
    explorationState           = ExplorationState::EXPLORE_NONE;
}

void Skeleton::clearAggroState()
{
    hasDetectedPlayer = false;
    hasLastKnownPlayerPos = false;
    awarenessState_ = SkeletonAwarenessState::Patrol;
    action_ = IDLE;
    initialWalkSpeed = 0.f;
    isPlayerOutOfReach = true;
}

bool Skeleton::isPlayerBeyondBlockedChaseBoundary(float playerX) const
{
    if (hasBlockedChaseLeft_ && playerX <= blockedChaseLeftX_ - BLOCKED_CHASE_RELEASE_MARGIN) {
        return true;
    }
    if (hasBlockedChaseRight_ && playerX >= blockedChaseRightX_ + BLOCKED_CHASE_RELEASE_MARGIN) {
        return true;
    }

    return false;
}

void Skeleton::updateBlockedChaseBoundaries(float playerX)
{
    if (hasBlockedChaseLeft_ && playerX >= blockedChaseLeftX_ + BLOCKED_CHASE_RELEASE_MARGIN) {
        hasBlockedChaseLeft_ = false;
        blockedChaseLeftX_ = 0.f;
    }

    if (hasBlockedChaseRight_ && playerX <= blockedChaseRightX_ - BLOCKED_CHASE_RELEASE_MARGIN) {
        hasBlockedChaseRight_ = false;
        blockedChaseRightX_ = 0.f;
    }
}

void Skeleton::registerBlockedChaseBoundary(bool blockedLeft)
{
    const float deadEndX = getCenterPosition().x;

    if (blockedLeft) {
        hasBlockedChaseLeft_ = true;
        blockedChaseLeftX_ = deadEndX;
    } else {
        hasBlockedChaseRight_ = true;
        blockedChaseRightX_ = deadEndX;
    }

    pushRing(
        getCenterPosition(),
        sf::Color(104, 138, 154, 140),
        12.f,
        46.f,
        2.6f,
        2.2f,
        140.f
    );
    spawnParticleBurst(
        {getCenterPosition().x, getCenterPosition().y + 6.f},
        sf::Color(124, 150, 164, 140),
        5,
        24.f,
        86.f,
        2.2f,
        -10.f,
        0.46f
    );

    clearAggroState();
}

// ========== АТАКА ИГРОКА ==========
void Skeleton::tryAttackPlayer() {
    sf::Vector2f skeletonPos = skeletonRect->getGlobalBounds().getCenter();
    sf::Vector2f playerPos = player_->playerRectangle_->getGlobalBounds().getCenter();
    float distanceX = std::abs(skeletonPos.x - playerPos.x);
    float distanceY = std::abs(skeletonPos.y - playerPos.y);

    if (distanceX < distanceToHit_byAttack && distanceY < skeletonRect->getSize().y) {
        bool hitSide = (skeletonPos.x > playerPos.x);
        player_->takeDMG(DMG_, knockback_, hitSide);
    }
}

void Skeleton::beginAttack(skeletonAction attackAction)
{
    action_ = attackAction;
    attackDamageApplied = false;

    texturesIterHelper* helper = attackAction == ATTACK1 ? &skeleton_attack1_helper : &skeleton_attack2_helper;
    helper->ptrToTexture = 0;
    helper->iterationCounter = 0;
    helper->goForward = true;

    spawnAttackEffect(attackAction == ATTACK2);
}

// ========== ОБРАБОТКА ПОПАДАНИЯ ПУЛИ ==========
void Skeleton::onBulletHit(const Bullet& bullet, bool splashHit) {
    isPlayingHurtAnimation = true;
    action_ = HURT;
    
    // Сброс анимации
    if (skeleton_hurt_helper.ptrToTexture == skeleton_hurt_helper.countOfTextures) {
        skeleton_hurt_helper.ptrToTexture = 0;
        skeleton_hurt_helper.iterationCounter = 0;
    }
    
    // Сброс скорости при отталкивании
    if (knockbacks) initialWalkSpeed = 0.f;
    
    // Уменьшение здоровья
    const int damage = splashHit
        ? std::max(1, static_cast<int>(std::round(static_cast<float>(bullet.getDamage()) * 0.7f)))
        : bullet.getDamage();
    HP_ -= damage;

    const sf::Color impactColor = bullet.getConfig().impactColor;
    const sf::Color ringColor = splashHit
        ? sf::Color(impactColor.r, impactColor.g, impactColor.b, 120)
        : sf::Color(impactColor.r, impactColor.g, impactColor.b, 170);

    spawnParticleBurst(
        getCenterPosition(),
        splashHit ? sf::Color(impactColor.r, impactColor.g, impactColor.b, 160) : impactColor,
        splashHit ? 5 : 7,
        splashHit ? 45.f : 70.f,
        splashHit ? 120.f : 165.f,
        splashHit ? 2.2f : 2.8f,
        splashHit ? 18.f : 34.f,
        splashHit ? 0.35f : 0.45f
    );
    pushRing(
        getCenterPosition(),
        ringColor,
        splashHit ? 10.f : 12.f,
        splashHit ? 28.f : 38.f,
        splashHit ? 2.6f : 3.2f,
        splashHit ? 2.f : 2.5f,
        splashHit ? 120.f : 170.f
    );
}

// ========== КОЛЛИЗИИ С ЗЕМЛЕЙ ==========
void Skeleton::checkGroundCollision(Ground &ground) {
    float skeletonX = skeletonRect->getPosition().x;
    float skeletonBottom = skeletonRect->getPosition().y + skeletonRect->getSize().y;
    float groundTop = ground.getRect().getPosition().y;

    if (skeletonBottom >= groundTop) {
        isFalling = false;
        fallingSpeed = 0.f;
        skeletonRect->setPosition({skeletonX, groundTop - skeletonRect->getSize().y});
    }
}

// ========== КОЛЛИЗИИ С ПЛАТФОРМАМИ ==========
void Skeleton::checkPlatformCollision(Platform& platforms) {
    for (auto& rectPtr : platforms.getRects()) {
        sf::FloatRect skeletonBounds = skeletonRect->getGlobalBounds();
        sf::FloatRect platformBounds = rectPtr->getGlobalBounds();

        if (skeletonBounds.findIntersection(platformBounds)) {
            // Расчет перекрытий
            float overlapLeft = skeletonBounds.position.x + skeletonBounds.size.x - platformBounds.position.x;
            float overlapRight = platformBounds.position.x + platformBounds.size.x - skeletonBounds.position.x;
            float overlapTop = skeletonBounds.position.y + skeletonBounds.size.y - platformBounds.position.y;
            float overlapBottom = platformBounds.position.y + platformBounds.size.y - skeletonBounds.position.y;

            bool fromLeft = (overlapLeft < overlapRight);
            bool fromTop = (overlapTop < overlapBottom);
            float minXOverlap = fromLeft ? overlapLeft : overlapRight;
            float minYOverlap = fromTop ? overlapTop : overlapBottom;

            // Боковые коллизии
            if (minXOverlap < minYOverlap) {
                if (fromLeft) {
                    skeletonRect->setPosition({platformBounds.position.x - skeletonBounds.size.x, skeletonBounds.position.y});
                } else {
                    skeletonRect->setPosition({platformBounds.position.x + platformBounds.size.x, skeletonBounds.position.y});
                }

                const bool blockedWhileChasingRight = hasDetectedPlayer && action_ == WALKRIGHT && fromLeft;
                const bool blockedWhileChasingLeft = hasDetectedPlayer && action_ == WALKLEFT && !fromLeft;
                if (blockedWhileChasingRight || blockedWhileChasingLeft) {
                    registerBlockedChaseBoundary(blockedWhileChasingLeft);
                }
            } 
            // Вертикальные коллизии
            else {
                if (fromTop) {
                    isFalling = false;
                    if (fallingSpeed > 0.f) fallingSpeed = 0.f;
                    
                    if (fallingSpeed >= -0.1f && fallingSpeed <= 0.1f) {
                        if (skeletonBounds.position.y + skeletonBounds.size.y >= platformBounds.position.y + 3.f) {
                            skeletonRect->setPosition({skeletonBounds.position.x, skeletonBounds.position.y - 2.f});
                        }
                    }
                } else {
                    skeletonRect->setPosition({skeletonBounds.position.x, platformBounds.position.y + platformBounds.size.y});
                    fallingSpeed = -(fallingSpeed);
                }
            }
        }
    }
}

// ========== КОЛЛИЗИИ С ПУЛЯМИ ==========
void Skeleton::checkBulletCollision(Player& player) {
    for (auto it = player_->bullets.begin(); it != player_->bullets.end(); ++it) {
        Bullet& bullet = *(*it);
        if (bullet.getBulletRect().getGlobalBounds().findIntersection(skeletonRect->getGlobalBounds()) &&
            !bullet.isSheduledToBeDestroyed &&
            bullet.canHitTarget(this)) {
            bullet.registerHitTarget(this);
            receiveBulletHit(bullet);

            if (bullet.getSplashRadius() > 0.f && enemyManager)
            {
                enemyManager->applySplashDamage(
                    bullet.getCenterPosition(),
                    bullet.getSplashRadius(),
                    bullet,
                    this
                );
            }
        }
    }
}

// ========== ТРЕНИЕ ==========
void Skeleton::applyFriction(float &walkSpeed, float friction) {
    if (walkSpeed > 0.f) {
        walkSpeed = std::max(0.f, walkSpeed - friction);
    } else if (walkSpeed < 0.f) {
        walkSpeed = std::min(0.f, walkSpeed + friction);
    }
    
    if (std::abs(walkSpeed) < friction * 0.5f) {
        walkSpeed = 0.f;
    }
}

// ========== ЗАГРУЗКА ДАННЫХ ==========
void Skeleton::loadData() {
    const nlohmann::json& j = gameData->getEnemySettings();
    
    enemyScale_ = sf::Vector2f(j["general"]["scaleX"], j["general"]["scaleY"]);
    distanceToMakeAttack = j["general"]["distanceToMakeAttack"];
    distanceToHit_byAttack = j["general"]["distanceToHit_byAttack"];
    
    if (type_ == "white") {
        maxWalkSpeed = random(1.5f, 3.0f);
        speed = j["skeleton-white"]["acceleration"];
        frictionForce = j["skeleton-white"]["friction"];
        HP_ = j["skeleton-white"]["HP"];
        DMG_ = j["skeleton-white"]["DMG"];
        knockback_ = sf::Vector2f(j["skeleton-white"]["KnockbackX"], j["skeleton-white"]["KnockbackY"]);
        knockbacks = j["skeleton-white"]["knockbacks"];
    } else if (type_ == "yellow") {
        maxWalkSpeed = j["skeleton-yellow"]["maxSpeed"];
        speed = j["skeleton-yellow"]["acceleration"];
        frictionForce = j["skeleton-yellow"]["friction"];
        HP_ = j["skeleton-yellow"]["HP"];
        DMG_ = j["skeleton-yellow"]["DMG"];
        knockback_ = {j["skeleton-yellow"]["KnockbackX"], j["skeleton-white"]["KnockbackY"]};
        knockbacks = j["skeleton-yellow"]["knockbacks"];
    }

    baseMaxWalkSpeed = maxWalkSpeed;
    alertDistance_ = distanceToMakeAttack * 3.0f;
    loseAggroDistance_ = alertDistance_ * 1.45f;
    verticalAlertTolerance_ = std::max(145.f, distanceToMakeAttack * 1.35f);
    verticalAttackTolerance_ = std::max(skeletonRect ? skeletonRect->getSize().y * 1.1f : 62.f, 68.f);
}

sf::Vector2f Skeleton::getCenterPosition() const
{
    return skeletonRect ? skeletonRect->getGlobalBounds().getCenter() : enemyPos;
}

float Skeleton::getFacingDirection() const
{
    return (!skeletonSprite || skeletonSprite->getScale().x >= 0.f) ? 1.f : -1.f;
}

void Skeleton::pushRing(
    const sf::Vector2f& position,
    const sf::Color& color,
    float radius,
    float maxRadius,
    float growth,
    float thickness,
    float alpha
)
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

void Skeleton::spawnParticleBurst(
    const sf::Vector2f& origin,
    const sf::Color& color,
    int count,
    float minSpeed,
    float maxSpeed,
    float radius,
    float gravity,
    float lifetime
)
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
            sf::Vector2f{random(-50.f, 50.f), random(-35.f, 35.f)},
            color,
            radius,
            gravity,
            0.88f,
            lifetime
        );
    }
}

void Skeleton::spawnPatrolEffect()
{
    const sf::Vector2f center = getCenterPosition();
    const sf::Vector2f footOrigin = {
        center.x + random(-8.f, 8.f),
        skeletonRect->getPosition().y + skeletonRect->getSize().y - 3.f
    };

    spawnParticleBurst(
        footOrigin,
        sf::Color(134, 154, 170, 95),
        2,
        18.f,
        42.f,
        2.1f,
        -8.f,
        0.6f
    );
}

void Skeleton::spawnNoticeEffect()
{
    const sf::Vector2f center = getCenterPosition();

    pushRing(
        {center.x, center.y - 12.f},
        type_ == "yellow" ? sf::Color(255, 208, 146, 210) : sf::Color(220, 100, 86, 210),
        12.f,
        56.f,
        3.8f,
        3.f,
        210.f
    );
    pushRing(
        {center.x, center.y - 20.f},
        sf::Color(255, 232, 194, 155),
        4.f,
        28.f,
        2.6f,
        2.f,
        155.f
    );
    spawnParticleBurst(
        {center.x, center.y - 14.f},
        type_ == "yellow" ? sf::Color(255, 210, 126, 215) : sf::Color(234, 96, 83, 215),
        type_ == "yellow" ? 10 : 8,
        65.f,
        160.f,
        2.6f,
        22.f,
        0.55f
    );
    alertPulseClock.restart();
}

void Skeleton::spawnAttackEffect(bool heavyAttack)
{
    const sf::Vector2f center = getCenterPosition();
    const float direction = player_->playerRectangle_->getGlobalBounds().getCenter().x >= center.x ? 1.f : -1.f;
    const sf::Vector2f swingOrigin = {center.x + direction * 18.f, center.y - 12.f};
    const sf::Color effectColor = heavyAttack
        ? sf::Color(255, 198, 108, 215)
        : sf::Color(220, 88, 74, 210);

    pushRing(
        swingOrigin,
        effectColor,
        10.f,
        heavyAttack ? 64.f : 50.f,
        heavyAttack ? 4.f : 3.2f,
        heavyAttack ? 3.f : 2.4f,
        185.f
    );
    spawnParticleBurst(
        swingOrigin,
        effectColor,
        heavyAttack ? 10 : 7,
        90.f,
        heavyAttack ? 210.f : 165.f,
        heavyAttack ? 3.1f : 2.5f,
        18.f,
        heavyAttack ? 0.65f : 0.5f
    );
}

void Skeleton::spawnAttackImpactEffect()
{
    const sf::Vector2f center = getCenterPosition();
    const float direction = getFacingDirection();
    const sf::Vector2f impactPoint = {
        center.x + direction * (26.f + enemyScale_.x * 8.f),
        center.y - 8.f
    };

    pushRing(
        impactPoint,
        sf::Color(255, 220, 164, 175),
        8.f,
        42.f,
        3.8f,
        2.f,
        175.f
    );
    spawnParticleBurst(
        impactPoint,
        type_ == "yellow" ? sf::Color(255, 222, 155, 225) : sf::Color(245, 128, 102, 220),
        8,
        85.f,
        205.f,
        2.4f,
        26.f,
        0.42f
    );
}

void Skeleton::spawnDeathEffect()
{
    const sf::Vector2f center = getCenterPosition();
    const sf::Color mainColor = type_ == "yellow"
        ? sf::Color(255, 195, 116, 230)
        : sf::Color(196, 60, 67, 230);

    pushRing(center, mainColor, 12.f, 74.f, 4.5f, 3.5f, 220.f);
    pushRing(center, sf::Color(255, 236, 188, 150), 6.f, 36.f, 2.8f, 2.f, 150.f);
    spawnParticleBurst(center, mainColor, type_ == "yellow" ? 18 : 14, 90.f, 220.f, 3.3f, 32.f, 0.9f);
    spawnParticleBurst(center, sf::Color(54, 18, 22, 215), 10, 48.f, 132.f, 4.4f, -9.f, 1.1f);
    deathSmokeClock.restart();
}

void Skeleton::updateVisualEffects()
{
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
        ring.alpha = std::max(0.f, ring.alpha - ring.growth * 3.6f);
    }

    effectRings_.erase(
        std::remove_if(effectRings_.begin(), effectRings_.end(), [](const VisualRing& ring) {
            return ring.radius >= ring.maxRadius || ring.alpha <= 3.f;
        }),
        effectRings_.end()
    );
}

void Skeleton::drawAttackTelegraph()
{
    if (action_ != ATTACK1 && action_ != ATTACK2)
    {
        return;
    }

    const texturesIterHelper& helper = action_ == ATTACK1 ? skeleton_attack1_helper : skeleton_attack2_helper;
    const int textureCount = std::max(1, helper.countOfTextures + 1);
    const float progress = static_cast<float>(helper.ptrToTexture) / static_cast<float>(textureCount);
    const float intensity = std::sin(std::clamp(progress, 0.f, 1.f) * kPi);
    if (intensity <= 0.04f)
    {
        return;
    }

    const float direction = getFacingDirection();
    const sf::Vector2f center = getCenterPosition();
    const sf::Color slashColor = type_ == "yellow"
        ? sf::Color(255, 212, 130, static_cast<std::uint8_t>(90.f + intensity * 85.f))
        : sf::Color(214, 88, 74, static_cast<std::uint8_t>(78.f + intensity * 92.f));

    sf::RectangleShape slashGlow({52.f + intensity * 34.f, 18.f + intensity * 7.f});
    slashGlow.setOrigin({slashGlow.getSize().x * 0.25f, slashGlow.getSize().y / 2.f});
    slashGlow.setPosition({center.x + direction * (24.f + intensity * 10.f), center.y - 10.f});
    slashGlow.setRotation(sf::degrees(direction > 0.f ? -18.f : 198.f));
    slashGlow.setFillColor(sf::Color(slashColor.r, slashColor.g, slashColor.b, static_cast<std::uint8_t>(slashColor.a * 0.45f)));
    window->draw(slashGlow);

    sf::RectangleShape slashCore({36.f + intensity * 22.f, 7.f + intensity * 4.f});
    slashCore.setOrigin({slashCore.getSize().x * 0.22f, slashCore.getSize().y / 2.f});
    slashCore.setPosition(slashGlow.getPosition());
    slashCore.setRotation(slashGlow.getRotation());
    slashCore.setFillColor(slashColor);
    window->draw(slashCore);
}

void Skeleton::drawVisualEffects()
{
    for (const auto& ring : effectRings_)
    {
        sf::CircleShape circle(ring.radius);
        circle.setOrigin({circle.getRadius(), circle.getRadius()});
        circle.setPosition(ring.position);
        circle.setFillColor(sf::Color::Transparent);
        circle.setOutlineThickness(ring.thickness);
        circle.setOutlineColor(sf::Color(ring.color.r, ring.color.g, ring.color.b, static_cast<std::uint8_t>(ring.alpha)));
        window->draw(circle);
    }

    for (const auto& particle : effectParticles_)
    {
        particle.draw(*window);
    }

    drawAttackTelegraph();
}

// ========== КОНСТРУКТОР ==========
Skeleton::Skeleton(GameData &gameData, EnemyManager& em, GameLevel& gl, sf::RenderWindow &window, Ground& ground, 
             Platform& platform, Player& player, std::string type, sf::Vector2f pos) 
    : Enemy(gameData) {
    
    this->window = &window;
    this->ground_ = &ground;
    this->platform_ = &platform;
    this->player_ = &player;
    this->gameData = &gameData;
    this->enemyManager = &em;
    this->type_ = type;
    this->enemyPos = pos;
    this->portal = std::make_unique<enemyPortal>(gameData,pos);
    this->gameLevel = &gl;
    
    loadData();

    std::transform(type.begin(), type.end(), type.begin(), ::tolower);
    
    // Загрузка текстур в зависимости от типа
    if (type == "white") {
        attachTexture(gameData.skeletonWhite_idleTextures_, skeleton_idleTextures, 
                     gameData.skeletonWhite_idle_helper, skeleton_idle_helper);
        attachTexture(gameData.skeletonWhite_walkTextures, skeleton_walkTextures, 
                     gameData.skeletonWhite_walk_helper, skeleton_walk_helper);
        attachTexture(gameData.skeletonWhite_hurtTextures, skeleton_hurtTextures, 
                     gameData.skeletonWhite_hurt_helper, skeleton_hurt_helper);
        attachTexture(gameData.skeletonWhite_dieTextures, skeleton_dieTextures, 
                     gameData.skeletonWhite_die_helper, skeleton_die_helper);
        attachTexture(gameData.skeletonWhite_attack1Textures, skeleton_attack1Textures, 
                     gameData.skeletonWhite_attack1_helper, skeleton_attack1_helper);
        attachTexture(gameData.skeletonWhite_attack2Textures, skeleton_attack2Textures, 
                     gameData.skeletonWhite_attack2_helper, skeleton_attack2_helper);
        
        skeletonSprite = std::make_unique<sf::Sprite>(skeleton_idleTextures->at(0));
    } else if (type == "yellow") {
        attachTexture(gameData.skeletonYellow_idleTextures, skeleton_idleTextures, 
                     gameData.skeletonYellow_idle_helper, skeleton_idle_helper);
        attachTexture(gameData.skeletonYellow_walkTextures, skeleton_walkTextures, 
                     gameData.skeletonYellow_walk_helper, skeleton_walk_helper);
        attachTexture(gameData.skeletonYellow_hurtTextures, skeleton_hurtTextures, 
                     gameData.skeletonYellow_hurt_helper, skeleton_hurt_helper);
        attachTexture(gameData.skeletonYellow_dieTextures, skeleton_dieTextures, 
                     gameData.skeletonYellow_die_helper, skeleton_die_helper);
        attachTexture(gameData.skeletonYellow_attack1Textures, skeleton_attack1Textures, 
                     gameData.skeletonYellow_attack1_helper, skeleton_attack1_helper);
        attachTexture(gameData.skeletonYellow_attack2Textures, skeleton_attack2Textures, 
                     gameData.skeletonYellow_attack2_helper, skeleton_attack2_helper);
        
        skeletonSprite = std::make_unique<sf::Sprite>(skeleton_idleTextures->at(0));
    }
    
    // Инициализация спрайта
    skeletonSprite->setScale(enemyScale_);
    setSpriteOriginToMiddle(*skeletonSprite);

    // Инициализация хитбокса
    skeletonRect = std::make_unique<sf::RectangleShape>();
    float sizeX = 23.f * enemyScale_.x;
    float sizeY = 47.f * enemyScale_.y; 
    skeletonRect->setSize({sizeX, sizeY});
    skeletonRect->setFillColor(sf::Color::Red);
    skeletonRect->setPosition(enemyPos);
    verticalAttackTolerance_ = std::max(skeletonRect->getSize().y * 1.15f, 72.f);
    
    sf::Vector2f rectCenter = skeletonRect->getGlobalBounds().getCenter();
    skeletonSprite->setPosition({rectCenter.x, rectCenter.y - 15.f});
    portal->setPosition(rectCenter);

    // Полоска здоровья
    healthbar = std::make_unique<HealthBar>(skeletonRect.get(), window, sf::Color::Red, sf::Color::Green,
                             sf::Vector2f{50.f, 5.f}, HP_, sf::Vector2f{0.f, -50.f});

    attackCooldownClock.restart();
    aggroMemoryClock.restart();
    patrolEffectClock.restart();
    deathSmokeClock.restart();
    alertPulseClock.restart();
}

// ========== ДЕСТРУКТОР ==========
Skeleton::~Skeleton() = default;

// ========== ОБНОВЛЕНИЕ ИИ ==========
void Skeleton::updateAI() {
    if (!isAlive || !portal->getIsHalfPassed()) {
        return;
    }

    const sf::Vector2f skeletonPos = getCenterPosition();
    const sf::Vector2f playerPos = player_->playerRectangle_->getGlobalBounds().getCenter();
    const float distanceX = std::abs(skeletonPos.x - playerPos.x);
    const float distanceY = std::abs(skeletonPos.y - playerPos.y);
    updateBlockedChaseBoundaries(playerPos.x);

    const bool playerBeyondBlockedBoundary = isPlayerBeyondBlockedChaseBoundary(playerPos.x);
    const bool canNoticePlayer = distanceX <= alertDistance_ &&
        distanceY <= verticalAlertTolerance_ &&
        !playerBeyondBlockedBoundary;
    const bool canAttackPlayer = distanceX <= distanceToMakeAttack &&
        distanceY <= verticalAttackTolerance_ &&
        !playerBeyondBlockedBoundary;

    if (playerBeyondBlockedBoundary && hasDetectedPlayer) {
        clearAggroState();
    }

    if (canNoticePlayer) {
        lastKnownPlayerPos = playerPos;
        hasLastKnownPlayerPos = true;
        aggroMemoryClock.restart();

        if (!hasDetectedPlayer) {
            hasDetectedPlayer = true;
            awarenessState_ = SkeletonAwarenessState::Alert;
            spawnNoticeEffect();
        }
    }

    maxWalkSpeed = hasDetectedPlayer ? baseMaxWalkSpeed * ALERT_SPEED_MULTIPLIER : baseMaxWalkSpeed;

    if (isPlayingDieAnimation || isPlayingHurtAnimation) {
        return;
    }

    if (action_ == ATTACK1 || action_ == ATTACK2) {
        deadEndCheckTimer.restart();
        directionSwitchTimer.restart();
        blackoutTimer.restart();
        return;
    }

    const bool hasAggroMemory = hasLastKnownPlayerPos &&
        aggroMemoryClock.getElapsedTime().asMilliseconds() < AGGRO_MEMORY_MS;
    const bool lostPlayerCompletely = distanceX > loseAggroDistance_ || distanceY > verticalAlertTolerance_ * 1.6f;

    if (hasDetectedPlayer) {
        if (canAttackPlayer &&
            attackCooldownClock.getElapsedTime().asMilliseconds() >= ATTACK_COOLDOWN_MS) {
            beginAttack((rand() % 2 == 0) ? ATTACK1 : ATTACK2);
        } else if (canNoticePlayer) {
            awarenessState_ = SkeletonAwarenessState::Alert;
            chasePlayer(skeletonPos, playerPos);
        } else if (hasAggroMemory && !lostPlayerCompletely) {
            awarenessState_ = SkeletonAwarenessState::Search;
            chasePlayer(skeletonPos, lastKnownPlayerPos);
        } else {
            clearAggroState();
            makeRandomPatrolVariables();
        }
    }

    if (!hasDetectedPlayer) {
        awarenessState_ = SkeletonAwarenessState::Patrol;
        isPlayerOutOfReach = true;
        makeRandomPatrolVariables();
        patrol();
    } else {
        isPlayerOutOfReach = false;
    }

    if (awarenessState_ == SkeletonAwarenessState::Patrol &&
        (action_ == WALKLEFT || action_ == WALKRIGHT) &&
        !isPatrolPaused &&
        patrolEffectClock.getElapsedTime().asMilliseconds() >= PATROL_EFFECT_INTERVAL_MS) {
        patrolEffectClock.restart();
        spawnPatrolEffect();
    }

    if (action_ == ATTACK1 || action_ == ATTACK2 || leftExplored == false || rightExplored == false || hasDetectedPlayer) {
        AFKPastPosUpdateTimer.restart();
        AFKTimeTimer.reset();
    }

    afk_current_pos = skeletonRect->getGlobalBounds().getCenter().x;
    if (checkInterval(AFKPastPosUpdateTimer, AFK_BEFORE_UPDATE_TIME) || !AFKPastPosUpdateTimer.isRunning())
    {
        AFKPastPosUpdateTimer.restart();
        afk_past_pos = skeletonRect->getGlobalBounds().getCenter().x;
    }

    if (std::abs(afk_current_pos - afk_past_pos) < afk_detect_difference && awarenessState_ == SkeletonAwarenessState::Patrol) {
        AFKTimeTimer.start();
        if (AFKTimeTimer.getElapsedTime().asMilliseconds() > MAX_AFK_TIME) {
            resetAllThatHeKnows();
        }
    } else {
        AFKTimeTimer.reset();
    }

    if (action_ == ATTACK1 || action_ == ATTACK2 || leftExplored == false || rightExplored == false || hasDetectedPlayer) {
        blackoutTimer.restart();
    }
    if (checkInterval(blackoutTimer, timeToResetALLThatHeKnows)) {
        resetAllThatHeKnows();
    }
}

// ========== УПРАВЛЕНИЕ ==========
void Skeleton::updateControl() {
    if(!portal->getIsHalfPassed()) return;
    
    switch (action_) {
        case WALKLEFT:
            walkLeft();
            break;
        case WALKRIGHT:
            walkRight();
            break;
        // TODO: Добавить прыжки и другие действия
    }
}

// ========== ФИЗИКА ==========
void Skeleton::updatePhysics() {
    portal->update();
    updateVisualEffects();
    if(!portal->getIsHalfPassed()) return;

    applyFriction(initialWalkSpeed, frictionForce);

    sf::Vector2f levelBounds = static_cast<sf::Vector2f>(gameLevel->getLevelSize());
    const collision::MoveResult moveResult = collision::moveBodyWithWorldCollisions(
        *skeletonRect,
        {initialWalkSpeed, fallingSpeed},
        platform_->getRects(),
        &ground_->getRect(),
        levelBounds.x
    );

    if (moveResult.blockedLeft || moveResult.blockedRight) {
        initialWalkSpeed = 0.f;

        const bool blockedWhileChasingRight = hasDetectedPlayer && action_ == WALKRIGHT && moveResult.blockedRight;
        const bool blockedWhileChasingLeft = hasDetectedPlayer && action_ == WALKLEFT && moveResult.blockedLeft;
        if (blockedWhileChasingRight || blockedWhileChasingLeft) {
            registerBlockedChaseBoundary(blockedWhileChasingLeft);
        }
    }

    if (moveResult.hitCeiling && fallingSpeed < 0.f) {
        fallingSpeed = 0.f;
    }

    const bool standingOnGround = collision::isStandingOnGround(*skeletonRect, ground_->getRect());
    const bool standingOnPlatform = collision::findSupportingPlatform(*skeletonRect, platform_->getRects()) != nullptr;
    isFalling = !(moveResult.landed || standingOnGround || standingOnPlatform);

    if (isFalling) {
        fallingSpeed += 0.1f;
    } else {
        fallingSpeed = 0.f;
    }
    
    if (HP_ <= 0) {
        isPlayingDieAnimation = true;
    }

    if (isPlayingDieAnimation && deathSmokeClock.getElapsedTime().asMilliseconds() >= 130.f) {
        deathSmokeClock.restart();
        spawnParticleBurst(
            {getCenterPosition().x + random(-8.f, 8.f), getCenterPosition().y - random(6.f, 14.f)},
            type_ == "yellow" ? sf::Color(255, 196, 120, 150) : sf::Color(98, 22, 26, 170),
            2,
            18.f,
            56.f,
            type_ == "yellow" ? 3.2f : 4.2f,
            -14.f,
            0.65f
        );
    }

    if (HP_ > 0) checkBulletCollision(*player_);
    
    healthbar->update(HP_);
}

// ========== АНИМАЦИИ ==========
void Skeleton::updateTextures() {
    if (!isAlive) return;
    portal->updateTextures();
    if(!portal->getIsHalfPassed()) return;

    // Анимация получения урона
    if (isPlayingHurtAnimation && HP_ > 0) {
        pulseSprite(*skeletonSprite, sf::Color(255, 82, 76, 255), 1.8f, sf::seconds(0.2f));
        if (!switchToNextSprite(skeletonSprite.get(), *skeleton_hurtTextures, 
            skeleton_hurt_helper, switchSprite_SwitchOption::Single)) {
            isPlayingHurtAnimation = false;
            skeletonSprite->setColor({255, 255, 255, 255});
            action_ = IDLE;
        }
        
        sf::Vector2f rectCenter = skeletonRect->getGlobalBounds().getCenter();
        skeletonSprite->setPosition({rectCenter.x, rectCenter.y - 15.f});
        return;
    }
    
    // Анимация смерти
    if (isPlayingDieAnimation) {   
        if (!deathEffectPlayed) {
            spawnDeathEffect();
            deathEffectPlayed = true;
        }

        pulseSprite(
            *skeletonSprite,
            type_ == "yellow" ? sf::Color(255, 196, 124, 255) : sf::Color(214, 62, 70, 255),
            2.2f,
            sf::seconds(0.35f)
        );
        if (!switchToNextSprite(skeletonSprite.get(), *skeleton_dieTextures, 
            skeleton_die_helper, switchSprite_SwitchOption::Single)) {
            if(!hasDroppedGold && enemyManager)
            {
                enemyManager->dropGold(skeletonRect->getGlobalBounds().getCenter(), type_);
                hasDroppedGold = true;
            }
            isPlayingDieAnimation = false;
            isAlive = false;
        }
        
        sf::Vector2f rectCenter = skeletonRect->getGlobalBounds().getCenter();
        skeletonSprite->setPosition({rectCenter.x, rectCenter.y - 15.f});
        return;
    }

    // Анимации атаки
    if (action_ == ATTACK1 || action_ == ATTACK2) {
        bool attackFinished = false;

        // Направление взгляда в сторону игрока
        if (player_->playerRectangle_->getPosition().x > skeletonRect->getPosition().x) {
            skeletonSprite->setScale({enemyScale_.x, enemyScale_.y});
        } else {
            skeletonSprite->setScale({-enemyScale_.x, enemyScale_.y});
        } 

        if (action_ == ATTACK1) {
            attackFinished = !switchToNextSprite(skeletonSprite.get(), *skeleton_attack1Textures,
                            skeleton_attack1_helper, switchSprite_SwitchOption::Single);
            
            // Нанесение урона в середине анимации
            if (!attackDamageApplied && skeleton_attack1_helper.ptrToTexture >= 5) {
                attackDamageApplied = true;
                spawnAttackImpactEffect();
                tryAttackPlayer();
            }
        } else { 
            attackFinished = !switchToNextSprite(skeletonSprite.get(), *skeleton_attack2Textures,
                            skeleton_attack2_helper, switchSprite_SwitchOption::Single);
            
            // Нанесение урона в середине анимации
            if (!attackDamageApplied && skeleton_attack2_helper.ptrToTexture >= 4) {
                attackDamageApplied = true;
                spawnAttackImpactEffect();
                tryAttackPlayer();
            }
        }

        pulseSprite(
            *skeletonSprite,
            type_ == "yellow" ? sf::Color(255, 212, 132, 255) : sf::Color(208, 84, 70, 255),
            1.2f,
            sf::seconds(0.28f)
        );
        
        // Возврат в состояние покоя
        if (attackFinished) {
            action_ = IDLE;
            attackCooldownClock.restart();
            attackDamageApplied = false;
            skeleton_attack1_helper.ptrToTexture = 0;
            skeleton_attack1_helper.iterationCounter = 0;
            skeleton_attack2_helper.ptrToTexture = 0;
            skeleton_attack2_helper.iterationCounter = 0;
        }
        
        sf::Vector2f rectCenter = skeletonRect->getGlobalBounds().getCenter();
        skeletonSprite->setPosition({rectCenter.x, rectCenter.y - 15.f});
        return;
    } else{
        skeleton_attack1_helper.ptrToTexture = 0;
        skeleton_attack1_helper.iterationCounter = 0;
        skeleton_attack2_helper.ptrToTexture = 0;
        skeleton_attack2_helper.iterationCounter = 0;
        attackDamageApplied = false;
    }

    skeletonSprite->setColor({255, 255, 255, 255});

    // Обычные анимации
    switch (action_) {
        case IDLE:
            switchToNextSprite(skeletonSprite.get(), *skeleton_idleTextures, 
                            skeleton_idle_helper, switchSprite_SwitchOption::Loop);
            break;

        case WALKRIGHT:
            skeletonSprite->setScale({enemyScale_.x, enemyScale_.y});
            switchToNextSprite(skeletonSprite.get(), *skeleton_walkTextures,     
                            skeleton_walk_helper, switchSprite_SwitchOption::Loop);   
            break;

        case WALKLEFT:
            skeletonSprite->setScale({-enemyScale_.x, enemyScale_.y});
            switchToNextSprite(skeletonSprite.get(), *skeleton_walkTextures,
                            skeleton_walk_helper, switchSprite_SwitchOption::Loop);
            break;
    }

    // Позиционирование спрайта
    sf::Vector2f rectCenter = skeletonRect->getGlobalBounds().getCenter();
    skeletonSprite->setPosition({rectCenter.x, rectCenter.y - 15.f});
}

// ========== ОТРИСОВКА ==========
void Skeleton::draw() {
    // window->draw(*skeletonRect); // Для отладки хитбокса
    if (portal->getIsHalfPassed()) {
        drawVisualEffects();
    }
    if(portal->getIsHalfPassed()) window->draw(*skeletonSprite);
    if(portal->getIsHalfPassed()) healthbar->draw(!(this->isPlayingDieAnimation));
    if(portal->getIsExist()) portal->draw(*window);
}

// ========== ГЕТТЕРЫ ==========
sf::RectangleShape &Skeleton::getRect() {
    return *skeletonRect;
}

int Skeleton::getHP() {
    return HP_;
}

sf::Vector2f Skeleton::getPosition()
{
    return this->skeletonRect->getPosition();
}

void Skeleton::receiveBulletHit(const Bullet& bullet, bool splashHit)
{
    onBulletHit(bullet, splashHit);
}

void Skeleton::attachPlayer(Player &p)
{
    this->player_ = &p;
}
