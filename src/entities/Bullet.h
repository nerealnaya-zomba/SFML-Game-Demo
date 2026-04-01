#pragma once

#include <SFML/Graphics.hpp>
#include <Defines.h>
#include <GameData.h>
#include <Mounting.h>
#include <Particle.h>
#include <TexturesIterHelper.h>

#include <algorithm>
#include <vector>

const int   ALFA_REDUCTION_VALUE  = 5;
const float SPEED_REDUCTION_VALUE = 0.95f;

class Bullet
{
public:
    enum class Type
    {
        Ember,
        Piercing,
        Splash,
        Storm,
        Prism,
        Beam
    };

    struct Config
    {
        Type type = Type::Ember;
        sf::Vector2f hitboxSize{30.f, 30.f};
        sf::Vector2f spriteScale{1.f, 1.f};
        sf::Color tint{255, 255, 255, 255};
        sf::Color trailColor{127, 255, 212, 255};
        sf::Color impactColor{127, 255, 212, 255};
        int damage = 10;
        int maxHits = 1;
        float splashRadius = 0.f;
        bool keepAliveOnHit = false;
        bool beamLike = false;
        float beamLifetime = 0.12f;
        double particleCooldownMs = 50.0;
        int trailParticleCount = 5;
        int deathParticleCount = 40;
    };

private:
    sf::RectangleShape* bulletRect_ = nullptr;
    std::vector<sf::Texture>* bulletTextures_ = nullptr;
    texturesIterHelper satiro_bullet_helper;
    sf::Sprite* bulletSprite_ = nullptr;

    Config config_{};
    int remainingHits_ = 1;
    std::vector<const void*> hitTargets_;
    sf::Clock lifeClock_;

    void colorReduction(sf::Color& color, int reduction);
    void alignSpriteToVelocity();

public:
    double maxDistance_{};
    double distancePassed{};
    double speedReductionValue{};
    double maxReduction{};

    bool canBeDeleted = false;
    bool isSheduledToBeDestroyed = false;
    bool isMakedDeathParticles = false;

    sf::Vector2f speed_{};
    sf::Vector2f originalSpeed_{};
    std::vector<Particle> particles;
    sf::Clock makeParticles_clock;
    double makeParticles_cooldown = 50;
    bool makeParticles_isOnCooldown = false;

    Bullet(sf::Vector2f startPosition, float maxDistance, GameData& gamedata);
    Bullet(sf::Vector2f startPosition, float maxDistance, GameData& gamedata, const Config& config);
    virtual ~Bullet();

    void setSpeed(sf::Vector2f offset);
    void setSpriteTexture(sf::Texture& texture);
    void setSpriteScale(sf::Vector2f scale);
    void setPosition(const sf::Vector2f& position);

    sf::RectangleShape& getBulletRect();
    sf::Vector2f getPosition();
    sf::Vector2f getCenterPosition() const;
    const Config& getConfig() const;
    int getDamage() const;
    float getSplashRadius() const;
    bool isBeamLike() const;
    bool canHitTarget(const void* target) const;
    void registerHitTarget(const void* target);
    void scheduleDestroy();

    void moveBullet();
    void speedReduction();
    void update();

    void updateTextures();

    void makeAfterParticles();
    void makeDeathParticles();
    void updateParticles();

    void draw(sf::RenderWindow& window);
};
