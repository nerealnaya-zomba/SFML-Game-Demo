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

Trader::Trader(GameData& data, Player& p,sf::Vector2f& pos) : InteractiveObject(pos), player(&p)
{
    attachTexture(data.trader_idleTextures,traderTextures,data.trader_idle_helper,trader_helper);
}

void Trader::draw(sf::RenderWindow &window)
{
    window.draw(*sprite);
}

void Trader::update()
{
    updateTextures();

    checkIsInInteractionArea();

    if(isCanInteract)
    {
        if(keyPressed->scancode == keyToOpenShop)
        {
            // TODO
            // shop.open();
            // ...
        }
    }
    else
    {
        //shop.close();
    }
    
}

void Trader::handleEvent(const sf::Event &event)
{
    this->keyPressed = event.getIf<sf::Event::KeyPressed>();
}