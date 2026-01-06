#include "EnemyManager.h"
#include<Spawner.h>
#include<Skeleton.h>
#include<enemyPortal.h>

std::filesystem::path PATH_TO_LEVELS_FOLDER = "data/levelData";

void EnemyManager::updateSpawner()
{
    for (Spawner &spawner : spawners)
    {
        spawner.update();
    }
    
}

void EnemyManager::removeIfNotAlive()
{
    skeletons.erase(
        std::remove_if(skeletons.begin(), skeletons.end(), 
            [](auto& enemy) {
                return !enemy->isAlive;
            }),
        skeletons.end()
    );
}

void EnemyManager::loadSpawnerData()
{
    // Добавляем название уровня к пути
    PATH_TO_LEVELS_FOLDER /= levelName;
    //Открываем файл
    std::ifstream f(PATH_TO_LEVELS_FOLDER);
    //Проверка открылся ли
    if(!f.good()){
        std::cerr << "Error opening " << PATH_TO_LEVELS_FOLDER << " file does not exist. : EnemyManager\n";
        exit(1);
    }
    // Парсинг json-a
    nlohmann::json d = nlohmann::json::parse(f);
    for (auto &&spawner : d["Spawners"])
    {   
        // Координаты спавна
        sf::Vector2f spawnArea[2] = {
            {spawner["SpawnArea"][0][0],spawner["SpawnArea"][0][1]},
            {spawner["SpawnArea"][1][0],spawner["SpawnArea"][1][1]}
        };
        // Вставляем спавнер
        spawners.emplace_back(
            *this,
            spawner["EnemyName"],
            spawner["EnemyAmount"],
            spawner["SpawnCooldown"],
            spawnArea);
    }
}

void EnemyManager::addSkeleton(GameData& data,sf::RenderWindow& window,Ground& ground,Platform& platform,Player& player,std::string type,sf::Vector2f pos)
{
    skeletons.push_back(std::make_shared<Skeleton>(data,window,ground,platform,player,type,pos)); 
}   

void EnemyManager::updateAI_all()
{
    for (auto enemy : skeletons) {
        enemy->updateAI();
    }
}

void EnemyManager::updateControls_all()
{
    for (auto enemy : skeletons) {
        enemy->updateControl();
    }
}

void EnemyManager::updatePhysics_all()
{
    for (auto enemy : skeletons) {
        enemy->updatePhysics();
    }
}

void EnemyManager::updateTextures_all()
{
    for (auto enemy : skeletons) {
        enemy->updateTextures();
    }
    this->removeIfNotAlive();
}

void EnemyManager::updateSpawners_all()
{
    for (auto &&i : spawners)
    {
        i.update();
    }
    
}

void EnemyManager::draw_all()
{
    for (auto enemy : skeletons) {
        enemy->draw();
    }
}

void EnemyManager::addSpawner(std::string enemyName)
{
}

EnemyManager::EnemyManager(std::string ln)
    : levelName(ln)
{
    loadSpawnerData();
}

EnemyManager::~EnemyManager()
{
}