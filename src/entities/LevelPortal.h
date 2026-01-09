#pragma once
#include<SFML/Graphics.hpp>
#include<Interactive.h>
#include<Mounting.h>
#include<GameData.h>
#include<GameLevel.h>

class GameLevelManager;

const sf::Vector2f BASE_CLOSED_SCALE = {0.f,0.f};
const sf::Vector2f BASE_OPENED_SCALE = {0.3f,0.3f};
const sf::Vector2f BASE_TARGET_SCALE = {1.f,1.f};
const sf::Vector2f BASE_ENTER_AREA_CALCULATION_SCALE = {0.1f,0.29f};
const float BASE_TARGET_PERCENT_TO_SQUISH = 1.f;

class LevelPortal : public InteractiveObject
{
private:
    // Внешние объекты
    GameLevelManager* manager;
    
        //PortalBlue
    std::vector<sf::Texture>* portalBlue1Textures;
    texturesIterHelper portalBlue1Helper;
    std::vector<sf::Texture>* portalBlue2Textures;
    texturesIterHelper portalBlue2Helper;
    std::vector<sf::Texture>* portalBlue3Textures;
    texturesIterHelper portalBlue3Helper;
    std::vector<sf::Texture>* portalBlue4Textures;
    texturesIterHelper portalBlue4Helper;
    std::vector<sf::Texture>* portalBlue5Textures;
    texturesIterHelper portalBlue5Helper;
    std::vector<sf::Texture>* portalBlue6Textures;
    texturesIterHelper portalBlue6Helper;
    std::vector<sf::Texture>* portalBlue7Textures;
    texturesIterHelper portalBlue7Helper;
    std::vector<sf::Texture>* portalBlue8Textures;
    texturesIterHelper portalBlue8Helper;

    // Все массивы и их хелперы сгруппированные вместе(указатели на них)
    std::vector< std::pair< texturesIterHelper*, std::vector<sf::Texture>**> > allPortalBlue;
    std::vector< std::pair< texturesIterHelper*, std::vector<sf::Texture>**> >::iterator allTexturesIt;

    std::optional<std::string> levelName;

    sf::Clock existClock;
    sf::Clock appearClock;
    sf::Clock disappearClock;

    const int existTime;
    const sf::Vector2f speedOfOpening;
    const sf::Vector2f speedOfClosing;
    const sf::Vector2f openedScale;
    const sf::Vector2f closedScale;
    
    sf::Transformable* squishTargetSprite;
    sf::Transformable* squishTargetRect;

    sf::Vector2f baseTargetScale;
    sf::Vector2f squishSpeed;

    bool isUsed;

    bool isCalledForOpen;
    bool isCalledForClose;
    bool isOpened;
    bool isClosed;
    bool isTargetInAreaOfTeleportation;
    bool isTargetBeingSquished;

    void portalOpeningAnimation();
    void portalClosingAnimation();
    
    // Поставить allTexturesIt в начало
    void setPortalIteratorToBegin();
    
    bool squishTargetToZero();
    void initializeSquishVars(sf::Transformable &targetSprite, sf::Transformable &targetRect);
    void resetSquishVars();
    void resetTargetVars(sf::Transformable& target);

    void teleportTargetToCenterOfPortal();
public:
    LevelPortal(const sf::Vector2f basePos, const sf::Vector2f& sOO, const sf::Vector2f& sOC, const int eT, sf::Transformable& tS, sf::Transformable& tR, GameData &gameData, GameLevelManager &m);
    ~LevelPortal() = default;

    void draw(sf::RenderWindow& window)      override;
    void update()                            override;
    void handleEvent(const sf::Event& event) override;

    void setPortalDestination(std::string levelN);

    void openPortal();
    void closePortal();

    void checkIsTargetInAreaOfTeleportation(sf::Transformable& targetCheckSprite, sf::Transformable& targetRect);

    // Getters
    bool getIsOpened();
    bool getIsClosed();
    bool getIsCalledForOpen();
    bool getIsCalledForClose();
    bool getIsInAreaOfTeleportation();
    

    // Setters

};