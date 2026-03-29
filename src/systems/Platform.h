#pragma once
#include<SFML/Graphics.hpp>
#include<Mounting.h>
#include<vector>
#include<iostream>
class Platform
{
    public:
    
    void draw(sf::RenderWindow& window);

    //Single-angled
    //Single-flat
    //Single-square
    //Double-horizontal-1
    //Double-horizontal-2
    //Double-vertical
    //Triple
    //Quadruple
    void addPlatform(sf::Vector2f position, std::string name);

    //Returns array with raw pointers to rectangles-hitboxes (DO NOT DELETE MANUALLY)
    std::vector<std::shared_ptr<sf::RectangleShape>>& getRects();

    void clearPlatforms();

    Platform();
    ~Platform();

    private:
    std::vector<std::unique_ptr<sf::Sprite>> sprites;
    std::vector<std::shared_ptr<sf::RectangleShape>> rects;
    const std::vector<sf::Texture>* textures{};

    static const std::vector<sf::Texture>& getSharedTextures();
};
