#pragma once

#include <SFML/Graphics.hpp>
#include <cstddef>
#include <limits>
#include <vector>

enum class PlatformAtmosphereStyle
{
    None,
    Dust,
    Ember,
    Drip
};

class PlatformAtmosphere
{
public:
    static constexpr std::size_t InvalidEmitterIndex = std::numeric_limits<std::size_t>::max();

    struct EmitterConfig
    {
        sf::FloatRect bounds{};
        PlatformAtmosphereStyle style = PlatformAtmosphereStyle::None;
        sf::Color color = sf::Color::White;
        float density = 1.f;
    };

    void clear();
    std::size_t addEmitter(const EmitterConfig& config);
    void setEmitterBounds(std::size_t emitterIndex, const sf::FloatRect& bounds);
    void update();
    void draw(sf::RenderWindow& window) const;

private:
    struct Emitter
    {
        EmitterConfig config{};
        float cooldownSeconds = 0.f;
    };

    struct Particle
    {
        sf::Vector2f position{0.f, 0.f};
        sf::Vector2f velocity{0.f, 0.f};
        sf::Color color = sf::Color::White;
        float radius = 1.f;
        float ageSeconds = 0.f;
        float lifetimeSeconds = 1.f;
    };

    std::vector<Emitter> emitters_;
    std::vector<Particle> particles_;
    sf::Clock deltaClock_;

    void spawnParticle(const EmitterConfig& config);
    static float randomFloat(float minValue, float maxValue);
    static float resetCooldown(const EmitterConfig& config);
};
