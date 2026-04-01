#include <Bullet.h>

#include <cmath>

using namespace gameUtils;

namespace
{
constexpr float kPi = 3.14159265f;

float vectorLength(const sf::Vector2f& vector)
{
    return std::sqrt(vector.x * vector.x + vector.y * vector.y);
}

sf::Vector2f normalizeOrZero(const sf::Vector2f& vector)
{
    const float length = vectorLength(vector);
    if (length <= 0.0001f)
    {
        return {0.f, 0.f};
    }

    return vector / length;
}
}

void Bullet::colorReduction(sf::Color& color, int reduction)
{
    if(color.a > 0)
    {
        for (int i = 0; i < reduction; i++)
        {
            if(color.a > 0)
            {
                color.a--;
            }
        }
    }
}

void Bullet::alignSpriteToVelocity()
{
    if (!bulletSprite_)
    {
        return;
    }

    const sf::Vector2f velocity = speed_;
    if (vectorLength(velocity) <= 0.01f)
    {
        return;
    }

    const float angleDegrees = std::atan2(velocity.y, velocity.x) * 180.f / kPi;
    bulletSprite_->setRotation(sf::degrees(angleDegrees));
}

Bullet::Bullet(sf::Vector2f pos, float maxDistance, GameData& gamedata)
    : Bullet(pos, maxDistance, gamedata, Config{})
{
}

Bullet::Bullet(sf::Vector2f pos, float maxDistance, GameData &gamedata, const Config& config)
    : config_(config)
{
    maxDistance_ = maxDistance;
    remainingHits_ = std::max(1, config_.maxHits);
    makeParticles_cooldown = config_.particleCooldownMs;

    bulletRect_ = new sf::RectangleShape();
    bulletRect_->setFillColor(sf::Color::Green);
    bulletRect_->setSize(config_.hitboxSize);
    bulletRect_->setPosition(pos);

    attachTexture(gamedata.bulletTextures, this->bulletTextures_, gamedata.satiro_bullet_helper, this->satiro_bullet_helper);
    bulletSprite_ = new sf::Sprite(bulletTextures_->at(0));
    setSpriteOriginToMiddle(*bulletSprite_);
    bulletSprite_->setScale(config_.spriteScale);
    bulletSprite_->setColor(config_.tint);
    bulletSprite_->setPosition(bulletRect_->getGlobalBounds().getCenter());
}

Bullet::~Bullet()
{
    delete bulletRect_;
    delete bulletSprite_;
}

void Bullet::setSpeed(sf::Vector2f offset)
{
    speed_ = offset;
    originalSpeed_ = speed_;

    if (config_.beamLike)
    {
        maxReduction = 0.f;
        speedReductionValue = 0.f;
        return;
    }

    maxReduction = 1.f;
    const double originalMagnitude = std::max(0.001, static_cast<double>(vectorLength(originalSpeed_)));
    double reductionDistance = maxDistance_ / 2.0;
    reductionDistance /= originalMagnitude;
    speedReductionValue = (originalMagnitude - maxReduction) / std::max(1.0, reductionDistance);
    alignSpriteToVelocity();
}

void Bullet::setSpriteTexture(sf::Texture& texture)
{
    bulletSprite_->setTexture(texture);
}

void Bullet::setSpriteScale(sf::Vector2f scale)
{
    bulletSprite_->setScale(scale);
}

void Bullet::setPosition(const sf::Vector2f& position)
{
    bulletRect_->setPosition(position);
    bulletSprite_->setPosition(bulletRect_->getGlobalBounds().getCenter());
}

sf::RectangleShape &Bullet::getBulletRect()
{
    return *bulletRect_;
}

sf::Vector2f Bullet::getPosition()
{
    return bulletRect_->getPosition();
}

sf::Vector2f Bullet::getCenterPosition() const
{
    return bulletRect_->getGlobalBounds().getCenter();
}

const Bullet::Config& Bullet::getConfig() const
{
    return config_;
}

int Bullet::getDamage() const
{
    return config_.damage;
}

float Bullet::getSplashRadius() const
{
    return config_.splashRadius;
}

bool Bullet::isBeamLike() const
{
    return config_.beamLike;
}

bool Bullet::canHitTarget(const void* target) const
{
    return std::find(hitTargets_.begin(), hitTargets_.end(), target) == hitTargets_.end();
}

void Bullet::registerHitTarget(const void* target)
{
    if (!canHitTarget(target))
    {
        return;
    }

    hitTargets_.push_back(target);
    if (remainingHits_ > 0)
    {
        --remainingHits_;
    }

    if (!config_.keepAliveOnHit || remainingHits_ <= 0)
    {
        isSheduledToBeDestroyed = true;
    }
}

void Bullet::scheduleDestroy()
{
    isSheduledToBeDestroyed = true;
}

void Bullet::moveBullet()
{
    if (config_.beamLike)
    {
        if (lifeClock_.getElapsedTime().asSeconds() >= config_.beamLifetime)
        {
            isSheduledToBeDestroyed = true;
        }
        return;
    }

    if (isSheduledToBeDestroyed)
    {
        speed_ *= SPEED_REDUCTION_VALUE;
    }

    if(distancePassed >= (maxDistance_ / 2.0) && !isSheduledToBeDestroyed)
    {
        speedReduction();
    }

    distancePassed += vectorLength(speed_);
    bulletRect_->move(speed_);
    bulletSprite_->move(speed_);
}

void Bullet::speedReduction()
{
    const float currentMagnitude = vectorLength(speed_);
    if (currentMagnitude <= maxReduction || currentMagnitude <= 0.0001f)
    {
        return;
    }

    const float newMagnitude = std::max(static_cast<float>(maxReduction), currentMagnitude - static_cast<float>(speedReductionValue));
    speed_ = normalizeOrZero(speed_) * newMagnitude;
}

void Bullet::update()
{
    moveBullet();

    if(!makeParticles_isOnCooldown)
    {
        makeAfterParticles();
        makeParticles_isOnCooldown = true;
        makeParticles_clock.restart();
    }
    else if(makeParticles_clock.getElapsedTime().asMilliseconds() >= makeParticles_cooldown && !isSheduledToBeDestroyed)
    {
        makeParticles_isOnCooldown = false;
        makeParticles_clock.stop();
    }

    if(!isMakedDeathParticles && isSheduledToBeDestroyed)
    {
        makeDeathParticles();
        isMakedDeathParticles = true;
    }

    if(distancePassed >= maxDistance_ && !isSheduledToBeDestroyed && !config_.beamLike)
    {
        isSheduledToBeDestroyed = true;
    }

    updateParticles();

    if(isSheduledToBeDestroyed && particles.empty())
    {
        canBeDeleted = true;
    }
}

void Bullet::updateTextures()
{
    if (!config_.beamLike)
    {
        switchToNextSprite(bulletSprite_, *bulletTextures_, satiro_bullet_helper, switchSprite_SwitchOption::Loop);
    }
    const sf::Color currentColor = bulletSprite_->getColor();
    bulletSprite_->setColor(sf::Color(config_.tint.r, config_.tint.g, config_.tint.b, currentColor.a));
    alignSpriteToVelocity();
}

void Bullet::makeAfterParticles()
{
    const sf::Vector2f bulletCenter = getCenterPosition();
    const sf::FloatRect bounds = bulletRect_->getGlobalBounds();
    const int particleCount = std::max(1, config_.trailParticleCount);

    for (int i = 0; i < particleCount; i++)
    {
        const sf::Vector2f particlePos = config_.beamLike
            ? sf::Vector2f{
                random(bounds.position.x, bounds.position.x + bounds.size.x),
                random(bounds.position.y, bounds.position.y + bounds.size.y)
            }
            : sf::Vector2f{
                bulletCenter.x + random(-bounds.size.x * 0.25f, bounds.size.x * 0.25f),
                bulletCenter.y + random(-bounds.size.y * 0.25f, bounds.size.y * 0.25f)
            };

        const sf::Vector2f baseVelocity = config_.beamLike
            ? sf::Vector2f{random(-20.f, 20.f), random(-10.f, 10.f)}
            : normalizeOrZero(speed_) * random(-80.f, 20.f);

        particles.emplace_back(
            particlePos,
            baseVelocity,
            sf::Vector2f(random(-40.f, 40.f), random(-40.f, 40.f)),
            config_.trailColor,
            config_.beamLike ? 1.7f : 1.2f,
            config_.beamLike ? 0.f : 2.f,
            0.82f,
            config_.beamLike ? 0.18f : 0.38f
        );
    }
}

void Bullet::makeDeathParticles()
{
    const sf::Vector2f bulletCenter = getCenterPosition();
    const int particleCount = std::max(8, config_.deathParticleCount);

    for (int i = 0; i < particleCount; i++)
    {
        const float angle = random(0.f, 360.f) * kPi / 180.f;
        const float speedValue = random(40.f, config_.beamLike ? 130.f : 180.f);
        const sf::Vector2f velocity(
            std::cos(angle) * speedValue,
            std::sin(angle) * speedValue
        );

        particles.emplace_back(
            sf::Vector2f(
                bulletCenter.x + random(-bulletRect_->getSize().x * 0.35f, bulletRect_->getSize().x * 0.35f),
                bulletCenter.y + random(-bulletRect_->getSize().y * 0.35f, bulletRect_->getSize().y * 0.35f)
            ),
            velocity,
            sf::Vector2f(random(-28.f, 28.f), random(-28.f, 28.f)),
            config_.impactColor,
            config_.beamLike ? 2.1f : 1.5f,
            0.f,
            0.84f,
            config_.beamLike ? 0.48f : 0.72f
        );
    }
}

void Bullet::updateParticles()
{
    for (auto& particle : particles)
    {
        particle.update();
    }
    particles.erase(
        std::remove_if(particles.begin(), particles.end(),
            [](const Particle& particle) { return !particle.getIsAlive(); }),
        particles.end()
    );
}

void Bullet::draw(sf::RenderWindow &window)
{
    if (config_.beamLike)
    {
        const sf::Vector2f center = getCenterPosition();
        sf::RectangleShape beamGlow(bulletRect_->getSize());
        beamGlow.setOrigin({beamGlow.getSize().x / 2.f, beamGlow.getSize().y / 2.f});
        beamGlow.setPosition(center);
        beamGlow.setRotation(bulletSprite_->getRotation());
        beamGlow.setFillColor(sf::Color(config_.impactColor.r, config_.impactColor.g, config_.impactColor.b, 72));
        window.draw(beamGlow);
    }

    if(isSheduledToBeDestroyed)
    {
        sf::Color color = bulletSprite_->getColor();
        colorReduction(color, ALFA_REDUCTION_VALUE);
        bulletSprite_->setColor(color);
    }

    window.draw(*bulletSprite_);
    for (auto& particle : particles)
    {
        particle.draw(window);
    }
}
