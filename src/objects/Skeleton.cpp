#include<Skeleton.h>
#include "Ground.h"    
#include "Platform.h"  
#include "Player.h"    

void Skeleton::checkGroundCollision(Ground& ground)
{
    float playerX = skeletonRect->getPosition().x;
    float playerY = skeletonRect->getPosition().y+skeletonRect->getSize().y;
    float groundY = ground.getRect().getPosition().y; //invalid use of incomplete type 'class Ground'GCC Enemy.h(13, 7): forward declaration of 'class Ground'

    if(playerY>=groundY)
    {
        isFalling = false;
        fallingSpeed = 0.f;
        skeletonRect->setPosition({playerX,groundY-skeletonRect->getSize().y});
    }
}

void Skeleton::checkPlatformCollision(Platform& platforms)
{
    for (auto& rectPtr : platforms.getRects())  // incomplete type "Platform" is not allowedC/C++(70) invalid use of incomplete type 'class Platform'GCC Enemy.h(14, 7): forward declaration of 'class Platform'
    {
        // Получаем глобальные границы (для удобства)
        sf::FloatRect skeletonBounds = skeletonRect->getGlobalBounds();
        sf::FloatRect platformBounds = rectPtr->getGlobalBounds();  // используем -> для shared_ptr

        // Проверяем пересечение
        if (skeletonBounds.findIntersection(platformBounds)) 
        {
            // Определяем направление коллизии
            float overlapLeft   = skeletonBounds.position.x + skeletonBounds.size.x - platformBounds.position.x;
            float overlapRight  = platformBounds.position.x + platformBounds.size.x - skeletonBounds.position.x;
            float overlapTop    = skeletonBounds.position.y + skeletonBounds.size.y - platformBounds.position.y;
            float overlapBottom = platformBounds.position.y + platformBounds.size.y - skeletonBounds.position.y;

            // Ищем минимальное перекрытие
            bool fromLeft   = (overlapLeft < overlapRight);
            bool fromTop    = (overlapTop < overlapBottom);
            float minXOverlap = fromLeft ? overlapLeft : overlapRight;
            float minYOverlap = fromTop ? overlapTop : overlapBottom;

            // Коллизия с БОКОВЫМИ сторонами
            if (minXOverlap < minYOverlap) 
            {
                if (fromLeft) {
                    // Слева
                    skeletonRect->setPosition({platformBounds.position.x - skeletonBounds.size.x, skeletonBounds.position.y});
                } else {
                    // Справа
                    skeletonRect->setPosition({platformBounds.position.x + platformBounds.size.x, skeletonBounds.position.y});
                }
            } 
            // Коллизия с ВЕРХНЕЙ/НИЖНЕЙ сторонами
            else 
            {
                if (fromTop) {
                    // Сверху
                    isFalling = false;
                    if(fallingSpeed>0.f)
                    {
                        fallingSpeed = 0.f;
                    }
                    if(fallingSpeed >= -0.1f && fallingSpeed<=0.1f)
                    {
                        if(skeletonBounds.position.y+skeletonBounds.size.y >= platformBounds.position.y+3.f)
                        {
                            skeletonRect->setPosition({skeletonBounds.position.x,skeletonBounds.position.y-2.f});
                        }
                    }
                } else {
                    // Снизу
                    skeletonRect->setPosition({skeletonBounds.position.x, platformBounds.position.y + platformBounds.size.y});
                    fallingSpeed = -(fallingSpeed);
                }
            }
        }
    }
}

void Skeleton::checkBulletCollision(Player& player)
{
    for (auto &&i : player_->bullets)
    {
        if(i->getBulletRect().getGlobalBounds().findIntersection(this->skeletonRect->getGlobalBounds()))
        {
            this->action_=skeletonAction::HURT;
            break;
        }
    }
}

void Skeleton::applyFriction(float &walkSpeed, float friction)
{
    if (walkSpeed > 0.f)
    {
        walkSpeed = std::max(0.f, walkSpeed - friction);
    }
    else if (walkSpeed < 0.f)
    {
        walkSpeed = std::min(0.f, walkSpeed + friction);
    }
    // Если walkSpeed очень мал, можно сразу установить в 0
    if (std::abs(walkSpeed) < friction * 0.5f)
    {
        walkSpeed = 0.f;
    }
}

void Skeleton::switchToNextSprite(sf::Sprite* enemySprite, std::vector<sf::Texture> &texturesArray, texturesIterHelper &iterHelper)
{
    if(iterHelper.iterationCounter<iterHelper.iterationsTillSwitch)
    {
        iterHelper.iterationCounter++;
    }
    else
    {
        //Forward-backward logic
        if(iterHelper.ptrToTexture == iterHelper.countOfTextures)
        {
            iterHelper.goForward = false;
        }
        else if(iterHelper.ptrToTexture == 0)
        {
            iterHelper.goForward = true;
        }

        //Switch sprites
        enemySprite->setTexture(texturesArray.at(iterHelper.ptrToTexture));

        //Forward-backward logic
        if(iterHelper.goForward)
        {
            iterHelper.ptrToTexture++;
        }
        else
        {
            iterHelper.ptrToTexture--;
        }

        //reset iteration counter after all switches
        iterHelper.iterationCounter = 0;
    }
}

void Skeleton::loadData()
{
    std::ifstream in("data/enemySettings.json");
    nlohmann::json j = nlohmann::json::parse(in);
    
    //Pre-load variables
    this->enemyScale_ = sf::Vector2f(j["general"]["scaleX"],j["general"]["scaleY"]);
}

Skeleton::Skeleton(GameData &gameData, sf::RenderWindow &window, Ground& ground, Platform& platform, Player& player, std::string type) : Enemy(gameData)
{
    this->window = &window;
    this->ground_ = &ground;
    this->platform_ = &platform;
    this->player_ = &player;
    this->type_=type;
    loadData();

    std::transform(type.begin(), type.end(), type.begin(),
    [](unsigned char c){ return std::tolower(c); }); //Transform "type" text to lowercase
    if(type == "white")
    {
        attachTexture(gameData.skeletonWhite_idleTextures_, this->skeletonWhite_idleTextures, gameData.skeletonWhite_idle_helper, this->skeletonWhite_idle_helper);
        attachTexture(gameData.skeletonWhite_walkTextures, this->skeletonWhite_walkTextures, gameData.skeletonWhite_walk_helper, this->skeletonWhite_walk_helper);
        attachTexture(gameData.skeletonWhite_hurtTextures, this->skeletonWhite_hurtTextures, gameData.skeletonWhite_hurt_helper, this->skeletonWhite_hurt_helper);
        attachTexture(gameData.skeletonWhite_dieTextures, this->skeletonWhite_dieTextures, gameData.skeletonWhite_die_helper, this->skeletonWhite_die_helper);
        attachTexture(gameData.skeletonWhite_attack1Textures, this->skeletonWhite_attack1Textures, gameData.skeletonWhite_attack1_helper, this->skeletonWhite_attack1_helper);
        attachTexture(gameData.skeletonWhite_attack2Textures, this->skeletonWhite_attack2Textures, gameData.skeletonWhite_attack2_helper, this->skeletonWhite_attack2_helper);
        skeletonSprite = new sf::Sprite(skeletonWhite_idleTextures->at(0)); //Sprite initialization
    }
    else if(type == "yellow")
    {
        attachTexture(gameData.skeletonYellow_idleTextures, this->skeletonYellow_idleTextures, gameData.skeletonYellow_idle_helper, this->skeletonYellow_idle_helper);
        attachTexture(gameData.skeletonYellow_walkTextures, this->skeletonYellow_walkTextures, gameData.skeletonYellow_walk_helper, this->skeletonYellow_walk_helper);
        attachTexture(gameData.skeletonYellow_hurtTextures, this->skeletonYellow_hurtTextures, gameData.skeletonYellow_hurt_helper, this->skeletonYellow_hurt_helper);
        attachTexture(gameData.skeletonYellow_dieTextures, this->skeletonYellow_dieTextures, gameData.skeletonYellow_die_helper, this->skeletonYellow_die_helper);
        attachTexture(gameData.skeletonYellow_attack1Textures, this->skeletonYellow_attack1Textures, gameData.skeletonYellow_attack1_helper, this->skeletonYellow_attack1_helper);
        attachTexture(gameData.skeletonYellow_attack2Textures, this->skeletonYellow_attack2Textures, gameData.skeletonYellow_attack2_helper, this->skeletonYellow_attack2_helper);
        skeletonSprite = new sf::Sprite(skeletonYellow_idleTextures->at(0)); //Sprite initialization
    }
    //General sprite init
    skeletonSprite->setScale(enemyScale_);
    setSpriteOriginToMiddle(*skeletonSprite);

    //General rect initialization
    skeletonRect = new sf::RectangleShape();
    float sizeX = 23.f*enemyScale_.x;
    float sizeY = 47.f*enemyScale_.y; 
    skeletonRect->setSize({sizeX,sizeY});
    skeletonRect->setFillColor(sf::Color::Red);
    skeletonRect->setPosition(this->enemyPos);
}

Skeleton::~Skeleton()
{
}

void Skeleton::updateAI() //TODO Write better skeleton's intelligence
{   
    //NOTE ALL THE CODE BELOW I WRITED ONLY FOR TEST. IT'S ALL WORKS OKAY. U CAN FREELY DELETE THIS AND WRITE OWN LOGIC. SKELETON REACTS ON BULLET'S HIT IN checkBulletCollision() METHOD.
    static int scopeIter = 0;
    if(scopeIter==0)
    {
        static int localIter = 0;
        action_ = skeletonAction::IDLE;



        localIter++;
        if(localIter==60) scopeIter++;
    } 
    else if(scopeIter==1)
    {
        static int localIter = 0;
        action_ = skeletonAction::WALKLEFT;
        this->skeletonRect->move({-1.f,-0.f});


        localIter++;
        if(localIter==60) scopeIter++;
    }
    else if(scopeIter==2)
    {
        static int localIter = 0;
        action_ = skeletonAction::WALKRIGHT;
        this->skeletonRect->move({1.f,-0.f});


        localIter++;
        if(localIter==60) scopeIter++;
    }
    else if(scopeIter==3)
    {
        static int localIter = 0;
        action_ = skeletonAction::HURT;

        localIter++;
        if(localIter==60) scopeIter++;
    }
    else if(scopeIter==4)
    {
        static int localIter = 0;
        action_ = skeletonAction::DIE;

        localIter++;
        if(localIter==60) scopeIter++;
    }
    else if(scopeIter==5)
    {
        static int localIter = 0;
        action_ = skeletonAction::ATTACK1;

        localIter++;
        if(localIter==60) scopeIter++;
    }
    else if(scopeIter==6)
    {
        static int localIter = 0;
        action_ = skeletonAction::ATTACK2;

        localIter++;
        if(localIter==60) scopeIter++;
    }

}   

void Skeleton::updatePhysics()
{
    applyFriction(initialWalkSpeed,this->frictionForce);
    
    
    skeletonRect->move({0.f,this->fallingSpeed});
    
    skeletonRect->move({initialWalkSpeed,0.f});
    
    if(skeletonRect->getPosition().y+skeletonRect->getSize().y>=WINDOW_HEIGHT)
    {
        isFalling = false;
        skeletonRect->setPosition({skeletonRect->getPosition().x, WINDOW_HEIGHT-skeletonRect->getSize().y});
    }
    else
    {
        isFalling = true;
    }
    if(isFalling)
    {
        fallingSpeed+=0.1f;
    }
    else
    {
        fallingSpeed = 0.f;
    }
    //std::cout << playerRectangle->getPosition().x << std::endl;

    //Window border collision (left and right)
    if(skeletonRect->getPosition().x+skeletonRect->getSize().x>=WINDOW_WIDTH)
    {
        skeletonRect->setPosition({WINDOW_WIDTH-skeletonRect->getSize().x,skeletonRect->getPosition().y});
    }
    else if(skeletonRect->getPosition().x<=0)
    {
        skeletonRect->setPosition({0.f,skeletonRect->getPosition().y});
    }
    //NOTE those methods must be always last
    checkGroundCollision(*ground_);
    checkPlatformCollision(*platform_);
    checkBulletCollision(*player_);
}

void Skeleton::updateTextures()
{
    
    switch(this->action_)
    {
        case skeletonAction::IDLE:
            switchToNextSprite(skeletonSprite,*skeletonWhite_idleTextures,*skeletonWhite_idle_helper);
        break;

        case skeletonAction::WALKLEFT:
            switchToNextSprite(skeletonSprite,*skeletonWhite_walkTextures,*skeletonWhite_walk_helper);
        break;

        case skeletonAction::WALKRIGHT:
            switchToNextSprite(skeletonSprite,*skeletonWhite_walkTextures,*skeletonWhite_walk_helper);
        break;

        case skeletonAction::HURT:
            switchToNextSprite(skeletonSprite,*skeletonWhite_hurtTextures,*skeletonWhite_hurt_helper);
        break;

        case skeletonAction::DIE:
            switchToNextSprite(skeletonSprite,*skeletonWhite_dieTextures,*skeletonWhite_die_helper);
        break;

        case skeletonAction::ATTACK1:
            switchToNextSprite(skeletonSprite,*skeletonWhite_attack1Textures,*skeletonWhite_attack1_helper);
        break;

        case skeletonAction::ATTACK2:
            switchToNextSprite(skeletonSprite,*skeletonWhite_attack2Textures,*skeletonWhite_attack2_helper);
        break;
    }
    sf::Vector2f skeletonRectCenter = this->skeletonRect->getGlobalBounds().getCenter();
    this->skeletonSprite->setPosition({skeletonRectCenter.x,skeletonRectCenter.y-15.f});
}

void Skeleton::draw()
{
    window->draw(*skeletonRect);
    window->draw(*skeletonSprite);
}
