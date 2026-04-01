#include<Item.h>
#include<stdexcept>

Item::Item(
    GameData& data,
    sf::Vector2i iconSize,
    sf::Vector2i position,
    std::string name,
    std::string displayItemName,
    Quality q,
    Category c,
    int p,
    Stats itemStats,
    WeaponStats weaponData,
    std::string itemDescription
)
    : stats(itemStats)
    , weaponStats(weaponData)
    , quality(q)
    , category(c)
    , iconName(std::move(name))
    , price(p)
    , displayName(std::move(displayItemName))
    , description(std::move(itemDescription))
{
    // Получаем текстуру по названию
    auto itemIt = data.itemsTextures.find(iconName);
    if(itemIt == data.itemsTextures.end())
    {
        throw std::runtime_error("Cannot find item texture: " + iconName);
    }
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

    // Инициализируем базовый, константный, скейл
    baseScale = sprite->getScale();
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

bool Item::isIntersects(sf::Vector2i pos)
{
    sf::IntRect mouseRect;
    mouseRect.size = {1,1};
    mouseRect.position = pos;

    if(mouseRect.findIntersection(rect))
    {
        return true;
    }

    return false;
}

sf::Vector2f Item::getBaseScale()
{
    return baseScale;
}

sf::Vector2u Item::getTextureSize()
{
    return this->sprite->getTexture().getSize();
}

sf::Vector2f Item::getCenterPosition()
{
    return static_cast<sf::Vector2f>(this->rect.getCenter());
}

sf::FloatRect Item::getBounds() const
{
    return sprite->getGlobalBounds();
}

const sf::Texture &Item::getTexture() const
{
    return sprite->getTexture();
}

bool Item::isPurchased() const
{
    return purchased;
}

void Item::setPosition(const sf::Vector2i &pos)
{
    rect.position = {pos.x-rect.size.x,pos.y-rect.size.y};
    sprite->setPosition(static_cast<sf::Vector2f>(pos));
}

void Item::setScale(sf::Vector2f scale)
{
    this->sprite->setScale(scale);
}

void Item::setColor(const sf::Color &color)
{
    sprite->setColor(color);
}

void Item::markPurchased(bool value)
{
    purchased = value;
}
