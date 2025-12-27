#include <Skeleton.h>
#include "Ground.h"    
#include "Platform.h"  
#include "Player.h"

// ========== ДВИЖЕНИЕ ==========
void Skeleton::walkLeft() {
    if (initialWalkSpeed <= -maxWalkSpeed || isPlayingHurtAnimation) return; 
    initialWalkSpeed -= speed;
}

void Skeleton::walkRight() {
    if (initialWalkSpeed >= maxWalkSpeed || isPlayingHurtAnimation) return;
    initialWalkSpeed += speed;
}

// ========== ПРЕСЛЕДОВАНИЕ ИГРОКА ==========
void Skeleton::chasePlayer(sf::Vector2f skeletonPos, sf::Vector2f playerPos) {
    if (skeletonPos.x < playerPos.x) {
        action_ = WALKRIGHT;
    } else if (skeletonPos.x > playerPos.x) {
        action_ = WALKLEFT;
    }
}

// ========== ПАТРУЛИРОВАНИЕ ==========
void Skeleton::patrol() {
    currentSkeletonPos = skeletonRect->getGlobalBounds().getCenter().x;
    
    // Первый вход в патрулирование
    if (isFirstEnter) {
        isFirstEnter = false;
        leftExplored = false;
        rightExplored = false;
        
        // Случайный выбор начального направления
        if (rand() % 2 == 0) {
            explorationState = EXPLORE_LEFT;
            action_ = WALKLEFT;
        } else {
            explorationState = EXPLORE_RIGHT;
            action_ = WALKRIGHT;
        }
        
        exploreStartPos = currentSkeletonPos;
        deadEndCheckTimer.restart();
        return;
    }
    
    // Фаза исследования
    if (explorationState != PATROLLING) {
        // Проверка на тупик
        if (deadEndCheckTimer.getElapsedTime().asMilliseconds() >= TIME_TO_CHECK_DEADEND) {
            float distanceMoved = std::abs(currentSkeletonPos - exploreStartPos);
            
            if (distanceMoved < MIN_DISTANCE_FOR_DEADEND) {
                // Обнаружен тупик
                if (explorationState == EXPLORE_LEFT && !leftExplored) {
                    leftBound = currentSkeletonPos;
                    leftExplored = true;
                } else if (explorationState == EXPLORE_RIGHT && !rightExplored) {
                    rightBound = currentSkeletonPos;
                    rightExplored = true;
                }
                
                // Если обе стороны исследованы
                if (leftExplored && rightExplored) {
                    explorationState = PATROLLING;
                    // Выбор начального направления патрулирования
                    action_ = (currentSkeletonPos > (leftBound + rightBound) / 2.0f) ? WALKLEFT : WALKRIGHT;
                } else {
                    // Исследование другой стороны
                    explorationState = !leftExplored ? EXPLORE_LEFT : EXPLORE_RIGHT;
                    action_ = (explorationState == EXPLORE_LEFT) ? WALKLEFT : WALKRIGHT;
                    
                    exploreStartPos = currentSkeletonPos;
                    deadEndCheckTimer.restart();
                }
            } else {
                // Продолжаем движение
                exploreStartPos = currentSkeletonPos;
                deadEndCheckTimer.restart();
            }
        }
    } 
    // Фаза патрулирования между границами
    else {
        if (!recentlySwitchedDirection) {
            // Проверка достижения границ
            if (action_ == WALKLEFT && currentSkeletonPos <= leftBound + DIRECTION_SWITCH_OFFSET) {
                action_ = WALKRIGHT;
                recentlySwitchedDirection = true;
                directionSwitchTimer.restart();
            } else if (action_ == WALKRIGHT && currentSkeletonPos >= rightBound - DIRECTION_SWITCH_OFFSET) {
                action_ = WALKLEFT;
                recentlySwitchedDirection = true;
                directionSwitchTimer.restart();
            }
        }
        
        // Сброс флага смены направления
        if (recentlySwitchedDirection && 
            directionSwitchTimer.getElapsedTime().asMilliseconds() >= DIRECTION_SWITCH_COOLDOWN) {
            recentlySwitchedDirection = false;
        }
    }
}

// ========== СБРОС ПЕРЕМЕННЫХ ПАТРУЛИРОВАНИЯ ==========
void Skeleton::makeRandomPatrolVariables() {
    if (makeRandomStart != isPlayerOutOfReach) {
        makeRandomStart = isPlayerOutOfReach;
        isFirstEnter = true;
        explorationState = EXPLORE_NONE;
        leftExplored = false;
        rightExplored = false;
        leftBound = 0.0f;
        rightBound = 0.0f;
    }
}

// ========== АТАКА ИГРОКА ==========
void Skeleton::tryAttackPlayer() {
    sf::Vector2f skeletonPos = skeletonRect->getGlobalBounds().getCenter();
    sf::Vector2f playerPos = player_->playerRectangle_->getGlobalBounds().getCenter();
    float distanceX = std::abs(skeletonPos.x - playerPos.x);
    float distanceY = std::abs(skeletonPos.y - playerPos.y);

    if (distanceX < distanceToHit_byAttack && distanceY < skeletonRect->getSize().y) {
        bool hitSide = (skeletonPos.x > playerPos.x);
        player_->takeDMG(DMG_, knockback_, hitSide);
    }
}

// ========== ОБРАБОТКА ПОПАДАНИЯ ПУЛИ ==========
void Skeleton::onBulletHit() {
    isPlayingHurtAnimation = true;
    action_ = HURT;
    
    // Сброс анимации
    if (skeleton_hurt_helper.ptrToTexture == skeleton_hurt_helper.countOfTextures) {
        skeleton_hurt_helper.ptrToTexture = 0;
        skeleton_hurt_helper.iterationCounter = 0;
    }
    
    // Сброс скорости при отталкивании
    if (knockbacks) initialWalkSpeed = 0.f;
    
    // Уменьшение здоровья
    HP_ -= player_->DMG_;
}

// ========== КОЛЛИЗИИ С ЗЕМЛЕЙ ==========
void Skeleton::checkGroundCollision(Ground &ground) {
    float skeletonX = skeletonRect->getPosition().x;
    float skeletonBottom = skeletonRect->getPosition().y + skeletonRect->getSize().y;
    float groundTop = ground.getRect().getPosition().y;

    if (skeletonBottom >= groundTop) {
        isFalling = false;
        fallingSpeed = 0.f;
        skeletonRect->setPosition({skeletonX, groundTop - skeletonRect->getSize().y});
    }
}

// ========== КОЛЛИЗИИ С ПЛАТФОРМАМИ ==========
void Skeleton::checkPlatformCollision(Platform& platforms) {
    for (auto& rectPtr : platforms.getRects()) {
        sf::FloatRect skeletonBounds = skeletonRect->getGlobalBounds();
        sf::FloatRect platformBounds = rectPtr->getGlobalBounds();

        if (skeletonBounds.findIntersection(platformBounds)) {
            // Расчет перекрытий
            float overlapLeft = skeletonBounds.position.x + skeletonBounds.size.x - platformBounds.position.x;
            float overlapRight = platformBounds.position.x + platformBounds.size.x - skeletonBounds.position.x;
            float overlapTop = skeletonBounds.position.y + skeletonBounds.size.y - platformBounds.position.y;
            float overlapBottom = platformBounds.position.y + platformBounds.size.y - skeletonBounds.position.y;

            bool fromLeft = (overlapLeft < overlapRight);
            bool fromTop = (overlapTop < overlapBottom);
            float minXOverlap = fromLeft ? overlapLeft : overlapRight;
            float minYOverlap = fromTop ? overlapTop : overlapBottom;

            // Боковые коллизии
            if (minXOverlap < minYOverlap) {
                if (fromLeft) {
                    skeletonRect->setPosition({platformBounds.position.x - skeletonBounds.size.x, skeletonBounds.position.y});
                } else {
                    skeletonRect->setPosition({platformBounds.position.x + platformBounds.size.x, skeletonBounds.position.y});
                }
            } 
            // Вертикальные коллизии
            else {
                if (fromTop) {
                    isFalling = false;
                    if (fallingSpeed > 0.f) fallingSpeed = 0.f;
                    
                    if (fallingSpeed >= -0.1f && fallingSpeed <= 0.1f) {
                        if (skeletonBounds.position.y + skeletonBounds.size.y >= platformBounds.position.y + 3.f) {
                            skeletonRect->setPosition({skeletonBounds.position.x, skeletonBounds.position.y - 2.f});
                        }
                    }
                } else {
                    skeletonRect->setPosition({skeletonBounds.position.x, platformBounds.position.y + platformBounds.size.y});
                    fallingSpeed = -(fallingSpeed);
                }
            }
        }
    }
}

// ========== КОЛЛИЗИИ С ПУЛЯМИ ==========
void Skeleton::checkBulletCollision(Player& player) {
    for (auto it = player_->bullets.begin(); it != player_->bullets.end(); ++it) {
        if ((*it)->getBulletRect().getGlobalBounds().findIntersection(skeletonRect->getGlobalBounds()) 
            && !(*it)->isSheduledToBeDestroyed) {
            (*it)->isSheduledToBeDestroyed = true;
            onBulletHit();
        }
    }
}

// ========== ТРЕНИЕ ==========
void Skeleton::applyFriction(float &walkSpeed, float friction) {
    if (walkSpeed > 0.f) {
        walkSpeed = std::max(0.f, walkSpeed - friction);
    } else if (walkSpeed < 0.f) {
        walkSpeed = std::min(0.f, walkSpeed + friction);
    }
    
    if (std::abs(walkSpeed) < friction * 0.5f) {
        walkSpeed = 0.f;
    }
}

// ========== ЗАГРУЗКА ДАННЫХ ==========
void Skeleton::loadData() {
    std::ifstream in("data/enemySettings.json");
    nlohmann::json j = nlohmann::json::parse(in);
    
    enemyScale_ = sf::Vector2f(j["general"]["scaleX"], j["general"]["scaleY"]);
    distanceToMakeAttack = j["general"]["distanceToMakeAttack"];
    distanceToHit_byAttack = j["general"]["distanceToHit_byAttack"];
    
    if (type_ == "white") {
        maxWalkSpeed = random(1.5f, 3.0f);
        speed = j["skeleton-white"]["acceleration"];
        frictionForce = j["skeleton-white"]["friction"];
        HP_ = j["skeleton-white"]["HP"];
        DMG_ = j["skeleton-white"]["DMG"];
        knockback_ = sf::Vector2f(j["skeleton-white"]["KnockbackX"], j["skeleton-white"]["KnockbackY"]);
        knockbacks = j["skeleton-white"]["knockbacks"];
    } else if (type_ == "yellow") {
        maxWalkSpeed = j["skeleton-yellow"]["maxSpeed"];
        speed = j["skeleton-yellow"]["acceleration"];
        frictionForce = j["skeleton-yellow"]["friction"];
        HP_ = j["skeleton-yellow"]["HP"];
        DMG_ = j["skeleton-yellow"]["DMG"];
        knockback_ = {j["skeleton-yellow"]["KnockbackX"], j["skeleton-white"]["KnockbackY"]};
        knockbacks = j["skeleton-yellow"]["knockbacks"];
    }
}

// ========== КОНСТРУКТОР ==========
Skeleton::Skeleton(GameData &gameData, sf::RenderWindow &window, Ground& ground, 
                   Platform& platform, Player& player, std::string type, sf::Vector2f pos) 
    : Enemy(gameData) {
    
    this->window = &window;
    this->ground_ = &ground;
    this->platform_ = &platform;
    this->player_ = &player;
    this->type_ = type;
    this->enemyPos = pos;
    
    loadData();

    std::transform(type.begin(), type.end(), type.begin(), ::tolower);
    
    // Загрузка текстур в зависимости от типа
    if (type == "white") {
        attachTexture(gameData.skeletonWhite_idleTextures_, skeleton_idleTextures, 
                     gameData.skeletonWhite_idle_helper, skeleton_idle_helper);
        attachTexture(gameData.skeletonWhite_walkTextures, skeleton_walkTextures, 
                     gameData.skeletonWhite_walk_helper, skeleton_walk_helper);
        attachTexture(gameData.skeletonWhite_hurtTextures, skeleton_hurtTextures, 
                     gameData.skeletonWhite_hurt_helper, skeleton_hurt_helper);
        attachTexture(gameData.skeletonWhite_dieTextures, skeleton_dieTextures, 
                     gameData.skeletonWhite_die_helper, skeleton_die_helper);
        attachTexture(gameData.skeletonWhite_attack1Textures, skeleton_attack1Textures, 
                     gameData.skeletonWhite_attack1_helper, skeleton_attack1_helper);
        attachTexture(gameData.skeletonWhite_attack2Textures, skeleton_attack2Textures, 
                     gameData.skeletonWhite_attack2_helper, skeleton_attack2_helper);
        
        skeletonSprite = new sf::Sprite(skeleton_idleTextures->at(0));
    } else if (type == "yellow") {
        attachTexture(gameData.skeletonYellow_idleTextures, skeleton_idleTextures, 
                     gameData.skeletonYellow_idle_helper, skeleton_idle_helper);
        attachTexture(gameData.skeletonYellow_walkTextures, skeleton_walkTextures, 
                     gameData.skeletonYellow_walk_helper, skeleton_walk_helper);
        attachTexture(gameData.skeletonYellow_hurtTextures, skeleton_hurtTextures, 
                     gameData.skeletonYellow_hurt_helper, skeleton_hurt_helper);
        attachTexture(gameData.skeletonYellow_dieTextures, skeleton_dieTextures, 
                     gameData.skeletonYellow_die_helper, skeleton_die_helper);
        attachTexture(gameData.skeletonYellow_attack1Textures, skeleton_attack1Textures, 
                     gameData.skeletonYellow_attack1_helper, skeleton_attack1_helper);
        attachTexture(gameData.skeletonYellow_attack2Textures, skeleton_attack2Textures, 
                     gameData.skeletonYellow_attack2_helper, skeleton_attack2_helper);
        
        skeletonSprite = new sf::Sprite(skeleton_idleTextures->at(0));
    }
    
    // Инициализация спрайта
    skeletonSprite->setScale(enemyScale_);
    setSpriteOriginToMiddle(*skeletonSprite);

    // Инициализация хитбокса
    skeletonRect = new sf::RectangleShape();
    float sizeX = 23.f * enemyScale_.x;
    float sizeY = 47.f * enemyScale_.y; 
    skeletonRect->setSize({sizeX, sizeY});
    skeletonRect->setFillColor(sf::Color::Red);
    skeletonRect->setPosition(enemyPos);
    
    sf::Vector2f rectCenter = skeletonRect->getGlobalBounds().getCenter();
    skeletonSprite->setPosition({rectCenter.x, rectCenter.y - 15.f});

    // Полоска здоровья
    healthbar = new HealthBar(skeletonRect, window, sf::Color::Red, sf::Color::Green, 
                             {50.f, 5.f}, HP_, {0.f, -50.f});
}

// ========== ДЕСТРУКТОР ==========
Skeleton::~Skeleton() {
    delete skeletonSprite;
    delete skeletonRect;
}

// ========== ОБНОВЛЕНИЕ ИИ ==========
void Skeleton::updateAI() {
    if (!isAlive) return;
    
    sf::Vector2f skeletonPos = skeletonRect->getGlobalBounds().getCenter();
    sf::Vector2f playerPos = player_->playerRectangle_->getGlobalBounds().getCenter();
    float distanceX = std::abs(skeletonPos.x - playerPos.x);
    float distanceY = std::abs(skeletonPos.y - playerPos.y);
    
    //Player center to check if he passes left or right bound
    float playerCenterX = this->player_->playerRectangle_->getGlobalBounds().getCenter().x;
    // Атака при близости игрока
    if (distanceX < distanceToMakeAttack && distanceY < distanceToMakeAttack) {
        if (action_ != ATTACK1 && action_ != ATTACK2) {
            action_ = (rand() % 2 == 0) ? ATTACK1 : ATTACK2;
        }
    } else if (!isPlayerOutOfReach) {
        chasePlayer(skeletonPos, playerPos);
    }
    
    // Проверка досягаемости игрока по вертикали
    float skeletonTopY = skeletonRect->getGlobalBounds().getCenter().y - (skeletonRect->getSize().y / 2);
    float skeletonBotY = skeletonRect->getGlobalBounds().getCenter().y + (skeletonRect->getSize().y / 2);
    float playerBottomY = player_->playerRectangle_->getGlobalBounds().getCenter().y + (player_->playerRectangle_->getSize().y / 2);
    float playerTopY = player_->playerRectangle_->getGlobalBounds().getCenter().y - (player_->playerRectangle_->getSize().y / 2);
    
    bool isPlayerUnreachable = (playerBottomY < skeletonTopY || playerTopY > skeletonBotY) 
                            && !(playerCenterX > leftBound && playerCenterX < rightBound);

    // Проверка, находится ли игрок в зоне преследования (внутри границ патрулирования)
    bool isPlayerInPatrolZone = (playerCenterX > leftBound && playerCenterX < rightBound);

    // Проверка вертикальной досягаемости игрока
    bool isPlayerVerticallyReachable = !(playerBottomY < skeletonTopY || playerTopY > skeletonBotY);

    // Игрок достижим только если он и внутри зоны патрулирования, и на одном уровне
    bool isPlayerReachable = isPlayerInPatrolZone && isPlayerVerticallyReachable;

    if (!isPlayerReachable) {
        // Игрок недосягаем - запускаем/продолжаем таймер
        if (!isPlayerOutOfReachClock.isRunning()) {
            isPlayerOutOfReachClock.restart();
        }
        
        if (isPlayerOutOfReachClock.getElapsedTime().asMilliseconds() >= PATROL_SWITCH_DELAY) {
            isPlayerOutOfReach = true;
        }
    } else {
        // Игрок достижим - сбрасываем таймер и выходим из режима патрулирования
        isPlayerOutOfReachClock.restart();
        isPlayerOutOfReach = false;
    }

    // Переключение в патрулирование только если игрок недосягаем
    if (isPlayerOutOfReach) {
        makeRandomPatrolVariables();
        patrol();
    }
}

// ========== УПРАВЛЕНИЕ ==========
void Skeleton::updateControl() {
    switch (action_) {
        case WALKLEFT:
            walkLeft();
            break;
        case WALKRIGHT:
            walkRight();
            break;
        // TODO: Добавить прыжки и другие действия
    }
}

// ========== ФИЗИКА ==========
void Skeleton::updatePhysics() {
    applyFriction(initialWalkSpeed, frictionForce);
    
    skeletonRect->move({0.f, fallingSpeed});
    skeletonRect->move({initialWalkSpeed, 0.f});
    
    // Коллизии с границами окна
    if (skeletonRect->getPosition().y + skeletonRect->getSize().y >= WINDOW_HEIGHT) {
        isFalling = false;
        skeletonRect->setPosition({skeletonRect->getPosition().x, WINDOW_HEIGHT - skeletonRect->getSize().y});
    } else {
        isFalling = true;
    }
    
    if (isFalling) {
        fallingSpeed += 0.1f;
    } else {
        fallingSpeed = 0.f;
    }
    
    // Боковые границы
    if (skeletonRect->getPosition().x + skeletonRect->getSize().x >= WINDOW_WIDTH) {
        skeletonRect->setPosition({WINDOW_WIDTH - skeletonRect->getSize().x, skeletonRect->getPosition().y});
    } else if (skeletonRect->getPosition().x <= 0) {
        skeletonRect->setPosition({0.f, skeletonRect->getPosition().y});
    }
    
    if (HP_ <= 0) {
        isPlayingDieAnimation = true;
    }

    // Коллизии (всегда в конце)
    checkGroundCollision(*ground_);
    checkPlatformCollision(*platform_);
    if (HP_ > 0) checkBulletCollision(*player_);
    
    healthbar->update(HP_);
}

// ========== АНИМАЦИИ ==========
void Skeleton::updateTextures() {
    if (!isAlive) return;

    // Анимация получения урона
    if (isPlayingHurtAnimation && HP_ > 0) {
        pulseSprite(*skeletonSprite, sf::Color(255, 0, 0, 255), 1.f, sf::seconds(0.2f));
        if (!switchToNextSprite(skeletonSprite, *skeleton_hurtTextures, 
            skeleton_hurt_helper, switchSprite_SwitchOption::Single)) {
            isPlayingHurtAnimation = false;
            skeletonSprite->setColor({255, 255, 255, 255});
            action_ = IDLE;
        }
        
        sf::Vector2f rectCenter = skeletonRect->getGlobalBounds().getCenter();
        skeletonSprite->setPosition({rectCenter.x, rectCenter.y - 15.f});
        return;
    }
    
    // Анимация смерти
    if (isPlayingDieAnimation) {   
        skeletonSprite->setColor(sf::Color::Red);
        if (!switchToNextSprite(skeletonSprite, *skeleton_dieTextures, 
            skeleton_die_helper, switchSprite_SwitchOption::Single)) {
            isPlayingDieAnimation = false;
            isAlive = false;
        }
        
        sf::Vector2f rectCenter = skeletonRect->getGlobalBounds().getCenter();
        skeletonSprite->setPosition({rectCenter.x, rectCenter.y - 15.f});
        return;
    }

    // Анимации атаки
    if (action_ == ATTACK1 || action_ == ATTACK2) {
        bool attackFinished = false;

        // Направление взгляда в сторону игрока
        if (player_->playerRectangle_->getPosition().x > skeletonRect->getPosition().x) {
            skeletonSprite->setScale({enemyScale_.x, enemyScale_.y});
        } else {
            skeletonSprite->setScale({-enemyScale_.x, enemyScale_.y});
        } 

        if (action_ == ATTACK1) {
            attackFinished = !switchToNextSprite(skeletonSprite, *skeleton_attack1Textures,
                            skeleton_attack1_helper, switchSprite_SwitchOption::Single);
            
            // Нанесение урона в середине анимации
            if (skeleton_attack1_helper.ptrToTexture == skeleton_attack1_helper.countOfTextures / 2) {
                tryAttackPlayer();
            }
        } else { 
            attackFinished = !switchToNextSprite(skeletonSprite, *skeleton_attack2Textures,
                            skeleton_attack2_helper, switchSprite_SwitchOption::Single);
            
            // Нанесение урона в середине анимации
            if (skeleton_attack2_helper.ptrToTexture == skeleton_attack2_helper.countOfTextures / 2) {
                tryAttackPlayer();
            }
        }
        
        // Возврат в состояние покоя
        if (attackFinished) {
            action_ = IDLE;
        }
        
        sf::Vector2f rectCenter = skeletonRect->getGlobalBounds().getCenter();
        skeletonSprite->setPosition({rectCenter.x, rectCenter.y - 15.f});
        return;
    }

    // Обычные анимации
    switch (action_) {
        case IDLE:
            switchToNextSprite(skeletonSprite, *skeleton_idleTextures, 
                            skeleton_idle_helper, switchSprite_SwitchOption::Loop);
            break;

        case WALKRIGHT:
            skeletonSprite->setScale({enemyScale_.x, enemyScale_.y});
            switchToNextSprite(skeletonSprite, *skeleton_walkTextures,     
                            skeleton_walk_helper, switchSprite_SwitchOption::Loop);   
            break;

        case WALKLEFT:
            skeletonSprite->setScale({-enemyScale_.x, enemyScale_.y});
            switchToNextSprite(skeletonSprite, *skeleton_walkTextures,
                            skeleton_walk_helper, switchSprite_SwitchOption::Loop);
            break;
    }

    // Позиционирование спрайта
    sf::Vector2f rectCenter = skeletonRect->getGlobalBounds().getCenter();
    skeletonSprite->setPosition({rectCenter.x, rectCenter.y - 15.f});
}

// ========== ОТРИСОВКА ==========
void Skeleton::draw() {
    // window->draw(*skeletonRect); // Для отладки хитбокса
    window->draw(*skeletonSprite);
    healthbar->draw(!(this->isPlayingDieAnimation));
}

// ========== ГЕТТЕРЫ ==========
sf::RectangleShape &Skeleton::getRect() {
    return *skeletonRect;
}

int Skeleton::getHP() {
    return HP_;
}