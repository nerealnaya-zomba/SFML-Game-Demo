#pragma once
#include<SFML/Graphics.hpp>
#include<Interactive.h>

class Trader : public InteractiveObject
{
private:



public:

    void draw(sf::RenderWindow& window)         override;
    void update()                               override;
    void handleEvent(const sf::Event& event)    override;

};