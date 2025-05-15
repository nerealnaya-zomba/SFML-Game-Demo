#include "Ground.h"

Ground::Ground()
{
    texture = new sf::Texture();
    texture->loadFromFile("images/Ground/mramoric.png") ? std::cout << "Texture loaded: images/Ground/mramoric.png" << std::endl : std::cout << "Error loading texture: images/Ground/mramoric.png" << std::endl;

    sprite = new sf::Sprite(*texture);

    rect = new sf::RectangleShape();
    rect->setSize({WINDOW_WIDTH,39.f});
    rect->setPosition({0,0});
}
Ground::~Ground()
{
}
void Ground::draw(sf::RenderWindow& window, float yPos)
{
    float offSet = 8.f;
    rect->setPosition({0,yPos+offSet});
    float nextPos = 0.f;
    for (size_t i = 0; i < WINDOW_WIDTH/126.f; i++)
    {
        sprite->setPosition({nextPos,yPos});
        
        window.draw(*sprite);
        //39.f, 126.f is the width and height of mramoric.png
        nextPos += 126.f;
    }
    
}

sf::RectangleShape &Ground::getRect()
{
    return *this->rect;
}
