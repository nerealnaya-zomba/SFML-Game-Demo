#include<Trader.h>

#include <sstream>

namespace
{
std::string wrapTraderDialogue(const std::string& text, std::size_t maxLineLength)
{
    if (text.empty() || maxLineLength == 0)
    {
        return text;
    }

    std::istringstream words(text);
    std::ostringstream wrapped;
    std::string word;
    std::size_t currentLineLength = 0;

    while (words >> word)
    {
        const std::size_t requiredLength = currentLineLength == 0
            ? word.size()
            : currentLineLength + 1 + word.size();

        if (currentLineLength > 0 && requiredLength > maxLineLength)
        {
            wrapped << '\n' << word;
            currentLineLength = word.size();
            continue;
        }

        if (currentLineLength > 0)
        {
            wrapped << ' ';
            ++currentLineLength;
        }

        wrapped << word;
        currentLineLength += word.size();
    }

    return wrapped.str();
}
}

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

void Trader::updateDialogue()
{
    shouldDrawDialogue_ = isCanInteract || shop->getIsOpened();
    if (!shouldDrawDialogue_)
    {
        return;
    }

    const CampaignProgress& campaign = player->getCampaignProgress();
    dialogueTitleText.setString(campaign.getMerchantGreeting());
    dialogueBodyText.setString(wrapTraderDialogue(campaign.getMerchantAdvice(), 36));

    const sf::FloatRect bodyBounds = dialogueBodyText.getLocalBounds();
    const sf::FloatRect titleBounds = dialogueTitleText.getLocalBounds();
    const float bubbleWidth = std::max(280.f, std::max(bodyBounds.size.x, titleBounds.size.x) + 34.f);
    const float bubbleHeight = 74.f + bodyBounds.size.y;
    const sf::Vector2f bubblePos = {
        getCenterPosition().x - bubbleWidth / 2.f,
        sprite->getGlobalBounds().position.y - bubbleHeight - 42.f
    };

    dialogueShadow.setSize({bubbleWidth, bubbleHeight});
    dialogueShadow.setPosition({bubblePos.x + 6.f, bubblePos.y + 8.f});

    dialogueBack.setSize({bubbleWidth, bubbleHeight});
    dialogueBack.setPosition(bubblePos);

    dialogueAccent.setSize({bubbleWidth, 5.f});
    dialogueAccent.setPosition(bubblePos);

    dialogueTitleText.setPosition({bubblePos.x + 16.f, bubblePos.y + 12.f});
    dialogueBodyText.setPosition({bubblePos.x + 16.f, bubblePos.y + 38.f});
}

Trader::Trader(GameData& data, Player& p,sf::Vector2f& pos)
    : InteractiveObject(pos,data.trader_idleTextures[0])
    , player(&p)
    , dialogueTitleText(*data.gameFont)
    , dialogueBodyText(*data.gameFont)
{
    attachTexture(data.trader_idleTextures,traderTextures,data.trader_idle_helper,trader_helper);

    setScale({3.f,3.f});

    setSpriteOriginToMiddle(*sprite);

    setPosition(pos);

    offsetToInteract = 300.f;

    // Shop init
    sf::Vector2f shopPos = {sprite->getGlobalBounds().getCenter().x,sprite->getGlobalBounds().getCenter().y + BASE_SHOP_OFFSET.y}; 
    shop = std::make_unique<Shop>(data,p, shopPos);

    dialogueShadow.setFillColor(sf::Color(0, 0, 0, 86));

    dialogueBack.setFillColor(sf::Color(20, 14, 17, 236));
    dialogueBack.setOutlineThickness(2.f);
    dialogueBack.setOutlineColor(sf::Color(162, 110, 74, 255));

    dialogueAccent.setFillColor(sf::Color(220, 144, 76, 255));

    dialogueTitleText.setCharacterSize(15);
    dialogueTitleText.setFillColor(sf::Color(255, 211, 158));
    dialogueTitleText.setOutlineThickness(1.4f);
    dialogueTitleText.setOutlineColor(sf::Color(0, 0, 0, 170));

    dialogueBodyText.setCharacterSize(14);
    dialogueBodyText.setFillColor(sf::Color(232, 228, 220));
    dialogueBodyText.setOutlineThickness(1.2f);
    dialogueBodyText.setOutlineColor(sf::Color(0, 0, 0, 160));
    dialogueBodyText.setLineSpacing(1.06f);

    time.start();
}

void Trader::draw(sf::RenderWindow &window)
{
    window.draw(*sprite);
}

void Trader::drawOverlay(sf::RenderWindow &window)
{
    if (shouldDrawDialogue_)
    {
        window.draw(dialogueShadow);
        window.draw(dialogueBack);
        window.draw(dialogueAccent);
        window.draw(dialogueTitleText);
        window.draw(dialogueBodyText);
    }

    shop->draw(window);
}

void Trader::update()
{
    updateTextures();

    checkIsInInteractionArea();

    lookAtPlayerSide();

    ySmoothFloating();
    updateDialogue();

    shop->update();
    
}

bool Trader::handleEvent(const sf::Event &event)
{
    // Если подошел к торговцу
    if(isCanInteract)
    {
        // Event handling by shop if opened
        if(shop->getIsOpened())
        {
            if (shop->handleEvent(event))
            {
                return true;
            }
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

                return true;
            }
        }
    }
    else
    {
        shop->close();
    }

    return false;
}

bool Trader::isShopOpened()
{
    return this->shop->getIsOpened();
}

bool Trader::blocksPlayerInput() const
{
    return shop && shop->getIsOpened();
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
