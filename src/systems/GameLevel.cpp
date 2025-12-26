#include<GameLevel.h>

GameLevelManager::GameLevelManager()
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

GameLevel::GameLevel()
{
}

GameLevel::~GameLevel()
{
}

void GameLevel::update()
{
    for (auto &&i : decorations)
    {
        i->updateTextures();
    }
}

void GameLevel::draw(sf::RenderWindow &window)
{
    for (auto &&i : platforms)
    {
        i->draw(window);
    }
    for (auto &&i : decorations)
    {
        i->draw(window);
    }
    for (auto &&i : background)
    {
        i->drawBackground(window);
    }
    for (auto &&i : ground)
    {
        i->draw(window, 100.f); //FIXME ???? Что за второй аргумент в вызове метода ???? Разобраться
    }
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
