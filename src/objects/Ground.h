#pragma once
#include<SFML/Graphics.hpp>
#include<iostream>
#include<Defines.h>
class Ground
{
    public:
    Ground();
    ~Ground();

    void draw(sf::RenderWindow& window, float yPos);
    
    sf::RectangleShape& getRect();


    private:
    sf::RectangleShape* rect;
    sf::Texture* texture;
    sf::Sprite* sprite;
    

};