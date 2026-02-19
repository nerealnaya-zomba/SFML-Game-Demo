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
        std::make_shared<GameLevel>(
            *this->data,
            *this->player,
            *this->camera,
            *this,
            *this->window, 
            onlyFileName
        ));
    }
}

GameLevelManager::GameLevelManager(GameData &d, GameCamera& c,sf::RenderWindow& w, const std::string& lF)
    : data(&d), camera(&c), window(&w), levelsFolder(lF)
{
    initializeLevels(levelsFolder);     // Инициализирует все уровни
    
    levelIt = levels.find("level1.json");  
}

GameLevelManager::~GameLevelManager()
{
}

bool GameLevelManager::goToLevel(std::optional<std::string> levelName)
{
    if(levels.empty())
    {
        std::cerr << "LevelManager does not consist any level" << std::endl;
        exit(1);
    }
    
    if( levelName.has_value() && levels.find(levelName.value()) != levels.end() )
    {
        levelIt->second->saveLevelData(); // Сохранить прошлый уровень перед переходом
        
        levelIt->second->clearLevel();

        levelIt = levels.find(levelName.value());      
    
        levelIt->second->loadLevelData(levelName.value()); // Загрузить следующий уровень после перехода

        player->setPosition(levelIt->second->getPlayerSpawnPos());
        camera->setCenterPosition(levelIt->second->getPlayerSpawnPos());

        return true;
    }
    
    return false;
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

void GameLevelManager::updateEnemyManager()
{
    levelIt->second->updateEnemyManager();
}

void GameLevelManager::draw()
{
    if(levels.empty()) return;

    levelIt->second->draw();
}

void GameLevelManager::drawPlatforms()
{
    levelIt->second->drawPlatforms();
}

void GameLevelManager::drawDecorations()
{
    levelIt->second->drawDecorations();
}

void GameLevelManager::drawBackgrounds()
{
    levelIt->second->drawBackgrounds();
}

void GameLevelManager::drawGrounds()
{
    levelIt->second->drawGrounds();
}

void GameLevelManager::drawEnemyManager()
{
    levelIt->second->drawEnemyManager();
}

sf::Vector2i GameLevelManager::getCurrentLevelSize() const
{   
    return levelIt->second->getLevelSize();
}

std::string GameLevelManager::getCurrentLevelName() const
{
    return levelIt->second->levelName;
}

std::vector<std::shared_ptr<sf::RectangleShape>> &GameLevelManager::getPlatformRects()
{
    if(levelIt->second == nullptr){
        std::cerr << "Level doesn't exist\n";
        exit(1);
    }
    return levelIt->second->getPlatformRects();
}

sf::RectangleShape &GameLevelManager::getGroundRect()
{
    return levelIt->second->getGroundRect();
}

const std::map<std::string, std::shared_ptr<GameLevel>> &GameLevelManager::getLevelsMap() const
{
    return this->levels;
}

std::map<std::string, std::shared_ptr<GameLevel>>::iterator &GameLevelManager::getIteratorReference()
{
    this->levelIt;
}

void GameLevelManager::attachPlayer(Player& p)
{
    this->player = &p;
    for (auto &&level : levels)
    {
        level.second->attachPlayer(p);
    }
}

GameLevel::GameLevel(GameData& d, Player& p, GameCamera& c, GameLevelManager& m, sf::RenderWindow& w, const std::string& fileNamePath)
    : data(&d), player(&p), camera(&c), levelManager(&m), window(&w)
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

void GameLevel::updateEnemyManager()
{
    enemyManager->updateAI_all();
    enemyManager->updateControls_all();
    enemyManager->updatePhysics_all();
    enemyManager->updateSpawners_all();
    enemyManager->updateTextures_all();
}

void GameLevel::update()
{
    updatePlatforms();
    updateDecorations();
    updateBackgrounds();
    updateGrounds();
}

void GameLevel::drawPlatforms()
{
    platforms->draw(*window);
}

void GameLevel::drawDecorations()
{
    decorations->draw(*window);
}

void GameLevel::drawBackgrounds()
{
    for (auto &&i : background)
    {
        i->draw(*window);
    }
}

void GameLevel::drawGrounds()
{
        ground->draw(*window);
}

void GameLevel::drawEnemyManager()
{
    enemyManager->draw_all();
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
                *this,
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
        
        this->ground =std::make_shared<Ground>(
                *this->data,
                *this,
                groundName,
                position.x,
                position.y,
                yPos
            );
    }
}

void GameLevel::initializeEnemyManager(const nlohmann::json &data)
{
    enemyManager = new EnemyManager(data, 
        *this->data, 
        *platforms,
        *ground,
        *player,
        *window
    );
}

void GameLevel::draw()
{
    drawBackgrounds();
    drawDecorations();
    drawGrounds();
    drawPlatforms();
}

void GameLevel::loadLevelData(const std::string& fileNamePath)
{   
    // Пробуем открыть файл
    std::ifstream dataFile(LEVELS_FOLDER+fileNamePath);
    if(!dataFile.good()){
        std::cerr << "Error reading level's json data:\n\t" << LEVELS_FOLDER+fileNamePath << " not found!\n";
        exit(EXIT_FAILURE);
    }

    nlohmann::json data = nlohmann::json::parse(dataFile);

    this->size = sf::Vector2i(data["Presets"]["Size"][0],data["Presets"]["Size"][1]);

    initializePlatforms(data);

    initializeDecorations(data);

    initializeBackground(data);

    initializeGround(data);

    initializeEnemyManager(data);

    playerSpawnPos = {data["Presets"]["PlayerSpawn"][0],data["Presets"]["PlayerSpawn"][1]};
}

void GameLevel::clearLevel()
{
    platforms->clearPlatforms();
    decorations->clearDecorations();
    background.clear();
}

void GameLevel::saveLevelData()
{
}

void GameLevel::resetTobase()
{
}

sf::Vector2i GameLevel::getLevelSize() const
{
    return this->size;
}

std::vector<std::shared_ptr<sf::RectangleShape>> &GameLevel::getPlatformRects()
{
    return this->platforms->getRects();
}

sf::RectangleShape &GameLevel::getGroundRect()
{
    return ground->getRect();
}

sf::Vector2f GameLevel::getPlayerSpawnPos()
{
    return playerSpawnPos;
}

void GameLevel::attachPlayer(Player &p)
{
    this->player = &p;
    enemyManager->attachPlayer(p);
}
