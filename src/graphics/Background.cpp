#include<Background.h>

Background::Background()
{
    //Texture init
    this->skyTexture = new sf::Texture();
    skyTexture->loadFromFile("images/Background/Background_0.png") ? std::cout << "Texture loaded: images/Background/Background_0.png" << std::endl : std::cout << "Error loading texture: images/Background/Background_0.png" << std::endl;;
    this->mansionTexture = new sf::Texture();
    mansionTexture->loadFromFile("images/Background/Background_1.png") ? std::cout << "Texture loaded: images/Background/Background_1.png" << std::endl : std::cout << "Error loading texture: images/Background/Background_1.png" << std::endl;

    //Sprite init
    sky = new sf::Sprite(*skyTexture);
    sky->setScale({1920.f / 768.f, 1080.f / 416.f});
    mansion = new sf::Sprite(*mansionTexture);
    mansion->setScale({WINDOW_WIDTH / 768.f, WINDOW_HEIGHT / 416.f});
}

Background::~Background()
{
}

void Background::drawBackground(sf::RenderWindow &window)
{
    window.draw(*sky);
    window.draw(*mansion);
}
