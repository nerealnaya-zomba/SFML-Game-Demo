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
    auto ptrRect = std::make_shared<sf::RectangleShape>();
    ptrRect->setPosition(position);
    ptrRect->setFillColor(sf::Color::Green);
    ptrRect->setSize({355,55});

    rects.push_back(std::move(ptrRect));

    auto ptrSprite = std::make_unique<sf::Sprite>(textures.at(3));
    ptrSprite->setPosition({position.x-25,position.y-15});
    ptrSprite->setScale({0.2f,0.2f});
    ptrSprite->setColor(sf::Color(255,255,255,255));

    sprites.push_back(std::move(ptrSprite));
}

std::vector<sf::RectangleShape*> Platform::getRects()
{
    std::vector<sf::RectangleShape*> rects;
    for (std::shared_ptr<sf::RectangleShape> &i : this->rects)
    {
        rects.push_back(i.get());
    }
    
    return rects;
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
