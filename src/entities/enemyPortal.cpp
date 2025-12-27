#include<enemyPortal.h>

void enemyPortal::sizeUp()
{
    float sizeUpSpeedX =  PORTAL_BASE_SCALE.x / (PORTAL_EXIST_TIME/2);
    float sizeUpSpeedY =  PORTAL_BASE_SCALE.y / (PORTAL_EXIST_TIME/2);

    sf::Vector2f portalSpriteScale = this->portalSprite->getScale();

    this->portalSprite->setScale({
        portalSpriteScale.x + sizeUpSpeedX,
        portalSpriteScale.y + sizeUpSpeedY
    });
}

void enemyPortal::sizeDown()
{
    float sizeDownSpeedX = PORTAL_BASE_SCALE.x / (PORTAL_EXIST_TIME / 2);
    float sizeDownSpeedY = PORTAL_BASE_SCALE.y / (PORTAL_EXIST_TIME / 2);

    sf::Vector2f portalSpriteScale = this->portalSprite->getScale();

    this->portalSprite->setScale({
        portalSpriteScale.x - sizeDownSpeedX,
        portalSpriteScale.y - sizeDownSpeedY
    });
}

enemyPortal::enemyPortal(GameData& data, sf::Vector2f position)
{
    // Data
    this->data = &data;

    // Start clock
    this->portalClock.start();

    // Portal sprite
    this->portalSprite = new sf::Sprite(data.portalGreenTextures.at(0));
    this->portalSprite->setScale(PORTAL_BASE_SCALE);
    this->portalSprite->setPosition(position);
    
    // Portal helper
    this->portalHelper = data.portalGreen;
}

enemyPortal::~enemyPortal()
{
}

void enemyPortal::update()
{
    if(this->portalClock.getElapsedTime().asMilliseconds()<=(PORTAL_EXIST_TIME/2))
    {
        this->sizeUp();
    }   // Sizing up
    else
    {
        this->sizeDown();
    }   // Sizing down

    // Set isExist to false, if portal's time passed
    if(this->portalClock.getElapsedTime().asMilliseconds()>=PORTAL_EXIST_TIME) isExist = false;
    
    // Set isHalfPassed to true, if half of time passed
    if(this->portalClock.getElapsedTime().asMilliseconds()>=(PORTAL_EXIST_TIME/2)) isHalfPassed = true;
}

void enemyPortal::updateTextures()
{
    gameUtils::switchToNextSprite(this->portalSprite,data->portalGreenTextures,this->portalHelper,switchSprite_SwitchOption::Single);
}

void enemyPortal::draw(sf::RenderWindow &window)
{
    window.draw(*this->portalSprite);
}

bool enemyPortal::getIsExist()
{
    return this->isExist;
}
