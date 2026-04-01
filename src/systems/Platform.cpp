#include <Platform.h>

#include <algorithm>
#include <iostream>

namespace
{
constexpr bool DRAW_PLATFORM_HITBOXES = false;

const std::unordered_map<std::string, Platform::TypeDefinition>& buildPlatformDefinitions()
{
    static const std::unordered_map<std::string, Platform::TypeDefinition> definitions{
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
        {"Fallen-arcade", {"images/platform/Fallen-arcade.png", {336.f, 30.f}, {0.2f, 0.2f}, {-2.f, 6.f}, sf::Color(252, 248, 244, 255)}}
    };

    return definitions;
}

std::unordered_map<std::string, sf::Texture> loadPlatformTextures()
{
    std::unordered_map<std::string, sf::Texture> loadedTextures;

    for (const auto& [name, definition] : buildPlatformDefinitions())
    {
        sf::Texture texture;
        if (!texture.loadFromFile(definition.texturePath))
        {
            std::cerr << "Error loading texture: " << definition.texturePath << std::endl;
            continue;
        }

        if (texture.generateMipmap())
        {
            debugLog("Mipmap generated for platform: ", name);
        }
        texture.setSmooth(true);
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

    for (auto& sprite : sprites)
    {
        window.draw(*sprite);
    }
}

void Platform::addPlatform(sf::Vector2f position, std::string name)
{
    const auto& definitions = getTypeDefinitions();
    const auto definitionIt = definitions.find(name);
    if (definitionIt == definitions.end())
    {
        std::cerr << "Unknown platform type: " << name << std::endl;
        return;
    }

    const auto textureIt = textures->find(name);
    if (textureIt == textures->end())
    {
        std::cerr << "Texture not loaded for platform type: " << name << std::endl;
        return;
    }

    const TypeDefinition& definition = definitionIt->second;

    auto rect = std::make_shared<sf::RectangleShape>();
    rect->setPosition(position);
    rect->setFillColor(sf::Color(0, 0, 0, 0));
    rect->setSize(definition.hitboxSize);
    const sf::Vector2f center = rect->getGlobalBounds().getCenter();
    rects.push_back(rect);

    auto sprite = std::make_unique<sf::Sprite>(textureIt->second);
    sprite->setOrigin(sprite->getGlobalBounds().getCenter());
    sprite->setPosition(center + definition.spriteOffset);
    sprite->setScale(definition.spriteScale);
    sprite->setColor(definition.tint);
    sprites.push_back(std::move(sprite));
}

std::vector<std::shared_ptr<sf::RectangleShape>>& Platform::getRects()
{
    return rects;
}

void Platform::clearPlatforms()
{
    sprites.clear();
    rects.clear();
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

Platform::Platform()
    : textures(&getSharedTextures())
{
}

Platform::~Platform() = default;
