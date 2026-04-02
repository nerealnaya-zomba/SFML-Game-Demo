#pragma once

#include <SFML/Graphics.hpp>

#include <cstddef>
#include <string>
#include <vector>

class BackgroundAtmosphere
{
public:
    BackgroundAtmosphere(
        const std::string& themeName,
        std::size_t layerIndex,
        std::size_t layerCount,
        sf::Vector2f parallaxFactor
    );

    void update(float deltaTime);
    void drawBehind(sf::RenderWindow& window, const sf::FloatRect& viewRect) const;
    void drawOverlay(sf::RenderWindow& window, const sf::FloatRect& viewRect) const;

private:
    enum class Theme
    {
        VerdantDawn,
        StormFront,
        BloodMoon,
        BoneCrypt,
        TwilightRift,
        MidnightRain
    };

    struct DriftParticle
    {
        sf::Vector2f uv;
        sf::Vector2f velocity;
        float radius = 0.f;
        float alpha = 0.f;
        float phase = 0.f;
        float glowScale = 1.f;
    };

    struct MistBand
    {
        float vertical = 0.f;
        float amplitude = 0.f;
        float speed = 0.f;
        float thickness = 0.f;
        float alpha = 0.f;
        float rotation = 0.f;
        float phase = 0.f;
        float widthScale = 1.f;
    };

    struct RainDrop
    {
        sf::Vector2f uv;
        float speed = 0.f;
        float length = 0.f;
        float alpha = 0.f;
        float skew = 0.f;
    };

    Theme theme = Theme::VerdantDawn;
    std::size_t layerIndex = 0;
    std::size_t layerCount = 1;
    sf::Vector2f parallaxFactor = {1.f, 1.f};

    float time = 0.f;
    float cycleSpeed = 0.06f;
    float cycleBias = 0.5f;
    float cycleAmplitude = 0.2f;
    bool gradientEnabled = false;
    bool celestialEnabled = false;
    bool mistEnabled = false;
    bool particlesEnabled = false;
    bool rainEnabled = false;
    bool flashEnabled = false;

    sf::Color topNight = sf::Color::Black;
    sf::Color middleNight = sf::Color::Black;
    sf::Color bottomNight = sf::Color::Black;
    sf::Color topDay = sf::Color::Black;
    sf::Color middleDay = sf::Color::Black;
    sf::Color bottomDay = sf::Color::Black;
    sf::Color accentColor = sf::Color::White;
    sf::Color hazeColor = sf::Color::White;
    sf::Color particleColor = sf::Color::White;
    sf::Color weatherColor = sf::Color::White;

    std::vector<DriftParticle> particles;
    std::vector<MistBand> mistBands;
    std::vector<RainDrop> rainDrops;

    Theme parseTheme(const std::string& themeName) const;
    void configureTheme();
    void seedMistBands(int count);
    void seedParticles(int count, bool rising);
    void seedRain(int count);

    void updateParticles(float deltaTime);
    void updateRain(float deltaTime);
    void drawGradient(sf::RenderWindow& window, const sf::FloatRect& viewRect) const;
    void drawCelestial(sf::RenderWindow& window, const sf::FloatRect& viewRect) const;
    void drawMist(sf::RenderWindow& window, const sf::FloatRect& viewRect, float alphaMultiplier) const;
    void drawParticles(sf::RenderWindow& window, const sf::FloatRect& viewRect) const;
    void drawRain(sf::RenderWindow& window, const sf::FloatRect& viewRect) const;
    void drawFlash(sf::RenderWindow& window, const sf::FloatRect& viewRect) const;

    bool isBackLayer() const;
    bool isFrontLayer() const;
    float cycleMix() const;
};
