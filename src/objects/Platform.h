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


    Platform();
    ~Platform();

    private:
    sf::Vector2f pos;
    std::vector<std::unique_ptr<sf::Sprite>> sprites;
    std::vector<std::unique_ptr<sf::RectangleShape>> rects;

    std::vector<sf::Texture> textures;
    std::vector<std::string> paths{
        "images/platform/Double-horizontal-1.png",
        "images/platform/Double-horizontal-2.png",
        "images/platform/Double-vertical.png",
        "images/platform/Quadruple.png",
        "images/platform/Single-angled.png",
        "images/platform/Single-flat.png",
        "images/platform/Single-square.png",
        "images/platform/Triple.png"
    };

    void initTextures(std::vector<sf::Texture>& textures, std::vector<std::string> paths);
};