#include<Player.h>


Player::Player(GameData& gameTextures)
{
    //Loading data from GameData.json
    loadData();

    //Textures initialization
    attachTexture(gameTextures.idleTextures,this->idleTextures);
    attachTexture(gameTextures.runningTextures,this->runningTextures);
    attachTexture(gameTextures.fallingTextures,this->fallingTextures);
    attachTexture(gameTextures.bulletTextures,this->bulletTextures);
    attachTexture(gameTextures.satiro_dieTextures,this->satiro_dieTextures,gameTextures.satiro_die_helper,this->satiro_die_helper);
    attachTexture(gameTextures.satiro_hurtTextures,this->satiro_hurtTextures,gameTextures.satiro_hurt_helper,this->satiro_hurt_helper);
    attachTexture(gameTextures.satiro_dashTextures,this->satiro_dashTextures,gameTextures.satiro_dash_helper,this->satiro_dash_helper);
    attachTexture(gameTextures.satiro_slideTextures,this->satiro_slideTextures,gameTextures.satiro_slide_helper,this->satiro_slide_helper);

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

void Player::updateTextures()
{
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

    if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
    {
        if(!isFalling && !isPlayingDashAnimation)
        {
            switchToNextRunningSprite();
        }
        if(playerSprite->getScale().x>0)
        {
            playerSprite->setScale({-(playerSprite->getScale().x),1.f});
        }
    }
    else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
    {
        if(!isFalling && !isPlayingDashAnimation)
        {
            switchToNextRunningSprite();
        }
        if(playerSprite->getScale().x<0)
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

    if(isFalling)
    {  
        switchToNextFallingSprite();
    }
    if(isIdle && !isFalling)
    {
        switchToNextIdleSprite();
    }
    switchToNextBulletSprite();
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

void Player::applyFriction(float& walkSpeed, float friction) 
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
    this->playerPosX_m = data["Player"]["PosX"];
    this->playerPosY_m = data["Player"]["PosY"];
    this->HP_ = data["Player"]["HP"];
    this->takeDMG_cooldown = data["Player"]["takeDMG_cooldown"];

    this->DMG_ = data["Bullet"]["DMG"];
    this->bulletMaxDistance_ = data["Bullet"]["bulletMaxDistance"];
    this->bulletSpeed = data["Bullet"]["bulletSpeed"];

    this->dashForce = data["Dash"]["force"];
    this->dashCooldown = data["Dash"]["Cooldown"];
}

void Player::checkRectCollision(std::vector<std::shared_ptr<sf::RectangleShape>>& rects)
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
    //Find and erase bullet that out of bounds
    bullets.remove_if([](std::shared_ptr<Bullet> &loc){
        return (loc->distancePassed>=loc->maxDistance_);});
    if(bullets.size()==0) bullets.clear();
    
    for (auto &&i : bullets)
    {
        i->moveBullet();
    }
    
}

void Player::updateParticles()
{
    for (auto& particle : bloodParticles) { 
        particle.update();
    }
    bloodParticles.erase(
        std::remove_if(bloodParticles.begin(), bloodParticles.end(),
            [](const Particle& p) { return !p.getIsAlive(); }),
        bloodParticles.end()
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

    std::shared_ptr<Bullet> bulletPtr = std::make_shared<Bullet>(sf::Vector2f(playerRectangle_->getPosition().x+20.f,playerRectangle_->getPosition().y+20.f),this->bulletMaxDistance_);
    if(direction)
    {
        //If-else removes the possibility of spawning bullets slower than the standard bullet speed.
        if(bulletSpeed+initialWalkSpeed<bulletSpeed)
        {
            bulletPtr->offsetToMove_=sf::Vector2f(bulletSpeed,0.f);
        }
        else bulletPtr->offsetToMove_=sf::Vector2f(bulletSpeed+(initialWalkSpeed*1.5),0.f);
    }
    else
    {
        //If-else removes the possibility of spawning bullets slower than the standard bullet speed.
        if(-bulletSpeed+initialWalkSpeed>-bulletSpeed)
        {
            bulletPtr->offsetToMove_=sf::Vector2f(-bulletSpeed,0.f);
        }
        else bulletPtr->offsetToMove_=sf::Vector2f(-bulletSpeed+(initialWalkSpeed*1.5),0.f);
        bulletPtr->setSpriteScale({-1.f,1.f});
    }

    bullets.push_back(std::move(bulletPtr));
}
void Player::dashParticles()
{
    sf::Vector2f playerPos = this->playerRectangle_->getGlobalBounds().getCenter();
        
    for (int i = 0; i < 40; i++) {
        sf::Color particleColor = sf::Color(255,255,255);
        float playerRectDownSide = playerPos.y+(playerRectangle_->getSize().y/2);
        float particleSpeed = random(-150,150);
        float particleSize = 1.f;
        float particleGravity = 0.f;
        float accelerationDamping = 0.8f;
        float particleLifeTime = 0.2f;

        if(playerSprite->getScale().x > 0){
            bloodParticles.emplace_back(
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
            bloodParticles.emplace_back(
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
}

void Player::bloodExplode()
{
    sf::Vector2f playerPos = this->playerRectangle_->getGlobalBounds().getCenter();

    for (int i = 0; i < 40; i++) {
        bloodParticles.emplace_back(
            sf::Vector2f(playerPos.x,playerPos.y),
            sf::Vector2f(random(-150,150), random(-450,-150)*1.5),
            sf::Vector2f(random(-60,60), random(-60,60)),
            sf::Color(255, 255, 255),
            2.0f,
            150.0f*3,
            0.8f,
            2.0f
        );
    }
}

void Player::updateControls()
{
    if(!this->isAlive || isPlayingDieAnimation) return;
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
}

void Player::updatePhysics()
{
    applyFriction(initialWalkSpeed,this->frictionForce);
    
    updateParticles();

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

    //Window border collision (left and right)
    if(playerRectangle_->getPosition().x+playerRectangle_->getSize().x>=WINDOW_WIDTH)
    {
        playerRectangle_->setPosition({WINDOW_WIDTH-playerRectangle_->getSize().x,playerRectangle_->getPosition().y});
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

void Player::switchToNextBulletSprite()
{
    static int fps = 1;
    if(fps!=WINDOW_FPS/12)
    {   
        fps++;
        return;
    }
    static size_t i = 0;

    for (auto &&loc : bullets)
    {
        loc->setSpriteTexture(bulletTextures->at(i));
    }
    
    i++;
    if(i == bulletTextures->size()-1)
    {
        i=0;
    }
    
    fps=1;
}

void Player::draw(sf::RenderWindow& window)
{
    drawPlayerTrail(window);

    drawParticles(window);
    playerSprite->setPosition({(playerRectangle_->getPosition().x+playerRectangle_->getSize().x/2),(playerRectangle_->getPosition().y+playerRectangle_->getSize().y/2)-6.f});
    //window.draw(*playerRectangle);
    window.draw(*playerSprite);
}

void Player::drawParticles(sf::RenderWindow &window)
{
    for (auto& particle : bloodParticles) { //REMOVELATER particle
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
        if(std::abs(initialWalkSpeed) < maxWalkSpeed*1.5) {
            trail->trailColor = sf::Color(0,0,0,80); // Обычная скорость
        } else {
            trail->trailColor = sf::Color(0,0,0,180); // Макс скорость
        }
        trail->generateTrail(window);
    }
    trail->makeTrailDisappear();
    trail->drawTrail(window);
}
