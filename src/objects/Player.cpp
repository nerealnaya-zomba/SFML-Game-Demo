#include<Player.h>


Player::Player()
{
    //Textures initialization
    initTextures(idleTextures,idleTexturesPaths);
    initTextures(runningTextures,runningTexturesPaths);

    //Rectangles initialization
    playerRectangle = new sf::RectangleShape();
    playerRectangle->setSize({96.f,96.f});
    setRectangleOriginToMiddle(*playerRectangle);
    playerRectangle->setPosition({300.f,400.f});
}

Player::~Player()
{

}

void Player::initTextures(std::vector<sf::Texture>& textures, std::vector<std::string> paths)
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
    playerRectangle->setTexture(&idleTextures[i],true);
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
    playerRectangle->setTexture(&runningTextures[i],true);
    i++;
    if(i == runningTextures.size()-1)
    {
        i=0;
    }
    
    fps=1;
}

void Player::drawPlayer(sf::RenderWindow& window)
{
    window.draw(*playerRectangle);
}
