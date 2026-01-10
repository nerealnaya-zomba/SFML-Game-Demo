#include<Shop.h>

void Shop::update()
{
    if(!isOpened) return;
}

void Shop::handleEvent(const sf::Event &event)
{
    if(!isOpened) return;
}

void Shop::draw(sf::RenderWindow& window)
{
    if(!isOpened) return;

    window.draw(*sprite);

    for (auto &&item : items)
    {
        // TODO
        // item.draw(window)
    }
    
}

Shop::Shop(GameData& d, Player& p, sf::Vector2f pos)
    : InteractiveObject(pos), isOpened(false), data(&d), player(&p)
{

}

void Shop::open()
{
    isOpened = true;
}

void Shop::close()
{
    isOpened = false;
}
