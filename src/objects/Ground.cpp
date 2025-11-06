#include "Ground.h"

Ground::Ground(GameData& gameTextures)
{
    //Texture attaching
    attachTexture(gameTextures.ground1Texture,this->ground1Texture_m);

    ground1Sprite_m = new sf::Sprite(*ground1Texture_m);

    ground1Rect_m = new sf::RectangleShape();
    ground1Rect_m->setSize({WINDOW_WIDTH,39.f});
    ground1Rect_m->setPosition({0,0});
}
Ground::~Ground()
{
}
void Ground::draw(sf::RenderWindow& window, float yPos)
{
    float offSet = 8.f;
    ground1Rect_m->setPosition({0,yPos+offSet});
    float nextPos = 0.f;
    for (size_t i = 0; i < WINDOW_WIDTH/126.f; i++)
    {
        ground1Sprite_m->setPosition({nextPos,yPos});
        
        window.draw(*ground1Sprite_m);
        //39.f, 126.f is the width and height of mramoric.png
        nextPos += 126.f;
    }
    
}

sf::RectangleShape &Ground::getRect()
{
    return *this->ground1Rect_m;
}
