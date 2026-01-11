#include<Item.h>

Item::Item(GameData& data, sf::Vector2i iconSize, sf::Vector2i position, std::string name, std::string displayItemName, Quality q, int p, Stats itemStats)
    : quality(q), price(p), displayName(displayItemName), stats(itemStats)
{
    // Получаем текстуру по названию
    auto itemIt = data.itemsTextures.find(name);
    sprite = std::make_unique<sf::Sprite>(itemIt->second);

    setSpriteOriginToMiddle(*sprite);

    // Инициализация rect
    rect.size = iconSize;
    rect.position = position;

    // Корректирование размера спрайта под размер rect
    sf::Vector2i spriteSize = static_cast<sf::Vector2i>(sprite->getTexture().getSize());
    if(spriteSize!=rect.size)
    {
        sf::Vector2f rectSize = static_cast<sf::Vector2f>(rect.size);
        sf::Vector2f spriteSize = static_cast<sf::Vector2f>(sprite->getTexture().getSize());

        float spriteScaleX = (rectSize.x / spriteSize.x) * BASE_ITEM_SCALE.x;
        float spriteScaleY = (rectSize.y / spriteSize.y) * BASE_ITEM_SCALE.y;

        sprite->setScale({spriteScaleX,spriteScaleY});
    }
}

void Item::draw(sf::RenderWindow& window)
{
    // Позиционирование спрайта по центру
    if(static_cast<sf::Vector2i>(sprite->getGlobalBounds().getCenter()) != rect.getCenter())
    {
        sprite->setPosition(static_cast<sf::Vector2f>(rect.getCenter()));
    }

    window.draw(*sprite);
}

void Item::setPosition(sf::Vector2i& pos)
{
    rect.position = {pos.x-rect.size.x,pos.y-rect.size.y};
    sprite->setPosition(static_cast<sf::Vector2f>(pos));
}
