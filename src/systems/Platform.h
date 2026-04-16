#pragma once

#include <Mounting.h>
#include <PlatformAtmosphere.h>
#include <SFML/Graphics.hpp>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class Platform
{
public:
    struct TypeDefinition
    {
        std::string texturePath;
        sf::Vector2f hitboxSize;
        sf::Vector2f spriteScale;
        sf::Vector2f spriteOffset;
        sf::Color tint;
        PlatformAtmosphereStyle atmosphereStyle = PlatformAtmosphereStyle::None;
        sf::Color atmosphereColor = sf::Color::Transparent;
        float atmosphereDensity = 1.f;
        float hoverAmplitude = 1.35f;
        float hoverFrequency = 0.85f;
    };

    Platform();
    ~Platform();

    void update();
    void draw(sf::RenderWindow& window);
    void addPlatform(sf::Vector2f position, std::string name);
    void applyImpact(const sf::RectangleShape& rect, float fallSpeed);

    std::vector<std::shared_ptr<sf::RectangleShape>>& getRects();
    void clearPlatforms();

    static bool hasType(const std::string& name);
    static std::vector<std::string> getAvailableTypes();

private:
    struct PlatformInstance
    {
        std::shared_ptr<sf::RectangleShape> rect;
        std::unique_ptr<sf::Sprite> sprite;
        sf::Vector2f baseRectPosition{0.f, 0.f};
        sf::Vector2f spriteOffset{0.f, 0.f};
        float hoverAmplitude = 0.f;
        float hoverFrequency = 0.f;
        float hoverPhase = 0.f;
        float springOffset = 0.f;
        float springVelocity = 0.f;
        std::size_t emitterIndex = PlatformAtmosphere::InvalidEmitterIndex;
    };

    std::vector<PlatformInstance> instances_;
    std::vector<std::shared_ptr<sf::RectangleShape>> rects;
    const std::unordered_map<std::string, sf::Texture>* textures{};
    PlatformAtmosphere atmosphere_;
    sf::Clock animationClock_;
    sf::Clock deltaClock_;

    static const std::unordered_map<std::string, TypeDefinition>& getTypeDefinitions();
    static const std::unordered_map<std::string, sf::Texture>& getSharedTextures();
};
