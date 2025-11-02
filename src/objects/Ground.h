#pragma once
#include<SFML/Graphics.hpp>
#include<iostream>
#include<Defines.h>
#include<Mounting.h>
#include<GameTextures.h>
class Ground
{
    public:
    Ground(GameTextures& gameTextures);
    ~Ground();

    void draw(sf::RenderWindow& window, float yPos);
    
    sf::RectangleShape& getRect();


    private:
    sf::RectangleShape* ground1Rect_m;
    sf::Texture* ground1Texture_m;
    sf::Sprite* ground1Sprite_m;
    

};