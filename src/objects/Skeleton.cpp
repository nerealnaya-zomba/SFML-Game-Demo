#include<Skeleton.h>

void Skeleton::checkGroundCollision(Ground& ground)
{
    float playerX = skeletonRect->getPosition().x;
    float playerY = skeletonRect->getPosition().y+skeletonRect->getSize().y;
    float groundY = ground.getRect().getPosition().y;

    if(playerY>=groundY)
    {
        isFalling = false;
        fallingSpeed = 0.f;
        skeletonRect->setPosition({playerX,groundY-skeletonRect->getSize().y});
    }
}

void Skeleton::checkPlatformCollision(Platform& platforms)
{
    for (auto& rectPtr : platforms.getRects())  // меняем тип итератора
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

Skeleton::Skeleton(GameData &gameData, sf::RenderWindow &window, Ground& ground, Platform& platform, std::string type) : Enemy(gameData)
{
    this->window = &window;
    this->ground_ = &ground;
    this->platform_ = &platform;
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
    float sizeX = 37.f*enemyScale_.x;
    float sizeY = 53.f*enemyScale_.y; //FIXME Скелет не появляется, перед этим я добавил в enemySEttings scale и настроил подстройку rect и sprite под эти значения, чтобюы все совпадало. До этого скелет появлялся, сейчас ваще блин его нету.
    skeletonRect->setSize({sizeX,sizeY});
    skeletonRect->setFillColor(sf::Color::Red);
    skeletonRect->setPosition(this->enemyPos);
}

Skeleton::~Skeleton()
{
}

void Skeleton::updateAI()
{

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
}

void Skeleton::updateTextures()
{
    switchToNextSprite(skeletonSprite,*skeletonWhite_idleTextures,*skeletonWhite_idle_helper);

    this->skeletonSprite->setPosition(this->skeletonRect->getGlobalBounds().getCenter());
}

void Skeleton::draw()
{
    window->draw(*skeletonRect);
    window->draw(*skeletonSprite);
}
