#include<Entity.h>

Enemy::Enemy()
{
    //Error texture initialization
    if(errorTexture_ = new sf::Texture("images/error.png")){std::cout << "Error texture loaded\n";}
    else{std::cout << "Failed loading error texture\n"; return;}
    //Rectangles initialization
    entityRect_ = new sf::RectangleShape();
    entityRect_->setSize({37.f,53.f});
    entityRect_->setFillColor(sf::Color::Red);
    entityRect_->setPosition({WINDOW_WIDTH/2,WINDOW_HEIGHT/2});
    //Sprite initialization
    entitySprite_ = new sf::Sprite(*errorTexture_);
    setSpriteOriginToMiddle(*entitySprite_);


    //There shoud be initTextures

    
    
}

Enemy::~Enemy()
{
    delete entitySprite_, entityRect_;
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
    entityRect_->setPosition({entityRect_->getPosition().x,entityRect_->getPosition().y-1.f});
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
    entitySprite_->setPosition({(entityRect_->getPosition().x+entityRect_->getSize().x/2),(entityRect_->getPosition().y+entityRect_->getSize().y/2)-6.f});

    switchToNextSprite("images/satiro-idle-1.png");

    setSpriteOriginToMiddle(*entitySprite_);
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
    entitySprite_->setTexture(vector_textureREF[i],true);
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
    window.draw(*entityRect_);
    window.draw(*entitySprite_);
}

void Enemy::updatePhysics()
{
        applyFriction(initialWalkSpeed_,frictionForce_);
    
    
    entityRect_->move({0.f,fallingSpeed_});
    
    entityRect_->move({initialWalkSpeed_,0.f});
    
    if(entityRect_->getPosition().y+entityRect_->getSize().y>=WINDOW_HEIGHT)
    {
        isFalling_ = false;
        entityRect_->setPosition({entityRect_->getPosition().x, WINDOW_HEIGHT-entityRect_->getSize().y});
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
    if(entityRect_->getPosition().x+entityRect_->getSize().x>=WINDOW_WIDTH)
    {
        entityRect_->setPosition({WINDOW_WIDTH-entityRect_->getSize().x,entityRect_->getPosition().y});
    }
    else if(entityRect_->getPosition().x<=0)
    {
        entityRect_->setPosition({0.f,entityRect_->getPosition().y});
    }
}
void Enemy::checkRectCollision(std::vector<sf::RectangleShape*> rects)
{
        for (sf::RectangleShape* i : rects)
    {
                // Получаем глобальные границы (для удобства)
        sf::FloatRect playerBounds = entityRect_->getGlobalBounds();
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
                    entityRect_->setPosition({platformBounds.position.x - playerBounds.size.x, playerBounds.position.y});
                } else {
                    // Справа
                    entityRect_->setPosition({platformBounds.position.x + platformBounds.size.x, playerBounds.position.y});
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
                            entityRect_->setPosition({playerBounds.position.x,playerBounds.position.y-2.f});
                        }
                    }
                        

                } else {
                    // Снизу
                    entityRect_->setPosition({playerBounds.position.x, platformBounds.position.y + platformBounds.size.y});
                    fallingSpeed_ = -(fallingSpeed_);
                }
            }
        }
    }
    
}
void Enemy::checkGroundCollision(sf::RectangleShape& groundRect)
{
    float playerX = entityRect_->getPosition().x;
    float playerY = entityRect_->getPosition().y+entityRect_->getSize().y;
    float groundY = groundRect.getPosition().y;

    if(playerY>=groundY)
    {
        isFalling_ = false;
        fallingSpeed_ = 0.f;
        entityRect_->setPosition({playerX,groundY-entityRect_->getSize().y});
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