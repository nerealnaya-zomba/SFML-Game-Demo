#include<Skeleton.h>
#include "Ground.h"    
#include "Platform.h"  
#include "Player.h"

void Skeleton::walkLeft()
{
    if(initialWalkSpeed<=(-maxWalkSpeed) || isPlayingHurtAnimation)
    {
        return; 
    }
    initialWalkSpeed-=speed;
}

void Skeleton::walkRight()
{
    if(initialWalkSpeed>=maxWalkSpeed || isPlayingHurtAnimation)
    {
        return;
    }
    initialWalkSpeed+=speed;
}

void Skeleton::onBulletHit()
{
    isPlayingHurtAnimation = true;
    action_ = skeletonAction::HURT;
    // Сбрасываем итератор чтобы анимация началась с начала
    if((skeletonWhite_hurt_helper.ptrToTexture==(skeletonWhite_hurt_helper.countOfTextures)))
    {
        skeletonWhite_hurt_helper.ptrToTexture = 0;
        skeletonWhite_hurt_helper.iterationCounter = 0;
    }
    //Обновление переменных
        //Сбрасываем скорость хитбокса, чтобы он не бежал быстрее спрайта
    initialWalkSpeed = 0.f;
        //Уменьшаем здоровье
    HP_-=player_->DMG_;
}

void Skeleton::checkGroundCollision(Ground &ground)
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
    for (auto it = player_->bullets.begin(); it != player_->bullets.end(); ) {
        if((*it)->getBulletRect().getGlobalBounds().findIntersection(this->skeletonRect->getGlobalBounds())) {
            onBulletHit();
            // Удаляем пулю
            it = player_->bullets.erase(it);
            // Убавить здоровье
            // ...
        } else {
            ++it;
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

bool Skeleton::switchToNextSprite(sf::Sprite* enemySprite,
        std::vector<sf::Texture>& texturesArray, 
        texturesIterHelper& iterHelper, 
        switchSprite_SwitchOption option)
{
    if(iterHelper.iterationCounter < iterHelper.iterationsTillSwitch)
    {
        iterHelper.iterationCounter++;
        return true; // анимация еще идет
    }

    // Переключаем текстуру
    enemySprite->setTexture(texturesArray.at(iterHelper.ptrToTexture));
    iterHelper.ptrToTexture++;
    iterHelper.iterationCounter = 0;

    // Достигли конца анимации
    if(iterHelper.ptrToTexture >= iterHelper.countOfTextures)
    {
        iterHelper.ptrToTexture = 0;
        
        if(option == switchSprite_SwitchOption::Single)
            return false; // Single анимация ЗАВЕРШЕНА
        // Loop продолжается автоматически
    }

    return true; // анимация еще идет
}

void Skeleton::loadData()
{
    std::ifstream in("data/enemySettings.json");
    nlohmann::json j = nlohmann::json::parse(in);
    
    //Подгружаемые из enemySettings.json значения переменных
    this->enemyScale_ = sf::Vector2f(j["general"]["scaleX"],j["general"]["scaleY"]);
    this->maxWalkSpeed = j["skeleton-white"]["maxSpeed"];
    this->speed = j["skeleton-white"]["acceleration"];
    this->frictionForce = j["skeleton-white"]["friction"];
    this->HP_ = j["skeleton-white"]["HP"];
}

Skeleton::Skeleton(GameData &gameData, sf::RenderWindow &window, Ground& ground, Platform& platform, Player& player, std::string type, sf::Vector2f pos) : Enemy(gameData)
{
    this->window = &window;
    this->ground_ = &ground;
    this->platform_ = &platform;
    this->player_ = &player;
    this->type_=type;
    this->enemyPos = pos;
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
    sf::Vector2f skeletonRectCenter = this->skeletonRect->getGlobalBounds().getCenter();
    this->skeletonSprite->setPosition({skeletonRectCenter.x,skeletonRectCenter.y-15.f});

    //Health bar initialization
    this->healthbar = new HealthBar(skeletonRect, window, sf::Color::Red, sf::Color::Green, {50.f, 5.f}, this->HP_,{0.f,-50.f});

}

Skeleton::~Skeleton()
{
    delete this->skeletonSprite;
    delete this->skeletonRect;
}

void Skeleton::updateAI() //TODO Write better skeleton's intelligence
{
    //Позиции для удобства
    sf::Vector2f skeletonPos = skeletonRect->getGlobalBounds().getCenter();
    sf::Vector2f playerPos = player_->playerRectangle_->getGlobalBounds().getCenter();

    if(skeletonPos.x < playerPos.x)
    {
        walkRight();
        this->action_ = skeletonAction::WALKRIGHT;
    }
    
    else if(skeletonPos.x > playerPos.x)
    {
        walkLeft();
        this->action_ = skeletonAction::WALKLEFT;
    }
    //TODO Написать логику, чтобы скелет перепрыгивал платформы



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
    
    if(HP_<=0)
    {
        isPlayingDieAnimation = true;
    }

    //NOTE those methods must be always last
    checkGroundCollision(*ground_);
    checkPlatformCollision(*platform_);
    if(HP_>0) checkBulletCollision(*player_);
    healthbar->update(HP_);
}

void Skeleton::updateTextures()
{
    if(!isAlive) return;

    if (isPlayingHurtAnimation && HP_ > 0) {
        // Проигрываем HURT анимацию один раз
        pulseSprite(*skeletonSprite,sf::Color(255,0,0,255),1.f,sf::seconds(0.2f)); // Пульсация
        if (!switchToNextSprite(skeletonSprite, *skeletonWhite_hurtTextures, 
        skeletonWhite_hurt_helper, switchSprite_SwitchOption::Single)) {
            // Анимация завершена - возвращаем обычное состояние
            isPlayingHurtAnimation = false;
            skeletonSprite->setColor({255,255,255,255});
            action_ = skeletonAction::IDLE; // или предыдущее действие
        }
        return; // Перекрываем другие анимации
    }
    if(isPlayingDieAnimation)
    {
        // static int isCalled = 0;
        // if(isCalled != 15 )
        // {
        //     pulseSprite(*skeletonSprite,sf::Color(255,0,0,255),1.f,sf::seconds(0.1f));
        //     isCalled++;
        // } 
        // if(isCalled == 15 ) skeletonSprite->setColor({255,255,255,255}); //NOTE Остановился тут
        
        skeletonSprite->setColor(sf::Color::Red);

        if (!switchToNextSprite(skeletonSprite, *skeletonWhite_dieTextures, 
        skeletonWhite_die_helper, switchSprite_SwitchOption::Single))
        {
            isPlayingDieAnimation = false;
            isAlive = false;
        }

        return;
    }

    
    // Обычные анимации
    switch(this->action_) {
        case skeletonAction::IDLE:
            switchToNextSprite(skeletonSprite, *skeletonWhite_idleTextures, 
                            skeletonWhite_idle_helper, switchSprite_SwitchOption::Loop);
        break;

        case skeletonAction::WALKRIGHT:
            skeletonSprite->setScale({this->enemyScale_.x,this->enemyScale_.y});
            switchToNextSprite(skeletonSprite, *skeletonWhite_walkTextures,     
                            skeletonWhite_walk_helper, switchSprite_SwitchOption::Loop);   
        break;

        case skeletonAction::WALKLEFT:
            skeletonSprite->setScale({-this->enemyScale_.x,this->enemyScale_.y});
            switchToNextSprite(skeletonSprite, *skeletonWhite_walkTextures,
                            skeletonWhite_walk_helper, switchSprite_SwitchOption::Loop);
        break;
    }

    //NOTE do not remove this. This thing moves sprite to the skeletonRect(hitbox)
    sf::Vector2f skeletonRectCenter = this->skeletonRect->getGlobalBounds().getCenter();
    this->skeletonSprite->setPosition({skeletonRectCenter.x,skeletonRectCenter.y-15.f});
}

void Skeleton::draw()
{
    //window->draw(*skeletonRect); //REMOVELATER Only for skeleton hitbox overview.
    window->draw(*skeletonSprite);
    healthbar->draw();
}

sf::RectangleShape &Skeleton::getRect()
{
    return *this->skeletonRect;
}

int Skeleton::getHP()
{
    return this->HP_;
}
