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
