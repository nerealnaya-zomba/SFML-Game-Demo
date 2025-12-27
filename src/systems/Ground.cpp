#include "Ground.h"
#include<GameLevel.h>

Ground::Ground(GameData& gameTextures, GameLevelManager& levelManager, std::string groundFileName) 
{
    //Texture attaching
    attachTexture(gameTextures.TileSetGreenTextures.find(groundFileName)->second,this->ground1Texture_m);

    ground1Sprite_m = new sf::Sprite(*ground1Texture_m);

    ground1Rect_m = new sf::RectangleShape();
    ground1Rect_m->setSize({WINDOW_WIDTH,39.f});
    ground1Rect_m->setPosition({0,0});
}
Ground::~Ground()
{
    delete ground1Rect_m;
    delete ground1Sprite_m;
}
void Ground::draw(sf::RenderWindow& window, float yPos)
{
    sf::Vector2u tilesetsize = ground1Sprite_m->getTexture().getSize();
    float offSet = 8.f;
    ground1Rect_m->setPosition({0,yPos+offSet});
    float nextPos = 0.f;
    for (size_t i = 0; i < WINDOW_WIDTH/tilesetsize.x; i++)
    {
        ground1Sprite_m->setPosition({nextPos,yPos});
        
        window.draw(*ground1Sprite_m);
        //39.f, 126.f is the width and height of mramoric.png
        nextPos += tilesetsize.x;
    }
    
}

sf::RectangleShape &Ground::getRect()
{
    if(this->ground1Rect_m==nullptr)
    {
        throw std::runtime_error("ground is nullptr");
    }
    return *this->ground1Rect_m;
}

void Ground::setOffset(float offset)
{
    this->offset = offset;
}
