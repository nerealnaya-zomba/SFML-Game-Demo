#pragma once
#include <SFML/Graphics.hpp>
#include<string>
#include<iostream>

static void setRectangleOriginToMiddle(sf::RectangleShape& rect)
{
    rect.setOrigin({rect.getSize().x/2,rect.getSize().y/2});
}
static void setSpriteOriginToMiddle(sf::Sprite& sprite)
{
    sprite.setOrigin({sprite.getLocalBounds().size.x/2, sprite.getLocalBounds().size.y/2});
}

//Loading
static void initTextures(std::vector<sf::Texture> &textures, std::string path, int texturesCount, int maxDigits = 5)
{
    sf::Texture* texture = new sf::Texture;
    std::string basePath = path;
    int counter = 0;
    
    while (counter<=texturesCount) {
        // Форматируем число с ведущими нулями (00000, 00001, ...)
        std::ostringstream filename;
        filename << basePath 
                << std::setw(maxDigits) << std::setfill('0') << counter 
                << ".png";

        // Пытаемся загрузить текстуру
        
        if (!texture->loadFromFile(filename.str())) 
        {
            break;
        }
        textures.push_back(*texture);
        std::cout << "Loaded: " << filename.str() << std::endl;
        counter++;

    }
}