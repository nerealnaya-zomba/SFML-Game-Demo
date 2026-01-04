#include<GameLevel.h>

GameLevelManager::GameLevelManager(GameData& d, Player& p)
    :  data(&d), player(&p) 
{
    this->levelIt = levels.begin();
}

GameLevelManager::~GameLevelManager()
{
}

void GameLevelManager::goToLevel(std::string name)
{
    if(levels.empty()) return;

    levelIt->second->saveLevelData(); // Сохранить прошлый уровень перед переходом

    levelIt = levels.find(name);      
    
    levelIt->second->loadLevelData(); // Загрузить следующий уровень после перехода
}

void GameLevelManager::update()
{
    if(levels.empty()) return;

    levelIt->second->update();
}

void GameLevelManager::draw(sf::RenderWindow &window)
{
    if(levels.empty()) return;

    levelIt->second->draw(window);
}

sf::Vector2f GameLevelManager::getCurrentLevelSize() const
{
    return levelIt->second->getLevelSize();
}

GameLevel::GameLevel(GameData& d, Player& p)
{
    
}

GameLevel::~GameLevel()
{
}

void GameLevel::updatePlatforms()
{
}

void GameLevel::updateDecorations()
{
    for (auto &&i : decorations)
    {
        i->updateTextures();
    }
}

void GameLevel::updateBackgrounds()
{
    for (auto &&i : background)
    {
        i->update();
    }
}

void GameLevel::updateGrounds()
{
}

void GameLevel::update()
{
    updatePlatforms();
    updateDecorations();
    updateBackgrounds();
    updateGrounds();
}

void GameLevel::drawPlatforms(sf::RenderWindow& window)
{
    for (auto &&i : platforms)
    {
        i->draw(window);
    }
}

void GameLevel::drawDecorations(sf::RenderWindow& window)
{
    for (auto &&i : decorations)
    {
        i->draw(window);
    }
}

void GameLevel::drawBackgrounds(sf::RenderWindow& window)
{
    for (auto &&i : background)
    {
        i->draw(window);
    }
}

void GameLevel::drawGrounds(sf::RenderWindow& window)
{
    for (auto &&i : ground)
    {
        i->draw(window);
    }
}

void GameLevel::draw(sf::RenderWindow &window)
{
    void drawPlatforms(sf::RenderWindow& window);
    void drawDecorations(sf::RenderWindow& window);
    void drawBackgrounds(sf::RenderWindow& window);
    void drawGrounds(sf::RenderWindow& window);
}

void GameLevel::loadLevelData()
{
    
}

void GameLevel::saveLevelData()
{
}

void GameLevel::resetTobase()
{
}

sf::Vector2f GameLevel::getLevelSize() const
{
    return this->size;
}
