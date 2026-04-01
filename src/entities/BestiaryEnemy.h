#pragma once

#include "Enemy.h"
#include <SFML/Graphics.hpp>
#include <HealthBar.h>
#include <Particle.h>
#include <VisualEffects.h>
#include <enemyPortal.h>

#include <memory>
#include <string>
#include <vector>

class GameLevel;
class EnemyManager;
class Bullet;

class BestiaryEnemy : public Enemy
{
public:
    enum class Kind
    {
        WraithBat,
        VoidSlime,
        DreadScorpion
    };

    BestiaryEnemy(GameData& gameData,
                  EnemyManager& enemyManager,
                  GameLevel& gameLevel,
                  sf::RenderWindow& window,
                  Ground& ground,
                  Platform& platform,
                  Player& player,
                  const std::string& type,
                  sf::Vector2f position);
    ~BestiaryEnemy();

    bool isAlive = true;

    void updateAI();
    void updateControl();
    void updatePhysics();
    void updateTextures();
    void draw();

    sf::RectangleShape& getRect();
    sf::Vector2f getPosition() const;
    void attachPlayer(Player& player);
    void receiveBulletHit(const Bullet& bullet, bool splashHit = false);
    std::string getDropKey() const;

private:
    enum class State
    {
        Patrol,
        Chase,
        Windup,
        Attack,
        Recover,
        Hurt,
        Die
    };

    struct VisualRing
    {
        sf::Vector2f position{};
        sf::Color color{255, 255, 255, 255};
        float radius = 8.f;
        float maxRadius = 48.f;
        float growth = 2.f;
        float thickness = 2.f;
        float alpha = 180.f;
    };

    sf::RenderWindow* window_ = nullptr;
    Ground* ground_ = nullptr;
    Platform* platform_ = nullptr;
    Player* player_ = nullptr;
    GameLevel* gameLevel_ = nullptr;
    GameData* gameData_ = nullptr;
    EnemyManager* enemyManager_ = nullptr;

    Kind kind_ = Kind::WraithBat;
    State state_ = State::Patrol;
    std::string typeKey_;

    std::unique_ptr<enemyPortal> portal_;
    std::unique_ptr<sf::Sprite> sprite_;
    std::unique_ptr<sf::RectangleShape> rect_;
    std::unique_ptr<HealthBar> healthBar_;

    std::vector<sf::Texture>* idleTextures_ = nullptr;
    texturesIterHelper idleHelper_{};
    std::vector<sf::Texture>* moveTextures_ = nullptr;
    texturesIterHelper moveHelper_{};
    std::vector<sf::Texture>* attackTextures_ = nullptr;
    texturesIterHelper attackHelper_{};

    std::vector<Particle> effectParticles_{};
    std::vector<VisualRing> effectRings_{};

    sf::Vector2f spawnPosition_{};
    sf::Vector2f attackVelocity_{};
    sf::Vector2f enemyScale_{1.f, 1.f};
    sf::Vector2f knockback_{5.f, 4.f};
    sf::Vector2f hitboxSize_{32.f, 32.f};
    sf::Color baseTint_{255, 255, 255, 255};

    float spriteOffsetY_ = 0.f;
    float velocityX_ = 0.f;
    float velocityY_ = 0.f;
    float maxSpeed_ = 1.5f;
    float acceleration_ = 0.14f;
    float friction_ = 0.04f;
    float gravity_ = 0.18f;
    float alertDistance_ = 240.f;
    float attackDistance_ = 120.f;
    float loseDistance_ = 340.f;
    float verticalTolerance_ = 120.f;
    float patrolRange_ = 160.f;
    float hoverAmplitude_ = 18.f;
    float hoverFrequency_ = 2.5f;
    float chargeSpeed_ = 4.4f;
    float groundHopX_ = 1.8f;
    float groundHopY_ = 3.6f;
    float leapAttackX_ = 3.6f;
    float leapAttackY_ = 5.6f;
    float attackCooldownMs_ = 1000.f;
    float attackDurationMs_ = 420.f;
    float windupDurationMs_ = 220.f;
    float recoverDurationMs_ = 220.f;
    float hurtDurationMs_ = 180.f;
    float deathDurationMs_ = 620.f;
    float hoverPhase_ = 0.f;
    float patrolDir_ = 1.f;
    float attackImpactFrame_ = 3.f;
    float hitFlashDurationMs_ = 180.f;

    int HP_ = 100;
    int maxHP_ = 100;
    int DMG_ = 18;

    bool isOnGround_ = false;
    bool facingRight_ = true;
    bool lockedAttackFacingRight_ = true;
    bool playerDetected_ = false;
    bool attackDamageApplied_ = false;
    bool deathEffectPlayed_ = false;
    bool hasDroppedGold_ = false;
    bool collidedHorizontally_ = false;
    bool hitFlashActive_ = false;
    bool scorpionDeathPulseMidPlayed_ = false;
    bool scorpionDeathPulseLatePlayed_ = false;

    sf::Clock stateClock_{};
    sf::Clock attackCooldownClock_{};
    sf::Clock aggroClock_{};
    sf::Clock auraClock_{};
    sf::Clock patrolEffectClock_{};
    sf::Clock hopClock_{};
    sf::Clock deathSmokeClock_{};
    sf::Clock hitFlashClock_{};

    void loadData();
    void attachTextures();
    void resetAnimationHelpers();
    void setState(State newState);

    void updateBatAI(const sf::Vector2f& center, const sf::Vector2f& playerCenter);
    void updateSlimeAI(const sf::Vector2f& center, const sf::Vector2f& playerCenter);
    void updateScorpionAI(const sf::Vector2f& center, const sf::Vector2f& playerCenter);

    void updateBatControl(const sf::Vector2f& center, const sf::Vector2f& playerCenter);
    void updateSlimeControl(const sf::Vector2f& center, const sf::Vector2f& playerCenter);
    void updateScorpionControl(const sf::Vector2f& center, const sf::Vector2f& playerCenter);

    void updateGroundPhysics();
    void updateFlyingPhysics();
    void updateVisualEffects();
    void drawVisualEffects();
    void drawScorpionHitAura();
    void drawScorpionDeathAura();
    void triggerScorpionDeathBursts();

    void checkGroundCollision();
    void checkPlatformCollision();
    void checkBulletCollision();
    void applyGroundFriction();
    bool hasGroundAhead(float direction) const;
    void damagePlayerOnContact();
    void syncSpriteToRect();

    void spawnParticleBurst(const sf::Vector2f& origin,
                            const sf::Color& color,
                            int count,
                            float minSpeed,
                            float maxSpeed,
                            float radius,
                            float gravity,
                            float lifetime);
    void pushRing(const sf::Vector2f& position,
                  const sf::Color& color,
                  float radius,
                  float maxRadius,
                  float growth,
                  float thickness,
                  float alpha);
    void spawnNoticeEffect();
    void spawnHitEffect(const sf::Color& impactColor, bool splashHit);
    void spawnAttackEffect();
    void spawnImpactEffect();
    void spawnDeathEffect();
    void spawnPatrolEffect();
    void spawnLandingEffect();
    void spawnDeathSmoke();

    sf::Vector2f getCenterPosition() const;
    float getFacingSign() const;
};
