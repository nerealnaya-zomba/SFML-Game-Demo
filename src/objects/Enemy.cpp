#include<Enemy.h>

Enemy::Enemy()
{
    //Error texture initialization
    if(errorTexture_ = new sf::Texture("images/error.png")){std::cout << "Error texture loaded\n";}
    else{std::cout << "Failed loading error texture\n"; return;}
    //Rectangles initialization
    enemyRect_ = new sf::RectangleShape();
    enemyRect_->setSize({37.f,53.f});
    enemyRect_->setFillColor(sf::Color::Red);
    enemyRect_->setPosition({WINDOW_WIDTH/2,WINDOW_HEIGHT/2});
    //Sprite initialization
    enemySprite_ = new sf::Sprite(*errorTexture_);
    setSpriteOriginToMiddle(*enemySprite_);


    //There shoud be initTextures
    initTextures(idleTexturesPaths);
    
}

Enemy::~Enemy()
{
    delete enemySprite_, enemyRect_;
}

void Enemy::updateAI(sf::RectangleShape &enemyTarget)
{
    if(enemyRect_->getGlobalBounds().getCenter().x>=enemyTarget.getGlobalBounds().getCenter().x)
    {
        walkLeft();
    }
    else if(enemyRect_->getGlobalBounds().getCenter().x<=enemyTarget.getGlobalBounds().getCenter().x)
    {
        walkRight();
    }
}

void Enemy::walkLeft()
{
    if(initialWalkSpeed_<=(-maxWalkSpeed_))
    {
       return; 
    }
    initialWalkSpeed_-=speed_;
}
void Enemy::walkRight()
{
    if(initialWalkSpeed_>=maxWalkSpeed_)
    {
        return;
    }
    initialWalkSpeed_+=speed_;
}
void Enemy::jump()
{
    std::cout << "Jump" << std::endl;
    enemyRect_->setPosition({enemyRect_->getPosition().x,enemyRect_->getPosition().y-1.f});
    fallingSpeed_ = -5.5f;
}
void Enemy::fallDown()
{
    fallingSpeed_+=0.1f;
}


void Enemy::initTextures(std::vector<std::string> paths)
{
    
    std::vector<sf::Texture> loc_vec;

    for (size_t i = 0; i < paths.size(); i++)
    {
        
        sf::Texture texture;
        if(!texture.loadFromFile(paths[i]))
        {
            std::cout << "Error loading texture: " << paths[i] << std::endl;
        }
        else
        {
            std::cout << "Texture loaded: " << paths[i] << std::endl;
            loc_vec.push_back(std::move(texture));
        }
        
        
    }
    textures_.emplace(paths[0],std::move(loc_vec));

}
void Enemy::updateTextures()
{
    enemySprite_->setPosition({(enemyRect_->getPosition().x+enemyRect_->getSize().x/2),(enemyRect_->getPosition().y+enemyRect_->getSize().y/2)-6.f});

    switchToNextSprite("images/satiro-idle-1.png");

    setSpriteOriginToMiddle(*enemySprite_);
}



void Enemy::switchToNextSprite(std::string texture_first_name)
{
    static int fps{1};
    if(fps!=WINDOW_FPS/12)
    {   
        fps++;
        return;
    }
    static size_t i = 0;
    static bool goForward = true;
    std::vector<sf::Texture>& vector_textureREF = textures_.at(texture_first_name);
    enemySprite_->setTexture(vector_textureREF[i],true);
    if(i == vector_textureREF.size()-1 && goForward == true)
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

void Enemy::draw(sf::RenderWindow& window)
{
    window.draw(*enemyRect_);
    window.draw(*enemySprite_);
}

void Enemy::updatePhysics()
{
        applyFriction(initialWalkSpeed_,frictionForce_);
    
    
    enemyRect_->move({0.f,fallingSpeed_});
    
    enemyRect_->move({initialWalkSpeed_,0.f});
    
    if(enemyRect_->getPosition().y+enemyRect_->getSize().y>=WINDOW_HEIGHT)
    {
        isFalling_ = false;
        enemyRect_->setPosition({enemyRect_->getPosition().x, WINDOW_HEIGHT-enemyRect_->getSize().y});
    }
    else
    {
        isFalling_ = true;
    }
    if(isFalling_)
    {
        fallingSpeed_+=0.1f;
    }
    else
    {
        fallingSpeed_ = 0.f;
    }
    //std::cout << entityRect->getPosition().x << std::endl;

    //Window border collision (left and right)
    if(enemyRect_->getPosition().x+enemyRect_->getSize().x>=WINDOW_WIDTH)
    {
        enemyRect_->setPosition({WINDOW_WIDTH-enemyRect_->getSize().x,enemyRect_->getPosition().y});
    }
    else if(enemyRect_->getPosition().x<=0)
    {
        enemyRect_->setPosition({0.f,enemyRect_->getPosition().y});
    }
}
void Enemy::checkRectCollision(std::vector<sf::RectangleShape*> rects)
{
        for (sf::RectangleShape* i : rects)
    {
                // Получаем глобальные границы (для удобства)
        sf::FloatRect playerBounds = enemyRect_->getGlobalBounds();
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
                    enemyRect_->setPosition({platformBounds.position.x - playerBounds.size.x, playerBounds.position.y});
                } else {
                    // Справа
                    enemyRect_->setPosition({platformBounds.position.x + platformBounds.size.x, playerBounds.position.y});
                }
            } 
            // Коллизия с ВЕРХНЕЙ/НИЖНЕЙ сторонами
            else 
            {
                if (fromTop) {
                    // Сверху
                    isFalling_ = false;
                    if(fallingSpeed_>0.f)
                    {
                        fallingSpeed_ = 0.f;
                    }
                    if(fallingSpeed_ >= -0.1f && fallingSpeed_<=0.1f)
                    {
                        if(playerBounds.position.y+playerBounds.size.y >= platformBounds.position.y+3.f)
                        {
                            enemyRect_->setPosition({playerBounds.position.x,playerBounds.position.y-2.f});
                        }
                    }
                        

                } else {
                    // Снизу
                    enemyRect_->setPosition({playerBounds.position.x, platformBounds.position.y + platformBounds.size.y});
                    fallingSpeed_ = -(fallingSpeed_);
                }
            }
        }
    }
    
}
void Enemy::checkGroundCollision(sf::RectangleShape& groundRect)
{
    float playerX = enemyRect_->getPosition().x;
    float playerY = enemyRect_->getPosition().y+enemyRect_->getSize().y;
    float groundY = groundRect.getPosition().y;

    if(playerY>=groundY)
    {
        isFalling_ = false;
        fallingSpeed_ = 0.f;
        enemyRect_->setPosition({playerX,groundY-enemyRect_->getSize().y});
    }
}

void Enemy::applyFriction(float& walkSpeed, float friction)
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