#include<Player.h>


Player::Player(GameData& gameTextures)
{
    //Textures initialization
    attachTexture(gameTextures.idleTextures,this->idleTextures);
    attachTexture(gameTextures.runningTextures,this->runningTextures);
    attachTexture(gameTextures.fallingTextures,this->fallingTextures);
    attachTexture(gameTextures.bulletTextures,this->bulletTextures);

    //Rectangles initialization
    playerRectangle_ = new sf::RectangleShape();
    playerRectangle_->setSize({37.f,53.f});
    playerRectangle_->setFillColor(sf::Color::Red);
    playerRectangle_->setPosition({WINDOW_WIDTH/2,WINDOW_HEIGHT/2});
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

void Player::updateTextures()
{
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
    {
        if(!isFalling)
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
        if(!isFalling)
        {
            switchToNextRunningSprite();
        }
        if(playerSprite->getScale().x<0)
        {
            playerSprite->setScale({-(playerSprite->getScale().x),1.f});
        }
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

void Player::checkRectCollision(std::vector<sf::RectangleShape*> rects)
{
    for (sf::RectangleShape* i : rects)
    {
                // Получаем глобальные границы (для удобства)
        sf::FloatRect playerBounds = playerRectangle_->getGlobalBounds();
        sf::FloatRect platformBounds = i->getGlobalBounds();

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

void Player::walkLeft()
{
    if(initialWalkSpeed<=(-maxWalkSpeed))
    {
        return; 
    }
    initialWalkSpeed-=speed;
}

void Player::walkRight()
{
    if(initialWalkSpeed>=maxWalkSpeed)
    {
        return;
    }
    initialWalkSpeed+=speed;
}

void Player::jump()
{
    std::cout << "Jump" << std::endl;
    playerRectangle_->setPosition({playerRectangle_->getPosition().x,playerRectangle_->getPosition().y-1.f});
    fallingSpeed = -5.5f;
}

void Player::fallDown()
{
}

void Player::shoot(bool direction)
{
    std::shared_ptr<Bullet> bulletPtr = std::make_shared<Bullet>(sf::Vector2f(playerRectangle_->getPosition().x+20.f,playerRectangle_->getPosition().y+20.f),this->bulletMaxDistance_);
    if(direction)
    {
        //If-else removes the possibility of spawning bullets slower than the standard bullet speed.
        if(bulletSpeed+initialWalkSpeed<bulletSpeed)
        {
            bulletPtr->offsetToMove_=sf::Vector2f(bulletSpeed,0.f);
        }
        else bulletPtr->offsetToMove_=sf::Vector2f(bulletSpeed+initialWalkSpeed,0.f);
    }
    else
    {
        //If-else removes the possibility of spawning bullets slower than the standard bullet speed.
        if(-bulletSpeed+initialWalkSpeed>-bulletSpeed)
        {
            bulletPtr->offsetToMove_=sf::Vector2f(-bulletSpeed,0.f);
        }
        else bulletPtr->offsetToMove_=sf::Vector2f(-bulletSpeed+initialWalkSpeed,0.f);
        bulletPtr->setSpriteScale({-1.f,1.f});
    }

    bullets.push_back(std::move(bulletPtr));
}

void Player::updateControls()
{
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
    
    
    playerRectangle_->move({0.f,this->fallingSpeed});
    
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
        fallingSpeed+=0.1f;
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

    playerSprite->setPosition({(playerRectangle_->getPosition().x+playerRectangle_->getSize().x/2),(playerRectangle_->getPosition().y+playerRectangle_->getSize().y/2)-6.f});
    //window.draw(*playerRectangle);
    window.draw(*playerSprite);
}

void Player::drawPlayerTrail(sf::RenderWindow& window)
{
    trail->trailColor = sf::Color(0,255,255,80);
    trail->trailColor = sf::Color(0,0,0,70);
    trail->speedOfTrailDisappearing = 5;
    
    trail->generateTrail(window);
    trail->makeTrailDisappear();
    trail->drawTrail(window);
}
