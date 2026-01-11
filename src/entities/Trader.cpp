#include<Trader.h>

void Trader::updateTextures()
{
    gameUtils::switchToNextSprite(sprite.get(),*traderTextures,trader_helper,switchSprite_SwitchOption::Loop);
}

void Trader::checkIsInInteractionArea()
{
    isInAreaOfInteraction(player->getCenterPosition()) ? 
    isCanInteract=true : isCanInteract = false;
}

void Trader::lookAtPlayerSide()
{
    if(player->getCenterPosition().x <= getCenterPosition().x)
    {
        sf::Vector2f spriteScale = sprite->getScale();
        sprite->setScale({abs(spriteScale.x),spriteScale.y});
    }
    else if(player->getCenterPosition().x > getCenterPosition().x)
    {
        sf::Vector2f spriteScale = sprite->getScale();
        sprite->setScale({-abs(spriteScale.x),spriteScale.y});
    }
}

Trader::Trader(GameData& data, Player& p,sf::Vector2f& pos) : InteractiveObject(pos,data.trader_idleTextures[0]), player(&p)
{
    attachTexture(data.trader_idleTextures,traderTextures,data.trader_idle_helper,trader_helper);

    sf::Vector2f shopPos = {sprite->getGlobalBounds().getCenter().x,sprite->getGlobalBounds().getCenter().y + BASE_SHOP_OFFSET.y}; 
    shop = std::make_unique<Shop>(data,p, shopPos);

    setScale({3.f,3.f});

    setSpriteOriginToMiddle(*sprite);

    setPosition(pos);
}

void Trader::draw(sf::RenderWindow &window)
{
    window.draw(*sprite);

    shop->draw(window);
}

void Trader::update()
{
    updateTextures();

    checkIsInInteractionArea();

    lookAtPlayerSide();

    shop->update();

    // Открыть магазин только если игрок подошел к торговцу и нажал на keyToOpenShop
    if(isCanInteract)
    {
        if(keyPressed != nullptr && keyPressed->scancode == keyToOpenShop)
        {
            shop->open();
        }
    }
    else
    {
        shop->close();
    }
    
}

void Trader::handleEvent(const sf::Event &event)
{
    this->keyPressed = event.getIf<sf::Event::KeyPressed>();
}