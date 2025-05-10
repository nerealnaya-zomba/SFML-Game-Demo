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
    playerRectangle->setPosition({300.f,400.f});
    //Sprite initialization
    playerSprite = new sf::Sprite(idleTextures.at(0));
    setSpriteOriginToMiddle(*playerSprite);
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

void Player::walkLeft(float speed)
{
    if(walkSpeed<=(-maxWalkSpeed))
    {
       return; 
    }
    walkSpeed-=speed;
}

void Player::walkRight(float speed)
{
    if(walkSpeed>=maxWalkSpeed)
    {
        return;
    }
    walkSpeed+=speed;
}

void Player::updateControls()
{
    isIdle = true;
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
    {
        isIdle = false;

        walkLeft(0.25f);
    }
    else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
    {
        isIdle = false;

        
        walkRight(0.25f);
    }
}

void Player::updatePhysics()
{
    applyFriction(walkSpeed,0.1f);
    playerRectangle->move({0.f,fallingSpeed});
    playerRectangle->move({walkSpeed,0.f});
    if(walkSpeed>0.f)
    {
        walkSpeed-=0.1f;
    }
    else if(walkSpeed<0.f)
    {
        walkSpeed+=0.1f;
    }
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
    std::cout << playerRectangle->getPosition().x << std::endl;

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
        
        //Delete ptr on last interation
        if(i==paths.size()-1)
        {
            texture = nullptr;
            delete texture;
        }
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
    playerSprite->setPosition({(playerRectangle->getPosition().x+playerRectangle->getSize().x/2),(playerRectangle->getPosition().y+playerRectangle->getSize().y/2)-6.f});
    window.draw(*playerRectangle);
    window.draw(*playerSprite);
}
