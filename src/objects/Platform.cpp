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
    if(name == "Single-angled")
    {
        auto ptrRect = std::make_shared<sf::RectangleShape>();
        ptrRect->setPosition(position);
        ptrRect->setFillColor({0,0,0,0});
        ptrRect->setSize({40,40});
        sf::Vector2f center = ptrRect->getGlobalBounds().getCenter();
        rects.push_back(std::move(ptrRect));
        
        auto ptrSprite = std::make_unique<sf::Sprite>(textures.at(4));
        ptrSprite->setOrigin(ptrSprite->getGlobalBounds().getCenter());
        center.x-=6;
        ptrSprite->setPosition(center);
        ptrSprite->setScale({0.2f,0.2f});
        ptrSprite->setColor(sf::Color(255,255,255,255));
        
        sprites.push_back(std::move(ptrSprite));
    }
    else if(name == "Single-flat")
    {
        auto ptrRect = std::make_shared<sf::RectangleShape>();
        ptrRect->setPosition(position);
        ptrRect->setFillColor({0,0,0,0});
        ptrRect->setSize({40,30});
        sf::Vector2f center = ptrRect->getGlobalBounds().getCenter();
        rects.push_back(std::move(ptrRect));

        auto ptrSprite = std::make_unique<sf::Sprite>(textures.at(5));
        ptrSprite->setOrigin(ptrSprite->getGlobalBounds().getCenter());
        center.x-=6;
        ptrSprite->setPosition(center);
        ptrSprite->setScale({0.2f,0.2f});
        ptrSprite->setColor(sf::Color(255,255,255,255));
        
        sprites.push_back(std::move(ptrSprite));
    }
    else if(name == "Single-square")
    {
        auto ptrRect = std::make_shared<sf::RectangleShape>();
        ptrRect->setPosition(position);
        ptrRect->setFillColor({0,0,0,0});
        ptrRect->setSize({50,40});
        sf::Vector2f center = ptrRect->getGlobalBounds().getCenter();
        rects.push_back(std::move(ptrRect));

        auto ptrSprite = std::make_unique<sf::Sprite>(textures.at(6));
        ptrSprite->setOrigin(ptrSprite->getGlobalBounds().getCenter());
        center.x-=5;
        ptrSprite->setPosition(center);
        ptrSprite->setScale({0.2f,0.2f});
        ptrSprite->setColor(sf::Color(255,255,255,255));
        
        sprites.push_back(std::move(ptrSprite));
    }
    else if(name == "Double-horizontal-1")
    {
        auto ptrRect = std::make_shared<sf::RectangleShape>();
        ptrRect->setPosition(position);
        ptrRect->setFillColor({0,0,0,0});
        ptrRect->setSize({175,50});
        sf::Vector2f center = ptrRect->getGlobalBounds().getCenter();
        rects.push_back(std::move(ptrRect));

        auto ptrSprite = std::make_unique<sf::Sprite>(textures.at(0));
        ptrSprite->setOrigin(ptrSprite->getGlobalBounds().getCenter());
        ptrSprite->setPosition(center);
        ptrSprite->setScale({0.2f,0.2f});
        ptrSprite->setColor(sf::Color(255,255,255,255));
        
        sprites.push_back(std::move(ptrSprite));
    }
    else if(name == "Double-horizontal-2")
    {
        auto ptrRect = std::make_shared<sf::RectangleShape>();
        ptrRect->setPosition(position);
        ptrRect->setFillColor({0,0,0,0});
        ptrRect->setSize({175,55});
        sf::Vector2f center = ptrRect->getGlobalBounds().getCenter();
        rects.push_back(std::move(ptrRect));

        auto ptrSprite = std::make_unique<sf::Sprite>(textures.at(1));
        ptrSprite->setOrigin(ptrSprite->getGlobalBounds().getCenter());
        ptrSprite->setPosition(center);
        ptrSprite->setScale({0.2f,0.2f});
        ptrSprite->setColor(sf::Color(255,255,255,255));
        
        sprites.push_back(std::move(ptrSprite));
    }
    else if(name == "Double-vertical")
    {
        auto ptrRect = std::make_shared<sf::RectangleShape>();
        ptrRect->setPosition(position);
        ptrRect->setFillColor({0,0,0,0});
        ptrRect->setSize({55,140});
        sf::Vector2f center = ptrRect->getGlobalBounds().getCenter();
        rects.push_back(std::move(ptrRect));

        auto ptrSprite = std::make_unique<sf::Sprite>(textures.at(2));
        ptrSprite->setOrigin(ptrSprite->getGlobalBounds().getCenter());
        center.x-=3;
        ptrSprite->setPosition(center);
        ptrSprite->setScale({0.2f,0.2f});
        ptrSprite->setColor(sf::Color(255,255,255,255));
        
        sprites.push_back(std::move(ptrSprite));
    }
    else if(name == "Triple")
    {
        auto ptrRect = std::make_shared<sf::RectangleShape>();
        ptrRect->setPosition(position);
        ptrRect->setFillColor({0,0,0,0});
        ptrRect->setSize({270,50});
        sf::Vector2f center = ptrRect->getGlobalBounds().getCenter();
        rects.push_back(std::move(ptrRect));

        auto ptrSprite = std::make_unique<sf::Sprite>(textures.at(7));
        ptrSprite->setOrigin(ptrSprite->getGlobalBounds().getCenter());
        center.y-=10;
        center.x+=5;
        ptrSprite->setPosition(center);
        ptrSprite->setScale({0.2f,0.2f});
        ptrSprite->setColor(sf::Color(255,255,255,255));
        
        sprites.push_back(std::move(ptrSprite));
    }
    else if(name == "Quadruple")
    {
        auto ptrRect = std::make_shared<sf::RectangleShape>();
        ptrRect->setPosition(position);
        ptrRect->setFillColor({0,0,0,0});
        ptrRect->setSize({360,55});
        sf::Vector2f center = ptrRect->getGlobalBounds().getCenter();
        rects.push_back(std::move(ptrRect));

        auto ptrSprite = std::make_unique<sf::Sprite>(textures.at(3));
        ptrSprite->setOrigin(ptrSprite->getGlobalBounds().getCenter());
        ptrSprite->setPosition(center);
        ptrSprite->setScale({0.2f,0.2f});
        ptrSprite->setColor(sf::Color(255,255,255,255));
        
        sprites.push_back(std::move(ptrSprite));
    }
    
}

std::vector<std::shared_ptr<sf::RectangleShape>>& Platform::getRects()
{
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
