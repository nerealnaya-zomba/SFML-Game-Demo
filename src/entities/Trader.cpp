#include<Trader.h>

void Trader::updateTextures()
{
    switchToNextSpritePingPong(sprite.get(), *traderTextures, trader_helper);
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

void Trader::ySmoothFloating()
{
    float floatTime = (time.getElapsedTime().asMilliseconds()*0.001);
    sprite->setPosition(
        { sprite->getGlobalBounds().getCenter().x,
        sprite->getGlobalBounds().getCenter().y+(sin(floatTime*10))/2}
    );
}

Trader::Trader(GameData& data, Player& p,sf::Vector2f& pos) : InteractiveObject(pos,data.trader_idleTextures[0]), player(&p)
{
    attachTexture(data.trader_idleTextures,traderTextures,data.trader_idle_helper,trader_helper);

    setScale({3.f,3.f});

    setSpriteOriginToMiddle(*sprite);

    setPosition(pos);

    offsetToInteract = 300.f;

    // Shop init
    sf::Vector2f shopPos = {sprite->getGlobalBounds().getCenter().x,sprite->getGlobalBounds().getCenter().y + BASE_SHOP_OFFSET.y}; 
    shop = std::make_unique<Shop>(data,p, shopPos);

    time.start();
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

    ySmoothFloating();

    shop->update();
    
}

void Trader::handleEvent(const sf::Event &event)
{
    // Если подошел к торговцу
    if(isCanInteract)
    {
        // Event handling by shop if opened
        if(shop->getIsOpened())
        {
            shop->handleEvent(event);
        }

        // Открыть магазин если нажал на keyToOpenShop
        if(const auto* keyPressed = event.getIf<sf::Event::KeyPressed>())
        {
            // Открыть/закрыть на Enter
            if(keyPressed != nullptr && keyPressed->scancode == keyToOpenShop)
            {
                if(!shop->getIsOpened())
                {
                    shop->open();
                }
                else
                {
                    shop->close();
                }
            }
            // Закрыть магазин на C
            else if(keyPressed != nullptr && keyPressed->scancode == keyToOpenShop_second)
            {
                if(shop->getIsOpened())
                {
                    shop->close();
                }
            }
        }
    }
    else
    {
        shop->close();
    }
}

bool switchToNextSpritePingPong(sf::Sprite* sprite,
    std::vector<sf::Texture>& texturesArray, 
    texturesIterHelper& iterHelper)
{
    // Проверка безопасности
    if (!sprite || texturesArray.empty() || iterHelper.countOfTextures <= 0) {
        return false;
    }

    // Проверяем валидность текущего индекса
    if (iterHelper.ptrToTexture >= static_cast<int>(texturesArray.size())) {
        iterHelper.ptrToTexture = 0;
    }
    
    // Проверяем, что текстура по текущему индексу существует
    if (iterHelper.ptrToTexture < 0 || 
        iterHelper.ptrToTexture >= static_cast<int>(texturesArray.size())) {
        return false;
    }

    // Увеличиваем счетчик кадров
    iterHelper.iterationCounter++;
    
    // Проверяем, не пора ли переключить текстуру
    if (iterHelper.iterationCounter < iterHelper.iterationsTillSwitch) {
        return true;
    }
    
    // Сбрасываем счетчик кадров
    iterHelper.iterationCounter = 0;
    
    // Устанавливаем текущую текстуру
    sprite->setTexture(texturesArray[iterHelper.ptrToTexture]);
    
    // PingPong логика
    if (iterHelper.countOfTextures <= 1) {
        return true; // Одна текстура - ничего не делаем
    }
    
    if (iterHelper.goForward) {
        if (iterHelper.ptrToTexture >= iterHelper.countOfTextures - 1) {
            iterHelper.goForward = false;
            // Не выходим за границы
            iterHelper.ptrToTexture = std::max(0, iterHelper.countOfTextures - 2);
        } else {
            iterHelper.ptrToTexture++;
        }
    } else {
        if (iterHelper.ptrToTexture <= 0) {
            iterHelper.goForward = true;
            // Не выходим за границы
            iterHelper.ptrToTexture = std::min(1, iterHelper.countOfTextures - 1);
        } else {
            iterHelper.ptrToTexture--;
        }
    }
    
    // Гарантируем, что индекс в пределах массива текстур
    if (iterHelper.ptrToTexture < 0) {
        iterHelper.ptrToTexture = 0;
    }
    if (iterHelper.ptrToTexture >= static_cast<int>(texturesArray.size())) {
        iterHelper.ptrToTexture = texturesArray.size() - 1;
    }
    
    return true;
}