#include<Background.h>

Background::Background(sf::Vector2f pos, std::string bgName, Type t = Type::Single) : position(pos), name(bgName), type(t)
{
    //Texture init
    this->skyTexture = new sf::Texture();
    skyTexture->loadFromFile("images/Background/Background_0.png") ? std::cout << "Texture loaded: images/Background/Background_0.png" << std::endl : std::cout << "Error loading texture: images/Background/Background_0.png" << std::endl;;
    this->mansionTexture = new sf::Texture();
    mansionTexture->loadFromFile("images/Background/Background_1.png") ? std::cout << "Texture loaded: images/Background/Background_1.png" << std::endl : std::cout << "Error loading texture: images/Background/Background_1.png" << std::endl;

    //Sprite init
    sky = new sf::Sprite(*skyTexture);

    //Calibrating background to window width and height
    float window_width = static_cast<float>(WINDOW_WIDTH);
    float window_height = static_cast<float>(WINDOW_HEIGHT);
    sky->setScale({window_width / 768.f, window_height / 416.f});

    mansion = new sf::Sprite(*mansionTexture);
    mansion->setScale({WINDOW_WIDTH / 768.f, WINDOW_HEIGHT / 416.f});
}

Background::~Background()
{
}

void Background::draw(sf::RenderWindow &window)
{
    switch(type){
        case Type::Single:
        window.draw(*sky);
        window.draw(*mansion);
        break;

        case Type::Repeated:                    // IMPLEMENTME
        break;

        default:break;
    }
}
