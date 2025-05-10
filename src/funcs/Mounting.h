#pragma once
#include <SFML/Graphics.hpp>

static void setRectangleOriginToMiddle(sf::RectangleShape& rect)
{
    rect.setOrigin({rect.getSize().x/2,rect.getSize().y/2});
}
static void setSpriteOriginToMiddle(sf::Sprite& sprite)
{
    sprite.setOrigin({sprite.getLocalBounds().size.x/2, sprite.getLocalBounds().size.y/2});
}