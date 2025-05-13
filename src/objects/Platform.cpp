#include<Platform.h>

void Platform::draw(sf::RenderWindow &window)
{
    for (auto &i : rects)
    {
        window.draw(*i);
    }
    for (auto &i : sprites)
    {
        window.draw(*i);
    }
    
}

void Platform::addPlatform(sf::Vector2f position, std::string name)
{
    auto ptrRect = std::make_unique<sf::RectangleShape>();
    ptrRect->setPosition(position);
    ptrRect->setFillColor(sf::Color::Green);
    ptrRect->setSize({385,70});

    rects.push_back(std::move(ptrRect));

    auto ptrSprite = std::make_unique<sf::Sprite>(textures.at(3));
    ptrSprite->setPosition(position);
    ptrSprite->setScale({0.2f,0.2f});
    ptrSprite->setColor(sf::Color(255,255,255,150));

    sprites.push_back(std::move(ptrSprite));
}

Platform::Platform()
{
    initTextures(textures,paths);
}

Platform::~Platform()
{
}

void Platform::initTextures(std::vector<sf::Texture>& textures, std::vector<std::string> paths)
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

    for (sf::Texture& i : textures)
    {
        if(i.generateMipmap())
        {
            std::cout << "Mipmap generated\n";
        }
        i.setSmooth(true);
    }
    
}
