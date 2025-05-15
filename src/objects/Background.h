#pragma once
#include<SFML/Graphics.hpp>
#include<vector>
#include<iostream>
#include<Defines.h>
class Background
{  
    public:
    Background();
    ~Background();

    //Drawing
    void drawBackground(sf::RenderWindow& window);
    private:
    
    sf::Sprite* sky;
    sf::Sprite* mansion;

    sf::Texture* skyTexture;
    sf::Texture* mansionTexture;

    
};