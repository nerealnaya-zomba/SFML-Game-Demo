#include <Platform.h>

#include <algorithm>
#include <cmath>
#include <iostream>
#include <random>

namespace
{
constexpr bool DRAW_PLATFORM_HITBOXES = false;
constexpr char EXTRACTED_PLATFORM_PREFIX[] = "images/platform/Extracted/";
constexpr float PLATFORM_SPRING_STIFFNESS = 42.f;
constexpr float PLATFORM_SPRING_DAMPING = 10.5f;
constexpr float PLATFORM_SPRING_REST_THRESHOLD = 0.03f;

std::mt19937& platformAnimationRng()
{
    static std::mt19937 rng(std::random_device{}());
    return rng;
}

float randomFloat(const float minValue, const float maxValue)
{
    std::uniform_real_distribution<float> distribution(minValue, maxValue);
    return distribution(platformAnimationRng());
}

const std::unordered_map<std::string, Platform::TypeDefinition>& buildPlatformDefinitions()
{
    static const std::unordered_map<std::string, Platform::TypeDefinition> definitions{
        {"Invisible-wall", {"", {72.f, 180.f}, {1.f, 1.f}, {0.f, 0.f}, sf::Color::Transparent, PlatformAtmosphereStyle::None, sf::Color::Transparent, 1.f, 0.f, 0.f}},
        {"Single-angled", {"images/platform/Single-angled.png", {40.f, 40.f}, {0.2f, 0.2f}, {-6.f, 0.f}, sf::Color::White}},
        {"Single-flat", {"images/platform/Single-flat.png", {40.f, 30.f}, {0.2f, 0.2f}, {-6.f, 0.f}, sf::Color::White}},
        {"Single-square", {"images/platform/Single-square.png", {50.f, 40.f}, {0.2f, 0.2f}, {-5.f, 0.f}, sf::Color::White}},
        {"Double-horizontal-1", {"images/platform/Double-horizontal-1.png", {175.f, 50.f}, {0.2f, 0.2f}, {0.f, 0.f}, sf::Color::White}},
        {"Double-horizontal-2", {"images/platform/Double-horizontal-2.png", {175.f, 55.f}, {0.2f, 0.2f}, {0.f, 0.f}, sf::Color::White}},
        {"Double-vertical", {"images/platform/Double-vertical.png", {55.f, 140.f}, {0.2f, 0.2f}, {-3.f, 0.f}, sf::Color::White}},
        {"Triple", {"images/platform/Triple.png", {270.f, 50.f}, {0.2f, 0.2f}, {5.f, -10.f}, sf::Color::White}},
        {"Quadruple", {"images/platform/Quadruple.png", {360.f, 55.f}, {0.2f, 0.2f}, {0.f, 0.f}, sf::Color::White}},

        {"Thorn-ramp", {"images/platform/Thorn-ramp.png", {54.f, 28.f}, {0.2f, 0.2f}, {-4.f, 8.f}, sf::Color(250, 245, 240, 255)}},
        {"Runed-ledge", {"images/platform/Runed-ledge.png", {64.f, 30.f}, {0.2f, 0.2f}, {1.f, 8.f}, sf::Color(250, 245, 240, 255)}},
        {"Bone-dais", {"images/platform/Bone-dais.png", {66.f, 30.f}, {0.2f, 0.2f}, {0.f, 7.f}, sf::Color(250, 245, 240, 255)}},
        {"Obsidian-span", {"images/platform/Obsidian-span.png", {180.f, 30.f}, {0.2f, 0.2f}, {1.f, 7.f}, sf::Color(248, 246, 255, 255)}},
        {"Cathedral-span", {"images/platform/Cathedral-span.png", {180.f, 30.f}, {0.2f, 0.2f}, {1.f, 6.f}, sf::Color(255, 244, 240, 255)}},
        {"Crypt-pillar", {"images/platform/Crypt-pillar.png", {60.f, 150.f}, {0.2f, 0.2f}, {-2.f, 12.f}, sf::Color(250, 250, 255, 255)}},
        {"Ritual-bridge", {"images/platform/Ritual-bridge.png", {264.f, 32.f}, {0.2f, 0.2f}, {-2.f, 8.f}, sf::Color(255, 244, 240, 255)}},
        {"Fallen-arcade", {"images/platform/Fallen-arcade.png", {336.f, 30.f}, {0.2f, 0.2f}, {-2.f, 6.f}, sf::Color(252, 248, 244, 255)}},

        {"Abyss-Thorn-ramp", {"images/platform/Extracted/platform_variant_03.png", {54.f, 28.f}, {0.27f, 0.27f}, {-2.f, 0.f}, sf::Color(255, 248, 244, 255), PlatformAtmosphereStyle::Ember, sf::Color(255, 168, 96, 116), 0.72f}},
        {"Abyss-Runed-ledge", {"images/platform/Extracted/platform_variant_03.png", {64.f, 30.f}, {0.29f, 0.29f}, {0.f, 1.f}, sf::Color(255, 248, 244, 255), PlatformAtmosphereStyle::Ember, sf::Color(255, 168, 96, 110), 0.78f}},
        {"Abyss-Bone-dais", {"images/platform/Extracted/platform_variant_03.png", {66.f, 30.f}, {0.30f, 0.30f}, {0.f, 1.f}, sf::Color(255, 248, 244, 255), PlatformAtmosphereStyle::Ember, sf::Color(255, 174, 102, 106), 0.74f}},
        {"Abyss-Obsidian-span", {"images/platform/Extracted/platform_variant_14.png", {180.f, 30.f}, {0.74f, 0.74f}, {0.f, 18.f}, sf::Color(255, 250, 244, 255), PlatformAtmosphereStyle::Dust, sf::Color(224, 204, 160, 92), 0.82f}},
        {"Abyss-Cathedral-span", {"images/platform/Extracted/platform_variant_14.png", {180.f, 30.f}, {0.74f, 0.74f}, {0.f, 18.f}, sf::Color(255, 250, 244, 255), PlatformAtmosphereStyle::Dust, sf::Color(214, 204, 178, 88), 0.78f}},
        {"Abyss-Ritual-bridge", {"images/platform/Extracted/platform_variant_14.png", {264.f, 32.f}, {1.08f, 1.08f}, {0.f, 31.f}, sf::Color(255, 250, 244, 255), PlatformAtmosphereStyle::Dust, sf::Color(214, 196, 162, 86), 0.90f}},
        {"Abyss-Fallen-arcade", {"images/platform/Extracted/platform_variant_14.png", {336.f, 30.f}, {1.36f, 1.36f}, {0.f, 43.f}, sf::Color(255, 250, 244, 255), PlatformAtmosphereStyle::Dust, sf::Color(204, 192, 164, 82), 0.96f}},
        {"Abyss-Crypt-pillar", {"images/platform/Crypt-pillar.png", {60.f, 150.f}, {0.2f, 0.2f}, {-2.f, 12.f}, sf::Color(250, 250, 255, 255), PlatformAtmosphereStyle::Drip, sf::Color(212, 136, 255, 104), 0.66f}}
    };

    return definitions;
}

std::unordered_map<std::string, sf::Texture> loadPlatformTextures()
{
    std::unordered_map<std::string, sf::Texture> loadedTextures;

    for (const auto& [name, definition] : buildPlatformDefinitions())
    {
        if (definition.texturePath.empty())
        {
            continue;
        }

        sf::Texture texture;
        if (!texture.loadFromFile(definition.texturePath))
        {
            std::cerr << "Error loading texture: " << definition.texturePath << std::endl;
            continue;
        }

        const bool isExtractedPlatform = definition.texturePath.find(EXTRACTED_PLATFORM_PREFIX) == 0;
        if (!isExtractedPlatform && texture.generateMipmap())
        {
            debugLog("Mipmap generated for platform: ", name);
        }

        texture.setSmooth(!isExtractedPlatform);
        loadedTextures.emplace(name, std::move(texture));
    }

    return loadedTextures;
}
}

const std::unordered_map<std::string, Platform::TypeDefinition>& Platform::getTypeDefinitions()
{
    return buildPlatformDefinitions();
}

const std::unordered_map<std::string, sf::Texture>& Platform::getSharedTextures()
{
    static const std::unordered_map<std::string, sf::Texture> sharedTextures = loadPlatformTextures();
    return sharedTextures;
}

void Platform::draw(sf::RenderWindow& window)
{
    if constexpr (DRAW_PLATFORM_HITBOXES)
    {
        for (auto& rect : rects)
        {
            window.draw(*rect);
        }
    }

    for (auto& instance : instances_)
    {
        if (instance.sprite)
        {
            window.draw(*instance.sprite);
        }
    }

    atmosphere_.draw(window);
}

void Platform::update()
{
    const float deltaSeconds = std::min(0.05f, deltaClock_.restart().asSeconds());
    const float elapsedSeconds = animationClock_.getElapsedTime().asSeconds();

    for (auto& instance : instances_)
    {
        const float hoverOffset =
            instance.hoverAmplitude > 0.f
                ? std::sin(elapsedSeconds * instance.hoverFrequency + instance.hoverPhase) * instance.hoverAmplitude
                : 0.f;

        instance.springVelocity += (
            -instance.springOffset * PLATFORM_SPRING_STIFFNESS -
            instance.springVelocity * PLATFORM_SPRING_DAMPING
        ) * deltaSeconds;
        instance.springOffset += instance.springVelocity * deltaSeconds;

        if (std::abs(instance.springOffset) < PLATFORM_SPRING_REST_THRESHOLD &&
            std::abs(instance.springVelocity) < PLATFORM_SPRING_REST_THRESHOLD)
        {
            instance.springOffset = 0.f;
            instance.springVelocity = 0.f;
        }

        instance.rect->setPosition({
            instance.anchorPosition.x + instance.hitboxOffset.x,
            instance.anchorPosition.y + instance.hitboxOffset.y + hoverOffset + instance.springOffset
        });

        if (instance.sprite)
        {
            instance.sprite->setPosition({
                instance.anchorPosition.x + instance.spriteAnchorSize.x * 0.5f + instance.spriteOffset.x,
                instance.anchorPosition.y + instance.spriteAnchorSize.y * 0.5f + instance.spriteOffset.y + hoverOffset + instance.springOffset
            });
        }

        if (instance.emitterIndex != PlatformAtmosphere::InvalidEmitterIndex)
        {
            atmosphere_.setEmitterBounds(instance.emitterIndex, instance.rect->getGlobalBounds());
        }
    }

    atmosphere_.update();
}

void Platform::addPlatform(sf::Vector2f position, std::string name)
{
    addPlatform(position, std::move(name), InstanceOverrides{});
}

void Platform::addPlatform(sf::Vector2f position, std::string name, InstanceOverrides overrides)
{
    const auto& definitions = getTypeDefinitions();
    const auto definitionIt = definitions.find(name);
    if (definitionIt == definitions.end())
    {
        std::cerr << "Unknown platform type: " << name << std::endl;
        return;
    }

    const TypeDefinition& definition = definitionIt->second;
    const sf::Vector2f hitboxSize = overrides.hitboxSize.value_or(definition.hitboxSize);
    const sf::Vector2f hitboxOffset = overrides.hitboxOffset.value_or(sf::Vector2f{0.f, 0.f});
    const sf::Vector2f spriteScale = overrides.spriteScale.value_or(definition.spriteScale);
    const PlatformAtmosphereStyle atmosphereStyle = overrides.atmosphereStyle.value_or(definition.atmosphereStyle);
    const sf::Color atmosphereColor = overrides.atmosphereColor.value_or(definition.atmosphereColor);
    const float atmosphereDensity = overrides.atmosphereDensity.value_or(definition.atmosphereDensity);
    const sf::Texture* texture = nullptr;
    if (!definition.texturePath.empty())
    {
        const auto textureIt = textures->find(name);
        if (textureIt == textures->end())
        {
            std::cerr << "Texture not loaded for platform type: " << name << std::endl;
            return;
        }

        texture = &textureIt->second;
    }

    PlatformInstance instance;
    instance.rect = std::make_shared<sf::RectangleShape>();
    instance.anchorPosition = position;
    instance.hitboxOffset = hitboxOffset;
    instance.hitboxSize = hitboxSize;
    instance.spriteAnchorSize = definition.hitboxSize;
    instance.rect->setPosition(position + hitboxOffset);
    instance.rect->setFillColor(sf::Color(0, 0, 0, 0));
    instance.rect->setSize(hitboxSize);
    instance.spriteOffset = definition.spriteOffset;
    instance.hoverAmplitude = definition.texturePath.empty() ? 0.f : definition.hoverAmplitude;
    instance.hoverFrequency = definition.hoverFrequency;
    instance.hoverPhase = randomFloat(0.f, 6.28318f);

    if (texture != nullptr)
    {
        instance.sprite = std::make_unique<sf::Sprite>(*texture);
        instance.sprite->setOrigin(instance.sprite->getGlobalBounds().getCenter());
        instance.sprite->setScale(spriteScale);
        instance.sprite->setColor(definition.tint);
        instance.sprite->setPosition(position + definition.hitboxSize * 0.5f + definition.spriteOffset);
    }

    instance.emitterIndex = atmosphere_.addEmitter({
        instance.rect->getGlobalBounds(),
        atmosphereStyle,
        atmosphereColor,
        atmosphereDensity
    });

    rects.push_back(instance.rect);
    instances_.push_back(std::move(instance));
}

void Platform::applyImpact(const sf::RectangleShape& rect, const float fallSpeed)
{
    for (auto& instance : instances_)
    {
        if (instance.rect.get() != &rect || instance.hoverAmplitude <= 0.f)
        {
            continue;
        }

        const float impactStrength = std::clamp(fallSpeed * 0.9f, 1.4f, 6.8f);
        instance.springVelocity += impactStrength * 64.f;
        break;
    }
}

std::vector<std::shared_ptr<sf::RectangleShape>>& Platform::getRects()
{
    return rects;
}

void Platform::clearPlatforms()
{
    instances_.clear();
    rects.clear();
    atmosphere_.clear();
    animationClock_.restart();
    deltaClock_.restart();
}

bool Platform::hasType(const std::string& name)
{
    return getTypeDefinitions().find(name) != getTypeDefinitions().end();
}

std::vector<std::string> Platform::getAvailableTypes()
{
    std::vector<std::string> names;
    names.reserve(getTypeDefinitions().size());
    for (const auto& [name, _] : getTypeDefinitions())
    {
        names.push_back(name);
    }
    std::sort(names.begin(), names.end());
    return names;
}

std::optional<Platform::TypeDefinition> Platform::getTypeDefinition(const std::string& name)
{
    const auto& definitions = getTypeDefinitions();
    const auto definitionIt = definitions.find(name);
    if (definitionIt == definitions.end())
    {
        return std::nullopt;
    }

    return definitionIt->second;
}

Platform::Platform()
    : textures(&getSharedTextures())
{
    animationClock_.restart();
    deltaClock_.restart();
}

Platform::~Platform() = default;
