#include<LevelPortal.h>
#include<math.h>

void LevelPortal::portalOpeningAnimation()
{
    sf::Vector2f currentScale = sprite->getScale();
    
    // Если ещё не полностью открыт (сравниваем с openedScale)
    if (currentScale.x < openedScale.x || currentScale.y < openedScale.y) {
        float elapsed = appearClock.getElapsedTime().asMilliseconds();
        float progress = std::min(elapsed / appearTime, 1.0f);
        
        // Интерполяция от closedScale к openedScale
        sf::Vector2f newScale;
        newScale.x = closedScale.x + (openedScale.x - closedScale.x) * progress;
        newScale.y = closedScale.y + (openedScale.y - closedScale.y) * progress;
        
        sprite->setScale(newScale);
        
        if (progress >= 1.0f) {
            isOpened = true;
            isClosed = false;
            isCalledForOpen = false;
        }
    }
}

void LevelPortal::portalClosingAnimation()
{
    sf::Vector2f currentScale = sprite->getScale();
    
    // Если ещё не полностью закрыт (сравниваем с closedScale)
    if (currentScale.x > closedScale.x || currentScale.y > closedScale.y) {
        float elapsed = appearClock.getElapsedTime().asMilliseconds();
        float progress = std::min(elapsed / disappearTime, 1.0f);
        
        // Интерполяция от openedScale к closedScale
        sf::Vector2f newScale;
        newScale.x = openedScale.x + (closedScale.x - openedScale.x) * progress;
        newScale.y = openedScale.y + (closedScale.y - openedScale.y) * progress;
        
        sprite->setScale(newScale);
        
        if (progress >= 1.0f) {
            isOpened = false;
            isClosed = true;
            isCalledForClose = false;
        }
    }
}

LevelPortal::LevelPortal(const sf::Vector2f basePos, const int aT, const int dT, const int eT, GameData &gameData, GameLevelManager &m)
    : InteractiveObject(basePos), manager(&m), isUsed(false), appearTime(aT), disappearTime(dT), existTime(eT), isOpened(false), isClosed(true),
      openedScale(BASE_OPENED_SCALE), closedScale(BASE_CLOSED_SCALE)
{
    //portalBlue
    attachTexture(gameData.portalBlue1Textures, this->portalBlue1Textures,  gameData.portalBlue1Helper,   this->portalBlue1Helper   );
    attachTexture(gameData.portalBlue2Textures, this->portalBlue2Textures,  gameData.portalBlue2Helper,   this->portalBlue2Helper   );
    attachTexture(gameData.portalBlue3Textures, this->portalBlue3Textures,  gameData.portalBlue3Helper,   this->portalBlue3Helper   );
    attachTexture(gameData.portalBlue4Textures, this->portalBlue4Textures,  gameData.portalBlue4Helper,   this->portalBlue4Helper   );
    attachTexture(gameData.portalBlue5Textures, this->portalBlue5Textures,  gameData.portalBlue5Helper,   this->portalBlue5Helper   );
    attachTexture(gameData.portalBlue6Textures, this->portalBlue6Textures,  gameData.portalBlue6Helper,   this->portalBlue6Helper   );
    attachTexture(gameData.portalBlue7Textures, this->portalBlue7Textures,  gameData.portalBlue7Helper,   this->portalBlue7Helper   );
    attachTexture(gameData.portalBlue8Textures, this->portalBlue8Textures,  gameData.portalBlue8Helper,   this->portalBlue8Helper   );

    

    setSpriteOriginToMiddle(*sprite);

    sprite->setPosition(basePos);
}

void LevelPortal::draw(sf::RenderWindow &window)
{
    gameUtils::switchToNextSprite(sprite.get(),*portalBlue1Textures,portalBlue1Helper,switchSprite_SwitchOption::Loop);

    if(isOpened || isCalledForOpen || isCalledForClose) window.draw(*sprite);
}

void LevelPortal::update()
{
    if(isCalledForOpen) portalOpeningAnimation();
    if(isCalledForClose) portalClosingAnimation();

    if(isOpened)
    {
        if(isUsed){
            if(manager->goToLevel(levelName))
            {
                isUsed = false;
            } else{ 
                isUsed = false;
                // TODO Добавить предупреждение для игрока что локации не существует/не выбрана
            }
        }
    }
    
}

void LevelPortal::handleEvent(const sf::Event &event)
{
    if(const auto* keyPressed = event.getIf<sf::Event::KeyPressed>())
    {
        // Use portal
        if(isOpened)
        {
            if(keyPressed->scancode == sf::Keyboard::Scancode::Enter)
            {
                isUsed = true;
            }
        }
    }
}

void LevelPortal::setPortalDestination(std::string levelN)
{
    this->levelName = std::make_optional(levelN);
}

void LevelPortal::openPortal()
{
    isCalledForOpen = true;
    isCalledForClose = false;
}

void LevelPortal::closePortal()
{
    isCalledForOpen = false;
    isCalledForClose = true;
}
