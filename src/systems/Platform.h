#pragma once

#include <Mounting.h>
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
    };

    Platform();
    ~Platform();

    void draw(sf::RenderWindow& window);
    void addPlatform(sf::Vector2f position, std::string name);

    std::vector<std::shared_ptr<sf::RectangleShape>>& getRects();
    void clearPlatforms();

    static bool hasType(const std::string& name);
    static std::vector<std::string> getAvailableTypes();

private:
    std::vector<std::unique_ptr<sf::Sprite>> sprites;
    std::vector<std::shared_ptr<sf::RectangleShape>> rects;
    const std::unordered_map<std::string, sf::Texture>* textures{};

    static const std::unordered_map<std::string, TypeDefinition>& getTypeDefinitions();
    static const std::unordered_map<std::string, sf::Texture>& getSharedTextures();
};
