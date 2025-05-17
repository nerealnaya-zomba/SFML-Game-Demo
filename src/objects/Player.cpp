#include<Player.h>


Player::Player()
{
    //Textures initialization
    initTextures(idleTextures,idleTexturesPaths);
    initTextures(runningTextures,runningTexturesPaths);
    initTextures(fallingTextures,fallingTexturesPaths);

    //Rectangles initialization
    playerRectangle = new sf::RectangleShape();
    playerRectangle->setSize({37.f,53.f});
    playerRectangle->setFillColor(sf::Color::Red);
    playerRectangle->setPosition({WINDOW_WIDTH/2,WINDOW_HEIGHT/2});
    //Sprite initialization
    playerSprite = new sf::Sprite(idleTextures.at(0));
    setSpriteOriginToMiddle(*playerSprite);
    //Trail initialization
    trail = new Trail(*playerSprite);
}

void Player::switchToNextFallingTexture()
{
    static int fps = 1;
    if(fps!=WINDOW_FPS/12)
    {   
        fps++;
        return;
    }
    static size_t i = 0;
    playerSprite->setTexture(fallingTextures[i],true);
    i++;
    if(i == fallingTextures.size()-1)
    {
        i=0;
    }
    
    fps=1;   
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
        switchToNextFallingTexture();
    }
    if(isIdle && !isFalling)
    {
        switchToNextIdleSprite();
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
        sf::FloatRect playerBounds = playerRectangle->getGlobalBounds();
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
                    playerRectangle->setPosition({platformBounds.position.x - playerBounds.size.x, playerBounds.position.y});
                } else {
                    // Справа
                    playerRectangle->setPosition({platformBounds.position.x + platformBounds.size.x, playerBounds.position.y});
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
                            playerRectangle->setPosition({playerBounds.position.x,playerBounds.position.y-2.f});
                        }
                    }
                        

                } else {
                    // Снизу
                    playerRectangle->setPosition({playerBounds.position.x, platformBounds.position.y + platformBounds.size.y});
                    fallingSpeed = -(fallingSpeed);
                }
            }
        }
    }
    
}

void Player::checkGroundCollision(sf::RectangleShape& groundRect)
{
    float playerX = playerRectangle->getPosition().x;
    float playerY = playerRectangle->getPosition().y+playerRectangle->getSize().y;
    float groundY = groundRect.getPosition().y;

    if(playerY>=groundY)
    {
        isFalling = false;
        fallingSpeed = 0.f;
        playerRectangle->setPosition({playerX,groundY-playerRectangle->getSize().y});
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
    playerRectangle->setPosition({playerRectangle->getPosition().x,playerRectangle->getPosition().y-1.f});
    fallingSpeed = -5.5f;
}

void Player::fallDown()
{
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
    
    
    playerRectangle->move({0.f,this->fallingSpeed});
    
    playerRectangle->move({initialWalkSpeed,0.f});
    
    if(playerRectangle->getPosition().y+playerRectangle->getSize().y>=WINDOW_HEIGHT)
    {
        isFalling = false;
        playerRectangle->setPosition({playerRectangle->getPosition().x, WINDOW_HEIGHT-playerRectangle->getSize().y});
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
    if(playerRectangle->getPosition().x+playerRectangle->getSize().x>=WINDOW_WIDTH)
    {
        playerRectangle->setPosition({WINDOW_WIDTH-playerRectangle->getSize().x,playerRectangle->getPosition().y});
    }
    else if(playerRectangle->getPosition().x<=0)
    {
        playerRectangle->setPosition({0.f,playerRectangle->getPosition().y});
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
    playerSprite->setTexture(idleTextures[i],true);
    if(i == idleTextures.size()-1 && goForward == true)
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
    playerSprite->setTexture(runningTextures[i],true);
    i++;
    if(i == runningTextures.size()-1)
    {
        i=0;
    }
    
    fps=1;
}

void Player::drawPlayer(sf::RenderWindow& window)
{
    drawPlayerTrailOnFall(window);

    playerSprite->setPosition({(playerRectangle->getPosition().x+playerRectangle->getSize().x/2),(playerRectangle->getPosition().y+playerRectangle->getSize().y/2)-6.f});
    //window.draw(*playerRectangle);
    window.draw(*playerSprite);
}

void Player::drawPlayerTrailOnFall(sf::RenderWindow& window)
{
    trail->trailColor = sf::Color(0,255,255,80);
    trail->speedOfTrailDisappearing = 8;
    
    trail->generateTrail(window);
    trail->makeTrailDisappear();
    trail->drawTrail(window);
}
