#include <Platform.h>

namespace
{
constexpr bool DRAW_PLATFORM_HITBOXES = false;

const std::vector<std::string> kPlatformTexturePaths{
    "images/platform/Double-horizontal-1.png",
    "images/platform/Double-horizontal-2.png",
    "images/platform/Double-vertical.png",
    "images/platform/Quadruple.png",
    "images/platform/Single-angled.png",
    "images/platform/Single-flat.png",
    "images/platform/Single-square.png",
    "images/platform/Triple.png"
};

std::vector<sf::Texture> loadPlatformTextures()
{
    std::vector<sf::Texture> loadedTextures;
    loadedTextures.reserve(kPlatformTexturePaths.size());

    for (const std::string& path : kPlatformTexturePaths)
    {
        loadedTextures.emplace_back();
        if (!loadedTextures.back().loadFromFile(path))
        {
            loadedTextures.pop_back();
            std::cerr << "Error loading texture: " << path << std::endl;
            continue;
        }

        if (loadedTextures.back().generateMipmap())
        {
            debugLog("Mipmap generated");
        }
        loadedTextures.back().setSmooth(true);
    }

    return loadedTextures;
}
}

const std::vector<sf::Texture>& Platform::getSharedTextures()
{
    static const std::vector<sf::Texture> sharedTextures = loadPlatformTextures();
    return sharedTextures;
}

void Platform::draw(sf::RenderWindow &window)
{
    if constexpr (DRAW_PLATFORM_HITBOXES)
    {
        for (auto &rect : rects)
        {
            window.draw(*rect);
        }
    }

    for (auto &sprite : sprites)
    {
        window.draw(*sprite);
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
        
        auto ptrSprite = std::make_unique<sf::Sprite>(textures->at(4));
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

        auto ptrSprite = std::make_unique<sf::Sprite>(textures->at(5));
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

        auto ptrSprite = std::make_unique<sf::Sprite>(textures->at(6));
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

        auto ptrSprite = std::make_unique<sf::Sprite>(textures->at(0));
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

        auto ptrSprite = std::make_unique<sf::Sprite>(textures->at(1));
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

        auto ptrSprite = std::make_unique<sf::Sprite>(textures->at(2));
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

        auto ptrSprite = std::make_unique<sf::Sprite>(textures->at(7));
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

        auto ptrSprite = std::make_unique<sf::Sprite>(textures->at(3));
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

void Platform::clearPlatforms()
{
    sprites.clear();
    rects.clear();
}

Platform::Platform()
    : textures(&getSharedTextures())
{
}

Platform::~Platform() = default;
