#include <SFML/Graphics.hpp>

void setRectangleOriginToMiddle(sf::RectangleShape& rect)
{
    rect.setOrigin({rect.getSize().x/2,rect.getSize().y/2});
}