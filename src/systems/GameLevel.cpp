#include<GameLevel.h>

void GameLevelManager::initializeLevels(const std::string levelsFolder)
{
    std::filesystem::path p(levelsFolder);
    
    //Проходимся по всем файлам директории
    for (auto const &dir_entry : std::filesystem::directory_iterator{p})
    {
        std::string onlyFileName = dir_entry.path().filename().u8string();
        std::string fileNamePath = dir_entry.path().u8string();

        // Вставляем в хранилище
        levels.emplace(
            // Имя файла
        onlyFileName, 
            // Помещаем shared_ptr<GameLevel> в хранилище
        std::make_shared<GameLevel>(*this->data,*this->player,*this->camera,*this, fileNamePath));
    }
}

GameLevelManager::GameLevelManager(GameData &d, Player &p, GameCamera& c, const std::string& levelsFolder)
    : data(&d), player(&p), camera(&c)
{
        

    initializeLevels(levelsFolder);     // Инициализирует все уровни

    levelIt = levels.find("level1.json");  

}

GameLevelManager::~GameLevelManager()
{
}

void GameLevelManager::goToLevel(std::string levelName)
{
    if(levels.empty()) return;

    levelIt->second->saveLevelData(); // Сохранить прошлый уровень перед переходом

    levelIt = levels.find(levelName);      
    
    levelIt->second->loadLevelData(levelName); // Загрузить следующий уровень после перехода
}

void GameLevelManager::update()
{
    if(levels.empty()) return;

    levelIt->second->update();
}

void GameLevelManager::updatePlatforms()
{
    levelIt->second->updatePlatforms();
}

void GameLevelManager::updateDecorations()
{
    levelIt->second->updateDecorations();
}

void GameLevelManager::updateBackgrounds()
{
    levelIt->second->updateBackgrounds();
}

void GameLevelManager::updateGrounds()
{
    levelIt->second->updateGrounds();
}

void GameLevelManager::draw(sf::RenderWindow &window)
{
    if(levels.empty()) return;

    levelIt->second->draw(window);
}

void GameLevelManager::drawPlatforms(sf::RenderWindow &window)
{
    levelIt->second->drawPlatforms(window);
}

void GameLevelManager::drawDecorations(sf::RenderWindow &window)
{
    levelIt->second->drawDecorations(window);
}

void GameLevelManager::drawBackgrounds(sf::RenderWindow &window)
{
    levelIt->second->drawBackgrounds(window);
}

void GameLevelManager::drawGrounds(sf::RenderWindow &window)
{
    levelIt->second->drawGrounds(window);
}

sf::Vector2f GameLevelManager::getCurrentLevelSize() const
{
    return levelIt->second->getLevelSize();
}

std::vector<std::shared_ptr<sf::RectangleShape>> &GameLevelManager::getPlatformRects()
{
    return levelIt->second->getPlatformRects();
}

sf::RectangleShape &GameLevelManager::getGroundRect()
{
    return levelIt->second->getGroundRect();
}

GameLevel::GameLevel(GameData& d, Player& p, GameCamera& c, GameLevelManager& m, const std::string& fileNamePath)
    : data(&d), player(&p), camera(&c), manager(&m)
{
    loadLevelData(fileNamePath);
}

GameLevel::~GameLevel()
{
}

void GameLevel::updatePlatforms()
{
}

void GameLevel::updateDecorations()
{
    decorations->updateTextures();
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
    platforms->draw(window);
}

void GameLevel::drawDecorations(sf::RenderWindow& window)
{
    decorations->draw(window);
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

void GameLevel::initializePlatforms(const nlohmann::json& data)
{
    platforms = std::make_shared<Platform>();

    for (const auto &platform : data["Platforms"])
    {
        sf::Vector2f position = {
            platform["Position"][0],
            platform["Position"][1]};

        std::string type = platform["Type"];

        platforms->addPlatform(position,type);
    }
}

void GameLevel::initializeDecorations(const nlohmann::json& data)
{
    decorations = std::make_shared<Decoration>(*this->data,*this->camera);

    for (const auto &decoration : data["Decorations"])
    {
        std::string name = decoration["Name"];
        sf::Vector2f position = {decoration["Position"][0],decoration["Position"][1]};
        sf::Vector2f scale = {decoration["Scale"][0],decoration["Scale"][1]};
        sf::Color color = sf::Color{decoration["Color"][0],decoration["Color"][1],decoration["Color"][2],decoration["Color"][3]};
        sf::Vector2f parallaxFactor = {decoration["ParallaxFactor"][0],decoration["ParallaxFactor"][1]};
        int ZDepth = decoration["Z"];

        decorations->addDecoration(name,position,scale,parallaxFactor,ZDepth,color);
    }
}

void GameLevel::initializeBackground(const nlohmann::json& data)
{
    for (const auto &background : data["Background"])
    {
        sf::Vector2f position = {background["Position"][0],background["Position"][1]};
        sf::Vector2f parallaxFactor = {background["ParallaxFactor"][0],background["ParallaxFactor"][1]};
        std::string name = background["BgName"];
        Type type = background["Type"];

        this->background.push_back(
            std::make_shared<Background>(
                *this->data,
                *this->camera,
                position,name,
                parallaxFactor,
                type));
    }
}

void GameLevel::initializeGround(const nlohmann::json& data)
{
    for (const auto &ground : data["Ground"])
    {
        std::string groundName = ground["GroundName"];
        sf::Vector2u position = {ground["Points"][0],ground["Points"][1]};
        unsigned int yPos = ground["YPos"];

        this->ground.push_back(
            std::make_shared<Ground>(
                *this->data,
                *this->manager,
                groundName,
                position.x,
                position.y,
                yPos));
    }
}

void GameLevel::draw(sf::RenderWindow &window)
{
    drawBackgrounds(window);
    drawDecorations(window);
    drawGrounds(window);
    drawPlatforms(window);
}

void GameLevel::loadLevelData(const std::string& fileNamePath)
{
    std::ifstream dataFile(fileNamePath);
    if(!dataFile.good()){
        std::cerr << "Error reading level's json data:\n\t" << fileNamePath << " not found!\n";
        exit(EXIT_FAILURE);
    }

    nlohmann::json data = nlohmann::json::parse(dataFile);

    initializePlatforms(data);

    initializeDecorations(data);

    initializeBackground(data);

    initializeGround(data);

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

std::vector<std::shared_ptr<sf::RectangleShape>> &GameLevel::getPlatformRects()
{
    return this->platforms->getRects();
}

sf::RectangleShape &GameLevel::getGroundRect()
{
    if(ground.empty()) exit(EXIT_FAILURE);

    return ground.at(0)->getRect();
}
