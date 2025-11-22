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

void Skeleton::chasePlayer(sf::Vector2f skeletonPos, sf::Vector2f playerPos)
{
    if(skeletonPos.x < playerPos.x)
    {
        this->action_ = skeletonAction::WALKRIGHT;
    }
    
    else if(skeletonPos.x > playerPos.x)
    {
        this->action_ = skeletonAction::WALKLEFT;
    }
}

void Skeleton::tryAttackPlayer()
{
    sf::Vector2f skeletonPos = skeletonRect->getGlobalBounds().getCenter();
    sf::Vector2f playerPos = player_->playerRectangle_->getGlobalBounds().getCenter();
    float distance = std::abs(skeletonPos.x - playerPos.x);
    
    // Проверяем дистанцию для атаки
    if (distance < 80.f) {
        // TODO Наносим урон игроку
        //player_->HP_ -= 10;
        
        // Визуальный эффект при успешной атаке
        //pulseSprite(*skeletonSprite, sf::Color(255, 255, 0, 255), 3.f, sf::seconds(0.3f));
    }
}

void Skeleton::onBulletHit()
{
    isPlayingHurtAnimation = true;
    action_ = skeletonAction::HURT;
    // Сбрасываем итератор чтобы анимация началась с начала
    if((skeleton_hurt_helper.ptrToTexture==(skeleton_hurt_helper.countOfTextures)))
    {
        skeleton_hurt_helper.ptrToTexture = 0;
        skeleton_hurt_helper.iterationCounter = 0;
    }
    //Обновление переменных
        //Сбрасываем скорость хитбокса, чтобы он не бежал быстрее спрайта
    if(this->knockbacks) initialWalkSpeed = 0.f;
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
    this->distanceToMakeAttack = j["general"]["distanceToMakeAttack"];
    if(type_=="white")
    {
        this->maxWalkSpeed = j["skeleton-white"]["maxSpeed"];
        this->speed = j["skeleton-white"]["acceleration"];
        this->frictionForce = j["skeleton-white"]["friction"];
        this->HP_ = j["skeleton-white"]["HP"];
        this->knockbacks = j["skeleton-white"]["knockbacks"];
    }

    else if(type_=="yellow")
    {
        this->maxWalkSpeed = j["skeleton-yellow"]["maxSpeed"];
        this->speed = j["skeleton-yellow"]["acceleration"];
        this->frictionForce = j["skeleton-yellow"]["friction"];
        this->HP_ = j["skeleton-yellow"]["HP"];
        this->knockbacks = j["skeleton-yellow"]["knockbacks"];
    }
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
        attachTexture(gameData.skeletonWhite_idleTextures_, this->skeleton_idleTextures, gameData.skeletonWhite_idle_helper, this->skeleton_idle_helper);
        attachTexture(gameData.skeletonWhite_walkTextures, this->skeleton_walkTextures, gameData.skeletonWhite_walk_helper, this->skeleton_walk_helper);
        attachTexture(gameData.skeletonWhite_hurtTextures, this->skeleton_hurtTextures, gameData.skeletonWhite_hurt_helper, this->skeleton_hurt_helper);
        attachTexture(gameData.skeletonWhite_dieTextures, this->skeleton_dieTextures, gameData.skeletonWhite_die_helper, this->skeleton_die_helper);
        attachTexture(gameData.skeletonWhite_attack1Textures, this->skeleton_attack1Textures, gameData.skeletonWhite_attack1_helper, this->skeleton_attack1_helper);
        attachTexture(gameData.skeletonWhite_attack2Textures, this->skeleton_attack2Textures, gameData.skeletonWhite_attack2_helper, this->skeleton_attack2_helper);
        skeletonSprite = new sf::Sprite(this->skeleton_idleTextures->at(0)); //Sprite initialization
    }
    else if(type == "yellow")
    {
        attachTexture(gameData.skeletonYellow_idleTextures, this->skeleton_idleTextures, gameData.skeletonYellow_idle_helper, this->skeleton_idle_helper);
        attachTexture(gameData.skeletonYellow_walkTextures, this->skeleton_walkTextures, gameData.skeletonYellow_walk_helper, this->skeleton_walk_helper);
        attachTexture(gameData.skeletonYellow_hurtTextures, this->skeleton_hurtTextures, gameData.skeletonYellow_hurt_helper, this->skeleton_hurt_helper);
        attachTexture(gameData.skeletonYellow_dieTextures, this->skeleton_dieTextures, gameData.skeletonYellow_die_helper, this->skeleton_die_helper);
        attachTexture(gameData.skeletonYellow_attack1Textures, this->skeleton_attack1Textures, gameData.skeletonYellow_attack1_helper, this->skeleton_attack1_helper);
        attachTexture(gameData.skeletonYellow_attack2Textures, this->skeleton_attack2Textures, gameData.skeletonYellow_attack2_helper, this->skeleton_attack2_helper);
        skeletonSprite = new sf::Sprite(this->skeleton_idleTextures->at(0)); //Sprite initialization
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
    // Позиции для удобства
    sf::Vector2f skeletonPos = skeletonRect->getGlobalBounds().getCenter();
    sf::Vector2f playerPos = player_->playerRectangle_->getGlobalBounds().getCenter();
    
    float distanceX = std::abs(skeletonPos.x - playerPos.x);
    float distanceY = std::abs(skeletonPos.y - playerPos.y);

    // АТАКА: если игрок близко И скелет не атакует сейчас
    if (distanceX < this->distanceToMakeAttack && distanceY < this->distanceToMakeAttack) {
        // Проверяем что не атакуем в данный момент
        if (action_ != ATTACK1 && action_ != ATTACK2) {
            // Случайный выбор атаки
            if (rand() % 2 == 0) {
                action_ = skeletonAction::ATTACK1;
            } else {
                action_ = skeletonAction::ATTACK2;
            }
        }
    } 
    else {
        // Преследование если не атакуем
        if (action_ != ATTACK1 && action_ != ATTACK2) {
            chasePlayer(skeletonPos, playerPos);
        }
    }
}

void Skeleton::updateControl()
{
    switch(action_)
    {
        case skeletonAction::WALKLEFT:
            walkLeft();
        break;

        case skeletonAction::WALKRIGHT:
            walkRight();
        break;
        //TODO Добавить прыжки и че нить еще
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

    // Анимация получения урона
    if (isPlayingHurtAnimation && HP_ > 0) {
        pulseSprite(*skeletonSprite, sf::Color(255,0,0,255), 1.f, sf::seconds(0.2f));
        if (!switchToNextSprite(skeletonSprite, *skeleton_hurtTextures, 
        skeleton_hurt_helper, switchSprite_SwitchOption::Single)) {
            isPlayingHurtAnimation = false;
            skeletonSprite->setColor({255,255,255,255});
            action_ = skeletonAction::IDLE;
        }
        
        sf::Vector2f skeletonRectCenter = this->skeletonRect->getGlobalBounds().getCenter();
        this->skeletonSprite->setPosition({skeletonRectCenter.x,skeletonRectCenter.y-15.f});
        return;
    }
    
    // Анимация смерти
    if(isPlayingDieAnimation) {   
        skeletonSprite->setColor(sf::Color::Red);
        if (!switchToNextSprite(skeletonSprite, *skeleton_dieTextures, 
        skeleton_die_helper, switchSprite_SwitchOption::Single)) {
            isPlayingDieAnimation = false;
            isAlive = false;
        }
        
        sf::Vector2f skeletonRectCenter = this->skeletonRect->getGlobalBounds().getCenter();
        this->skeletonSprite->setPosition({skeletonRectCenter.x,skeletonRectCenter.y-15.f});
        return;
    }

    // АНИМАЦИИ АТАКИ
    if (action_ == ATTACK1 || action_ == ATTACK2) {
        bool attackFinished = false;
        
        if (action_ == ATTACK1) {
            attackFinished = !switchToNextSprite(skeletonSprite, *skeleton_attack1Textures,
                            skeleton_attack1_helper, switchSprite_SwitchOption::Single);
            
            // Наносим урон в середине анимации
            if (skeleton_attack1_helper.ptrToTexture == skeleton_attack1_helper.countOfTextures / 2) {
                tryAttackPlayer();
            }
        } else {
            attackFinished = !switchToNextSprite(skeletonSprite, *skeleton_attack2Textures,
                            skeleton_attack2_helper, switchSprite_SwitchOption::Single);
            
            // Наносим урон в середине анимации
            if (skeleton_attack2_helper.ptrToTexture == skeleton_attack2_helper.countOfTextures / 2) {
                tryAttackPlayer();
            }
        }
        
        // Завершаем атаку
        if (attackFinished) {
            action_ = skeletonAction::IDLE; // Возвращаемся в покой после атаки
        }
        
        sf::Vector2f skeletonRectCenter = this->skeletonRect->getGlobalBounds().getCenter();
        this->skeletonSprite->setPosition({skeletonRectCenter.x,skeletonRectCenter.y-15.f});
        return;
    }

    // Обычные анимации
    switch(this->action_) {
        case skeletonAction::IDLE:
            switchToNextSprite(skeletonSprite, *skeleton_idleTextures, 
                            skeleton_idle_helper, switchSprite_SwitchOption::Loop);
            break;

        case skeletonAction::WALKRIGHT:
            skeletonSprite->setScale({this->enemyScale_.x,this->enemyScale_.y});
            switchToNextSprite(skeletonSprite, *skeleton_walkTextures,     
                            skeleton_walk_helper, switchSprite_SwitchOption::Loop);   
            break;

        case skeletonAction::WALKLEFT:
            skeletonSprite->setScale({-this->enemyScale_.x,this->enemyScale_.y});
            switchToNextSprite(skeletonSprite, *skeleton_walkTextures,
                            skeleton_walk_helper, switchSprite_SwitchOption::Loop);
            break;
    }

    // Позиционирование спрайта
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
