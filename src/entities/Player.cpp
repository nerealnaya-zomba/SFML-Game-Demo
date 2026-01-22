#include<Player.h>

using namespace gameUtils;
Player::Player(GameData& gameTextures, GameLevelManager& m, GameCamera& c)
{
    this->gameTextures = &gameTextures;
    this->levelManager = &m;
    //Loading data from GameData.json
    loadData();

    //Textures initialization
    attachTexture(gameTextures.idleTextures,this->idleTextures);
    attachTexture(gameTextures.runningTextures,this->runningTextures);
    attachTexture(gameTextures.fallingTextures,this->fallingTextures);
    attachTexture(gameTextures.satiro_dieTextures,this->satiro_dieTextures,gameTextures.satiro_die_helper,this->satiro_die_helper);
    attachTexture(gameTextures.satiro_hurtTextures,this->satiro_hurtTextures,gameTextures.satiro_hurt_helper,this->satiro_hurt_helper);
    attachTexture(gameTextures.satiro_dashTextures,this->satiro_dashTextures,gameTextures.satiro_dash_helper,this->satiro_dash_helper);
    attachTexture(gameTextures.satiro_slideTextures,this->satiro_slideTextures,gameTextures.satiro_slide_helper,this->satiro_slide_helper);
    attachTexture(gameTextures.satiro_jumpTextures,this->satiro_jumpTextures,gameTextures.satiro_jump_helper,this->satiro_jump_helper);
    attachTexture(gameTextures.satiro_landingTextures,this->satiro_landingTextures,gameTextures.satiro_landing_helper,this->satiro_landing_helper);

    //Rectangles initialization
    playerRectangle_ = new sf::RectangleShape();
    playerRectangle_->setSize({37.f,53.f});
    playerRectangle_->setFillColor(sf::Color::Red);
    playerRectangle_->setPosition({playerPosX_m,playerPosY_m});
    //Sprite initialization
    playerSprite = new sf::Sprite(idleTextures->at(0));
    setSpriteOriginToMiddle(*playerSprite);
    //Trail initialization
    trail = new Trail(*playerSprite);
    //Portal initizalization
    portal = new LevelPortal({0.0,0.0},BASE_PORTAL_SPEED_OF_OPENING,BASE_PORTAL_SPEED_OF_CLOSING,portalExistTime,*playerSprite,*playerRectangle_,gameTextures,m);
    portalCallCloseCooldownClock.reset();
    portalCallOpenCooldownClock.reset();
}

void Player::switchToNextFallingSprite()
{
    static int fps = 1;
    if(fps!=WINDOW_FPS/12)
    {   
        fps++;
        return;
    }
    static size_t i = 0;
    playerSprite->setTexture(fallingTextures->at(i),true);
    i++;
    if(i == fallingTextures->size()-1)
    {
        i=0;
    }
    
    fps=1;   
}

sf::Vector2f Player::getSpriteScale()
{
    return playerSprite->getScale();
}

sf::Vector2f Player::getCenterPosition()
{
    return playerRectangle_->getGlobalBounds().getCenter();
}

sf::Clock& Player::getShootClock()
{
    return shootTimer;
}

int &Player::getShootCooldown()
{
    return ButtonRepeat_shootCooldown;
}

sf::Clock &Player::getDashClock()
{
    return dash_Clock;
}

int &Player::getDashCooldown()
{
    return dashCooldown;
}

sf::Clock &Player::getPortalClock()
{
    return portalCallOpenCooldownClock;
}

int &Player::getPortalCooldown()
{
    return portalCallCooldown;
}

void Player::attachGameLevelManager(GameLevelManager& m)
{
    this->levelManager = &m;
}

void Player::setPosition(sf::Vector2f pos)
{
    this->playerRectangle_->setPosition(pos);
    this->playerSprite->setPosition(playerRectangle_->getGlobalBounds().getCenter());
}

void Player::updateTextures()
{
    portal->update();   //NOTE Should be updating no matter what
    if(portal->getIsInAreaOfTeleportation()) fallingSpeed = 0.f;

    if(!isAlive)
    {
        sf::Texture &lastDieTexture = satiro_dieTextures->at(satiro_dieTextures->size()-1);
        this->playerSprite->setTexture(lastDieTexture);
        return; //Nothing happens if player not alive
    }

    if(isPlayingDieAnimation)
    {
        if(!switchToNextSprite(this->playerSprite,*this->satiro_dieTextures,satiro_die_helper,switchSprite_SwitchOption::Single))
        {
            isAlive = false;
            isPlayingDieAnimation = false;
        }
        return;
    }

    if(isPlayingHurtAnimation)
    {
        if(!switchToNextSprite(this->playerSprite,*this->satiro_hurtTextures,satiro_hurt_helper,switchSprite_SwitchOption::Single))
        {
            isPlayingHurtAnimation = false;
        }
        return;
    }

    if(isFliesUp)
    {
        switchToNextSprite(this->playerSprite,*this->satiro_jumpTextures,satiro_jump_helper,switchSprite_SwitchOption::Loop);
    } else {
        if(isJumped && !isFalling)
        {
            if(!switchToNextSprite(this->playerSprite,*this->satiro_landingTextures,satiro_landing_helper,switchSprite_SwitchOption::Single))
            {
                isJumped = false;
            }
        }
    }

    if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
    {
        if(!isFalling && !isPlayingDashAnimation && !isJumped && !isControlsBlocked)
        {
            switchToNextRunningSprite();
        }
        if(playerSprite->getScale().x>0 && !isControlsBlocked)
        {
            playerSprite->setScale({-(playerSprite->getScale().x),1.f});
        }
    }
    else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
    {
        if(!isFalling && !isPlayingDashAnimation && !isJumped && !isControlsBlocked)
        {
            switchToNextRunningSprite();
        }
        if(playerSprite->getScale().x<0 && !isControlsBlocked)
        {
            playerSprite->setScale({-(playerSprite->getScale().x),1.f});
        }
    }

    if(isPlayingDashAnimation)
    {
        if(!switchToNextSprite(this->playerSprite,*this->satiro_dashTextures,satiro_dash_helper,switchSprite_SwitchOption::Single))
        {
            isPlayingDashAnimation= false;
        }
        dashParticles();
        return;
    }

    if(isFalling && !isFliesUp)
    {  
        switchToNextFallingSprite();
    }
    if(isIdle && !isFalling && !isJumped && !isFliesUp)
    {
        switchToNextIdleSprite();
    }
    for (auto &&i : bullets)
    {
        i->updateTextures();
    }
    
}

void Player::drawBullets(sf::RenderWindow& window)
{
    for (auto &&i : bullets)
    {
        i->draw(window);
    }
    
}

Player::~Player()
{
    
    
}

void Player::portalUpdate()
{
    //Call cooldown
    if(isPortalOnCooldown && portal->getIsClosed() && !portal->getIsCalledForClose() && !portal->getIsCalledForOpen() && !portalCallOpenCooldownClock.isRunning())
    {
        portalCallOpenCooldownClock.restart();
    }
    if(isPortalOnCooldown && checkInterval(portalCallOpenCooldownClock,portalCallCooldown))
    {
        isPortalOnCooldown = false;
        portalCallOpenCooldownClock.reset();
    }

    if(portalCallOpenCooldownClock.isRunning())
    {
        std::cout << "Current cooldown: " << portalCallOpenCooldownClock.getElapsedTime().asMilliseconds() <<"\n";
    }

    //Closing portal
    if(portal->getIsOpened() && !portal->getIsCalledForClose() && !portalCallCloseCooldownClock.isRunning())
    {
        portalCallCloseCooldownClock.restart();
    }
    if(checkInterval(portalCallCloseCooldownClock,portalExistTime))
    {
        portal->closePortal();
        portalCallCloseCooldownClock.reset();
    }

    //Is player in area of teleportation
    if(portal->getIsOpened())
    {
        portal->checkIsTargetInAreaOfTeleportation();
    }
}

void Player::tryOpenPortal()
{
    //Open portal only when it closed and is not called for open
    if( portal->getIsClosed() && !portal->getIsCalledForOpen() && !isPortalOnCooldown)
    {
        if(sf::Keyboard::isKeyPressed(portalCallKey))
        {
            //Setting portal position based on what side player watches now
                //Right
            if(playerSprite->getScale().x > 0.f)
            {
                portal->setSide(PortalCalledSide::RIGHT);
                portal->setPosition({playerSprite->getGlobalBounds().getCenter().x+BASE_OFFSET_TO_CREATE_PORTAL,playerSprite->getGlobalBounds().getCenter().y});
            }
                //Left
            else if(playerSprite->getScale().x < 0.f)
            {
                portal->setSide(PortalCalledSide::LEFT);
                portal->setPosition({playerSprite->getGlobalBounds().getCenter().x-BASE_OFFSET_TO_CREATE_PORTAL,playerSprite->getGlobalBounds().getCenter().y});
            }

            portal->openPortal();
            isPortalOnCooldown = true;
        }
    }
}

void Player::applyFriction(float &walkSpeed, float friction)
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

void Player::saveData()
{
    std::ifstream f("GameData.json");
    nlohmann::json data = nlohmann::json::parse(f);


    
}

void Player::loadData()
{
    std::fstream f("PlayerConfig.json");
    nlohmann::json data = nlohmann::json::parse(f);
    //Player
    this->playerPosX_m = data["Player"]["PosX"];
    this->playerPosY_m = data["Player"]["PosY"];
    this->HP_ = data["Player"]["HP"];
    this->takeDMG_cooldown = data["Player"]["takeDMG_cooldown"];

    //Jump
    this->ButtonRepeat_jumpCooldown = data["Jump"]["repeatCooldown"];

    //Bullet
    this->DMG_ = data["Bullet"]["DMG"];
    this->bulletMaxDistance_ = data["Bullet"]["bulletMaxDistance"];
    this->bulletSpeed = data["Bullet"]["bulletSpeed"];
    this->bulletSpeedReduction = data["Bullet"]["bulletSpeedReduction"];
    this->ButtonRepeat_shootCooldown = data["Bullet"]["repeatCooldown"];
    

    //Dash
    this->dashForce = data["Dash"]["force"];
    this->dashCooldown = data["Dash"]["Cooldown"];
    this->ButtonRepeat_dashCooldown = data["Dash"]["repeatCooldown"];
}

void Player::checkPlatformRectCollision(std::vector<std::shared_ptr<sf::RectangleShape>>& rects)
{
    for (auto& rectPtr : rects)  // меняем тип итератора
    {
        // Получаем глобальные границы (для удобства)
        sf::FloatRect playerBounds = playerRectangle_->getGlobalBounds();
        sf::FloatRect platformBounds = rectPtr->getGlobalBounds();  // используем -> для shared_ptr

        // Проверяем пересечение
        if (playerBounds.findIntersection(platformBounds)) 
        {
            // Определяем направление коллизии
            float overlapLeft   = playerBounds.position.x + playerBounds.size.x - platformBounds.position.x;
            float overlapRight  = platformBounds.position.x + platformBounds.size.x - playerBounds.position.x;
            float overlapTop    = playerBounds.position.y + playerBounds.size.y - platformBounds.position.y;
            float overlapBottom = platformBounds.position.y + platformBounds.size.y - playerBounds.position.y;

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
                    playerRectangle_->setPosition({platformBounds.position.x - playerBounds.size.x, playerBounds.position.y});
                } else {
                    // Справа
                    playerRectangle_->setPosition({platformBounds.position.x + platformBounds.size.x, playerBounds.position.y});
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
                        if(playerBounds.position.y+playerBounds.size.y >= platformBounds.position.y+3.f)
                        {
                            playerRectangle_->setPosition({playerBounds.position.x,playerBounds.position.y-2.f});
                        }
                    }
                } else {
                    // Снизу
                    playerRectangle_->setPosition({playerBounds.position.x, platformBounds.position.y + platformBounds.size.y});
                    fallingSpeed = -(fallingSpeed);
                }
            }
        }
    }
}

void Player::checkGroundCollision(sf::RectangleShape& groundRect)
{
    float playerX = playerRectangle_->getPosition().x;
    float playerY = playerRectangle_->getPosition().y+playerRectangle_->getSize().y;
    float groundY = groundRect.getPosition().y;

    if(playerY>=groundY)
    {
        isFalling = false;
        fallingSpeed = 0.f;
        playerRectangle_->setPosition({playerX,groundY-playerRectangle_->getSize().y});
    }
}

void Player::moveBullets()
{
    // Удаляем пули которые можно удалить
    bullets.remove_if([](std::shared_ptr<Bullet>& bullet) {
        return bullet->canBeDeleted;
    });
    
    for (auto &&i : bullets)
    {
        i->update();
    }
}

void Player::updateParticles()
{
    for (auto& particle : particles) { 
        particle.update();
    }
    particles.erase(
        std::remove_if(particles.begin(), particles.end(),
            [](const Particle& p) { return !p.getIsAlive(); }),
        particles.end()
    );
}

void Player::walkLeft()
{
    if(!this->isAlive || isPlayingDieAnimation || isPlayingDashAnimation) return;       //Locking movement on Die, Dash

    if(initialWalkSpeed<=(-maxWalkSpeed))
    {
        return; 
    }
    initialWalkSpeed-=speed;
}

void Player::walkRight()
{
    if(!this->isAlive || isPlayingDieAnimation || isPlayingDashAnimation) return;       //Locking movement on Die, Dash

    if(initialWalkSpeed>=maxWalkSpeed)
    {
        return;
    }
    initialWalkSpeed+=speed;
}

void Player::jump()
{
    if(!this->isAlive || isPlayingDieAnimation || isPlayingDashAnimation) return;       //Locking movement on Die, Dash

    std::cout << "Jump" << std::endl;
    playerRectangle_->setPosition({playerRectangle_->getPosition().x,playerRectangle_->getPosition().y-1.f});
    fallingSpeed = -5.5f;
}

void Player::fallDown()
{
}

void Player::dash()
{
    if(!this->isAlive || isPlayingDieAnimation) return;

        if(isDashOnCooldown)
    {
        if(dash_Clock.getElapsedTime().asMilliseconds() >= dashCooldown)
        {
            isDashOnCooldown = false;
            dash_Clock.stop();
        }
    }
    if(!isDashOnCooldown)
    {
        //Cooldown
        isPlayingDashAnimation = true;
        isDashOnCooldown = true;
        dash_Clock.restart();
        if(playerSprite->getScale().x>0){
            initialWalkSpeed = dashForce;
        } else{
            initialWalkSpeed = -dashForce;
        }
        std::cout << "Dash: "<< std::endl; // REMOVELATER Player dashed debug
    }
}

void Player::shoot(bool direction)
{
    if(!this->isAlive || isPlayingDieAnimation) return;

    std::shared_ptr<Bullet> bulletPtr = std::make_shared<Bullet>(sf::Vector2f(playerRectangle_->getPosition().x+20.f,playerRectangle_->getPosition().y+20.f),this->bulletMaxDistance_,*this->gameTextures);
    if(direction)
    {
        //If-else removes the possibility of spawning bullets slower than the standard bullet speed.
        if(bulletSpeed+initialWalkSpeed<bulletSpeed)
        {
            bulletPtr->setSpeed(sf::Vector2f(bulletSpeed,0.f));
        }
        else bulletPtr->setSpeed(sf::Vector2f(bulletSpeed+(initialWalkSpeed*1.5),0.f));
    }
    else
    {
        //If-else removes the possibility of spawning bullets slower than the standard bullet speed.
        if(-bulletSpeed+initialWalkSpeed>-bulletSpeed)
        {
            bulletPtr->setSpeed(sf::Vector2f(-bulletSpeed,0.f));
        }
        else bulletPtr->setSpeed(sf::Vector2f(-bulletSpeed+(initialWalkSpeed*1.5),0.f));
        bulletPtr->setSpriteScale({-1.f,1.f});
    }

    bullets.push_back(std::move(bulletPtr));
}
void Player::dashParticles()
{
    sf::Vector2f playerPos = this->playerRectangle_->getGlobalBounds().getCenter();
        
    for (int i = 0; i < 40; i++) {
        sf::Color particleColor = sf::Color(150,150,150);
        float playerRectDownSide = playerPos.y+(playerRectangle_->getSize().y/2);
        float particleSpeed = random(-150,150);
        float particleSize = 1.f;
        float particleGravity = 0.f;
        float accelerationDamping = 0.8f;
        float particleLifeTime = 0.2f;

        if(playerSprite->getScale().x > 0){
            particles.emplace_back(
            sf::Vector2f(playerPos.x,playerRectDownSide),
            sf::Vector2f(particleSpeed, 0.f),
            sf::Vector2f(random(-60,60), random(-60,60)),
            sf::Color(particleColor),
            particleSize,
            particleGravity,
            accelerationDamping,
            particleLifeTime
            );
        } else{
            particles.emplace_back(
            sf::Vector2f(playerPos.x,playerRectDownSide),
            sf::Vector2f(particleSpeed, 0.f),
            sf::Vector2f(random(-60,60), random(-60,60)),
            sf::Color(particleColor),
            particleSize,
            particleGravity,
            accelerationDamping,
            particleLifeTime 
            );
        } 
    }
}
/*
    Returns true on successful hit \ Returns false on unsucessful hit

    Has cooldown that declared in Player.h
*/
bool Player::takeDMG(int count, sf::Vector2f knockback, bool side) 
{
    if(takeDMG_isOnCooldown)
    {
        if(takeDMG_timer.getElapsedTime().asMilliseconds() >= takeDMG_cooldown)
        {
            takeDMG_isOnCooldown = false;
            takeDMG_timer.stop();
        }
        return false;
    }
    if(!takeDMG_isOnCooldown)
    {
        //Knockback
        this->fallingSpeed-=knockback.y;
        side ? this->initialWalkSpeed-=knockback.x : this->initialWalkSpeed+=knockback.x;

        //HP reduction
        this->HP_ -= count;

        //Hurt animation enabling
        isPlayingHurtAnimation = true;

        //Blood
        bloodExplode();

        //Cooldown
        takeDMG_isOnCooldown = true;
        takeDMG_timer.restart();
        std::cout << "Player hitted. HP: " << this->HP_ << std::endl; // REMOVELATER Player hitted debug
        return true;
    }
    return false;
}

void Player::bloodExplode()
{
    sf::Vector2f playerPos = this->playerRectangle_->getGlobalBounds().getCenter();

    for (int i = 0; i < 40; i++) {
        particles.emplace_back(
            sf::Vector2f(playerPos.x,playerPos.y),
            sf::Vector2f(random(-150,150), random(-450,-150)*1.5),
            sf::Vector2f(random(-60,60), random(-60,60)),
            sf::Color(138, 3, 3),
            2.0f,
            150.0f*3,
            0.8f,
            3.0f
        );
    }
}

void Player::updateControls()
{
    if(!this->isAlive || isPlayingDieAnimation || isControlsBlocked) return;
    
    // Обновляем готовность действий по таймерам
    if (!canShoot && shootTimer.getElapsedTime().asMilliseconds() >= ButtonRepeat_shootCooldown) {
        canShoot = true;
        shootTimer.stop();
    }
    
    if (!canJump && jumpTimer.getElapsedTime().asMilliseconds() >= ButtonRepeat_jumpCooldown) {
        canJump = true;
        jumpTimer.stop();
    }
    
    if (!canDash && dashTimer.getElapsedTime().asMilliseconds() >= ButtonRepeat_dashCooldown) {
        canDash = true;
        dashTimer.stop();
    }
    
    // Shooting (X key)
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::X) && canShoot)
    {
        std::cout << "Shoot" << std::endl;
        shoot(getSpriteScale().x > 0);
        canShoot = false;
        shootTimer.restart();
    }
    
    // Jumping (Z key)
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Z) && canJump)
    {
        if(!isFalling)
        {
            jump();
            canJump = false;
            jumpTimer.restart();
        }
    }
    
    // Dash (C key)
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::C) && canDash)
    {
        std::cout << "Dash" << std::endl;
        dash();
        canDash = false;
        dashTimer.restart();
    }
    
    // Moving right-left
    isIdle = true;
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
    {
        isIdle = false;
        walkLeft();
    }
    else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
    {
        isIdle = false;
        walkRight();
    }

    //Open portal  
    tryOpenPortal();
}

void Player::updatePhysics()
{
    portalUpdate();

    applyFriction(initialWalkSpeed,this->frictionForce);
    
    updateParticles();

    if(fallingSpeed<0)
    {
            isFliesUp = true;
            isJumped  = true;
    } else  isFliesUp = false;

    if(this->HP_<=0) isPlayingDieAnimation = true;
    
    if(!isPlayingDashAnimation) playerRectangle_->move({0.f,this->fallingSpeed});
    
    playerRectangle_->move({initialWalkSpeed,0.f});
    
    if(playerRectangle_->getPosition().y+playerRectangle_->getSize().y>=WINDOW_HEIGHT)
    {
        isFalling = false;
        playerRectangle_->setPosition({playerRectangle_->getPosition().x, WINDOW_HEIGHT-playerRectangle_->getSize().y});
    }
    else
    {
        isFalling = true;
    }
    if(isFalling)
    {
        if(isPlayingDashAnimation)
        {
            fallingSpeed= 0.f;
        } else{
            fallingSpeed+=0.1f;
        }

    }
    else
    {
        fallingSpeed = 0.f;
    }
    //std::cout << playerRectangle->getPosition().x << std::endl;

    //Level border collision (left and right)
    float levelWidth = static_cast<float>(levelManager->getCurrentLevelSize().x);
    float levelHeight = static_cast<float>(levelManager->getCurrentLevelSize().y);
    if(playerRectangle_->getPosition().x+playerRectangle_->getSize().x>=levelWidth)
    {
        playerRectangle_->setPosition({levelWidth-playerRectangle_->getSize().x,playerRectangle_->getPosition().y});
    }
    else if(playerRectangle_->getPosition().x<=0)
    {
        playerRectangle_->setPosition({0.f,playerRectangle_->getPosition().y});
    }
}

void Player::initTextures(std::vector<sf::Texture> &textures, std::vector<std::string> paths)
{
    for (size_t i = 0; i < paths.size(); i++)
    {
        sf::Texture* texture = new sf::Texture();
        if(!texture->loadFromFile(paths[i]))
        {
            std::cout << "Error loading texture: " << paths[i] << std::endl;
        }
        else
        {
            std::cout << "Texture loaded: " << paths[i] << std::endl;
        }
        textures.push_back(*texture);
        
        
    }
}

void Player::switchToNextIdleSprite()
{
    static int fps{1};
    if(fps!=WINDOW_FPS/12)
    {   
        fps++;
        return;
    }
    static size_t i = 0;
    static bool goForward = true;
    playerSprite->setTexture(idleTextures->at(i),true);
    if(i == idleTextures->size()-1 && goForward == true)
    {
        goForward = false;
    }
    else if(i == 0 && goForward == false)
    {
        goForward = true;
    }
    if(goForward)
    {
        i++;
    }
    else
    {
        i--;
    }
    fps=1;
}

void Player::switchToNextRunningSprite()
{
    static int fps = 1;
    if(fps!=WINDOW_FPS/12)
    {   
        fps++;
        return;
    }
    static size_t i = 0;
    playerSprite->setTexture(runningTextures->at(i),true);
    i++;
    if(i == runningTextures->size()-1)
    {
        i=0;
    }
    
    fps=1;
}



void Player::draw(sf::RenderWindow& window)
{
    portal->draw(window);

    drawPlayerTrail(window);

    drawParticles(window);
    playerSprite->setPosition({(playerRectangle_->getPosition().x+playerRectangle_->getSize().x/2),(playerRectangle_->getPosition().y+playerRectangle_->getSize().y/2)-6.f});
    // window.draw(*playerRectangle_);
    window.draw(*playerSprite);
}

void Player::drawParticles(sf::RenderWindow &window)
{
    for (auto& particle : particles) {
        particle.draw(window);
    }
}

void Player::drawPlayerTrail(sf::RenderWindow& window)
{
    //trail->trailColor = sf::Color(0,255,255,80);
    trail->speedOfTrailDisappearing = 5;
    trail->trailColor = sf::Color(0,0,0,80);
    // Трейл только когда игрок ДВИГАЕТСЯ
    if(std::abs(initialWalkSpeed) > 0.1f) {
        if(std::abs(initialWalkSpeed) > maxWalkSpeed*1.5) {
            trail->trailColor = sf::Color(0,0,0,180); // Обычная скорость
            trail->generateTrail(window);
        }
    }
    trail->makeTrailDisappear();
    trail->drawTrail(window);
}

void Player::blockControls()
{
    this->isControlsBlocked = true;
}

void Player::unblockControls()
{
    this->isControlsBlocked = false;
}
