#include<LevelPortal.h>
#include<math.h>

void LevelPortal::portalOpeningAnimation()
{
    sf::Vector2f currentScale = sprite->getScale();
    
    // Если ещё не полностью открыт (сравниваем с openedScale)
    if (currentScale.x < openedScale.x || currentScale.y < openedScale.y) {
        
        sf::Vector2f newScale = {
            currentScale.x+speedOfOpening.x,
            currentScale.y+speedOfOpening.y
        };
        sprite->setScale(newScale);
    }
    // Если открыт
    else if((currentScale.x >= openedScale.x || currentScale.y >= openedScale.y))
    {
        sprite->setScale(openedScale);
        isOpened = true;
        isClosed = false;
        isCalledForOpen = false;
        isCalledForClose = false;
    }
}

void LevelPortal::portalClosingAnimation()
{
    sf::Vector2f currentScale = sprite->getScale();
    
    // Если ещё не полностью закрыт (сравниваем с closedScale)
    if (currentScale.x > closedScale.x || currentScale.y > closedScale.y) {
        
        sf::Vector2f newScale = {
            currentScale.x-speedOfClosing.x,
            currentScale.y-speedOfClosing.y
        };
        sprite->setScale(newScale);
    }
    // Если закрыт
    else if(currentScale.x <= closedScale.x || currentScale.y <= closedScale.y)
    {
        sprite->setScale(closedScale);
        isOpened = false;
        isClosed = true;
        isCalledForClose = false;
        isCalledForOpen = false;
    }
}

LevelPortal::LevelPortal(const sf::Vector2f basePos, const sf::Vector2f& sOO, const sf::Vector2f& sOC, const int eT, GameData &gameData, GameLevelManager &m)
    : InteractiveObject(basePos), manager(&m), isUsed(false), speedOfOpening(sOO), speedOfClosing(sOC), existTime(eT), isOpened(false), isClosed(true),  
    isCalledForClose(false), isCalledForOpen(false), isTargetInAreaOfTeleportation(false), isTargetBeingSquished(false), openedScale(BASE_OPENED_SCALE), closedScale(BASE_CLOSED_SCALE),
    baseTargetScale(BASE_TARGET_SCALE)
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

    
    // Инициализация спрайтов
    setSpriteOriginToMiddle(*sprite);

    sprite->setScale({0.f,0.f});

    sprite->setPosition(basePos);

    // Группируем массивы текстур
    allPortalBlue.push_back(std::pair(&portalBlue1Helper,&portalBlue1Textures));
    allPortalBlue.push_back(std::pair(&portalBlue2Helper,&portalBlue2Textures));
    allPortalBlue.push_back(std::pair(&portalBlue3Helper,&portalBlue3Textures));
    allPortalBlue.push_back(std::pair(&portalBlue4Helper,&portalBlue4Textures));
    allPortalBlue.push_back(std::pair(&portalBlue5Helper,&portalBlue5Textures));
    allPortalBlue.push_back(std::pair(&portalBlue6Helper,&portalBlue6Textures));
    allPortalBlue.push_back(std::pair(&portalBlue7Helper,&portalBlue7Textures));
    allPortalBlue.push_back(std::pair(&portalBlue8Helper,&portalBlue8Textures));

    allTexturesIt = allPortalBlue.begin();
}

void LevelPortal::draw(sf::RenderWindow &window)
{
    // Выйти если не выполняется ни одно из следующих условий
    if(!(isOpened || isCalledForOpen || isCalledForClose)) return;

    
    if(allTexturesIt == (allPortalBlue.end()-1))
    {
        gameUtils::switchToNextSprite(sprite.get(),**allTexturesIt->second,*allTexturesIt->first,switchSprite_SwitchOption::Loop);
        window.draw(*sprite);
        return;
    }
    else if(!gameUtils::switchToNextSprite(sprite.get(),**allTexturesIt->second,*allTexturesIt->first,switchSprite_SwitchOption::Single))
    {
        allTexturesIt++;
    }
    window.draw(*sprite);
    
}

void LevelPortal::update()
{
    if(isCalledForOpen) portalOpeningAnimation();
    if(isCalledForClose) portalClosingAnimation();
    if(isClosed) setPortalIteratorToBegin();

    if(isOpened)
    {
        if(isTargetInAreaOfTeleportation){
            if(squishTargetToZero())
            {
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

void LevelPortal::checkIsTargetInAreaOfTeleportation(sf::Transformable& target)
{
    if(isInAreaOfInteraction(target.getPosition()))
    {
        isTargetInAreaOfTeleportation = true;
    }   else 
        isTargetInAreaOfTeleportation = false;

    if(isTargetInAreaOfTeleportation &&  !isTargetBeingSquished)
    {
        initializeSquishVars(target);
    }

    if (!isTargetInAreaOfTeleportation)
    {
        resetTargetVars(target);
        resetSquishVars();
    }
}

bool LevelPortal::getIsOpened()
{
    return this->isOpened;
}

bool LevelPortal::getIsClosed()
{
    return this->isClosed;
}

bool LevelPortal::getIsCalledForOpen()
{
    return this->isCalledForOpen;
}

bool LevelPortal::getIsCalledForClose()
{
    return this->isCalledForClose;
}

void LevelPortal::setPortalIteratorToBegin()
{
    this->allTexturesIt = allPortalBlue.begin();
}

bool LevelPortal::squishTargetToZero()
{
    sf::Vector2f currentScale = squishTarget->getScale();

    if(currentScale.x >= -0.3f && currentScale.x <= 0.3f )
    {
        squishTarget->setScale({0.f,0.f});
        isTargetBeingSquished = false;
        return true;
    }

    squishTarget->setScale({
        currentScale.x-(squishSpeed.x),
        currentScale.y
    });

    isTargetBeingSquished = true;
    return false;
}

void LevelPortal::initializeSquishVars(sf::Transformable &target)
{
    this->squishTarget = &target;

    baseTargetScale = target.getScale();
    squishSpeed = {
        (baseTargetScale.x / 100) * BASE_TARGET_PERCENT_TO_SQUISH,
        baseTargetScale.y
    };
}

void LevelPortal::resetSquishVars()
{
    squishSpeed         = {0.f,0.f};
    
    isTargetBeingSquished         = false;
    isTargetInAreaOfTeleportation = false;
}

void LevelPortal::resetTargetVars(sf::Transformable& target)
{
    target.setScale(baseTargetScale);

    this->squishTarget  = nullptr;
    baseTargetScale     = {BASE_TARGET_SCALE};
}
