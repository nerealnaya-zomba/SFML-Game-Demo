#include <PlatformAtmosphere.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <random>

namespace
{
std::mt19937& platformAtmosphereRng()
{
    static std::mt19937 rng(std::random_device{}());
    return rng;
}
}

void PlatformAtmosphere::clear()
{
    emitters_.clear();
    particles_.clear();
    deltaClock_.restart();
}

std::size_t PlatformAtmosphere::addEmitter(const EmitterConfig& config)
{
    if (config.style == PlatformAtmosphereStyle::None)
    {
        return InvalidEmitterIndex;
    }

    emitters_.push_back({
        config,
        resetCooldown(config)
    });
    return emitters_.size() - 1;
}

void PlatformAtmosphere::setEmitterBounds(const std::size_t emitterIndex, const sf::FloatRect& bounds)
{
    if (emitterIndex >= emitters_.size())
    {
        return;
    }

    emitters_[emitterIndex].config.bounds = bounds;
}

void PlatformAtmosphere::update()
{
    const float deltaSeconds = std::min(0.05f, deltaClock_.restart().asSeconds());
    if (deltaSeconds <= 0.f)
    {
        return;
    }

    for (auto& emitter : emitters_)
    {
        emitter.cooldownSeconds -= deltaSeconds;
        if (emitter.cooldownSeconds > 0.f)
        {
            continue;
        }

        spawnParticle(emitter.config);
        emitter.cooldownSeconds = resetCooldown(emitter.config);
    }

    for (auto& particle : particles_)
    {
        particle.ageSeconds += deltaSeconds;
        particle.position += particle.velocity * deltaSeconds;

        if (particle.ageSeconds < particle.lifetimeSeconds * 0.45f)
        {
            particle.velocity.x *= 0.995f;
        }
    }

    particles_.erase(
        std::remove_if(
            particles_.begin(),
            particles_.end(),
            [](const Particle& particle) {
                return particle.ageSeconds >= particle.lifetimeSeconds;
            }
        ),
        particles_.end()
    );
}

void PlatformAtmosphere::draw(sf::RenderWindow& window) const
{
    for (const auto& particle : particles_)
    {
        const float lifeRatio = 1.f - particle.ageSeconds / particle.lifetimeSeconds;
        if (lifeRatio <= 0.f)
        {
            continue;
        }

        sf::CircleShape shape(particle.radius);
        shape.setOrigin({particle.radius, particle.radius});
        shape.setPosition(particle.position);

        sf::Color drawColor = particle.color;
        drawColor.a = static_cast<std::uint8_t>(std::clamp(lifeRatio * static_cast<float>(particle.color.a), 0.f, 255.f));
        shape.setFillColor(drawColor);
        window.draw(shape);

        sf::CircleShape glowShape(particle.radius * 1.9f);
        glowShape.setOrigin({glowShape.getRadius(), glowShape.getRadius()});
        glowShape.setPosition(particle.position);
        sf::Color glowColor = drawColor;
        glowColor.a = static_cast<std::uint8_t>(std::clamp(static_cast<float>(drawColor.a) * 0.35f, 0.f, 255.f));
        glowShape.setFillColor(glowColor);
        window.draw(glowShape);
    }
}

void PlatformAtmosphere::spawnParticle(const EmitterConfig& config)
{
    Particle particle;
    particle.color = config.color;

    const bool tallAnchor = config.bounds.size.y >= 100.f;

    switch (config.style)
    {
    case PlatformAtmosphereStyle::Dust:
        particle.position = {
            randomFloat(config.bounds.position.x + 6.f, config.bounds.position.x + config.bounds.size.x - 6.f),
            config.bounds.position.y + randomFloat(std::min(config.bounds.size.y, 6.f), std::min(config.bounds.size.y, 14.f))
        };
        particle.velocity = {
            randomFloat(-11.f, 11.f),
            randomFloat(18.f, 36.f)
        };
        particle.radius = randomFloat(1.2f, 2.4f);
        particle.lifetimeSeconds = randomFloat(1.0f, 1.8f);
        particle.color.a = static_cast<std::uint8_t>(randomFloat(112.f, 176.f));
        break;

    case PlatformAtmosphereStyle::Ember:
        particle.position = {
            randomFloat(config.bounds.position.x + 8.f, config.bounds.position.x + config.bounds.size.x - 8.f),
            config.bounds.position.y + randomFloat(3.f, std::min(config.bounds.size.y, 10.f))
        };
        particle.velocity = {
            randomFloat(-8.f, 8.f),
            randomFloat(20.f, 38.f)
        };
        particle.radius = randomFloat(1.1f, 2.0f);
        particle.lifetimeSeconds = randomFloat(0.9f, 1.5f);
        particle.color.a = static_cast<std::uint8_t>(randomFloat(120.f, 188.f));
        break;

    case PlatformAtmosphereStyle::Drip:
        particle.position = {
            randomFloat(config.bounds.position.x + 10.f, config.bounds.position.x + config.bounds.size.x - 10.f),
            config.bounds.position.y + (tallAnchor ? randomFloat(18.f, config.bounds.size.y * 0.4f) : config.bounds.size.y - 2.f)
        };
        particle.velocity = {
            randomFloat(-3.6f, 3.6f),
            randomFloat(52.f, 88.f)
        };
        particle.radius = randomFloat(1.3f, 2.2f);
        particle.lifetimeSeconds = randomFloat(0.6f, 1.1f);
        particle.color.a = static_cast<std::uint8_t>(randomFloat(110.f, 170.f));
        break;

    case PlatformAtmosphereStyle::None:
    default:
        return;
    }

    particles_.push_back(particle);
}

float PlatformAtmosphere::randomFloat(const float minValue, const float maxValue)
{
    std::uniform_real_distribution<float> distribution(minValue, maxValue);
    return distribution(platformAtmosphereRng());
}

float PlatformAtmosphere::resetCooldown(const EmitterConfig& config)
{
    switch (config.style)
    {
    case PlatformAtmosphereStyle::Dust:
        return randomFloat(0.35f, 0.8f) / std::max(0.35f, config.density);
    case PlatformAtmosphereStyle::Ember:
        return randomFloat(0.25f, 0.65f) / std::max(0.35f, config.density);
    case PlatformAtmosphereStyle::Drip:
        return randomFloat(0.5f, 1.05f) / std::max(0.35f, config.density);
    case PlatformAtmosphereStyle::None:
    default:
        return 9999.f;
    }
}
