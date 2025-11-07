#include "GameData.h"

GameData::GameData(sf::RenderWindow* window,sf::Font* font)
{
    //Loading data from launchSettings.json
    loadData();

    //LoadingScreen initialization
    loadingScreen_m = new LoadingScreen(window,font,allOperations_count_m);

    //Textures
        //Satiro initialization
    if(initSatiroTextures(idleTextures,idleTexturesPaths)) succesedOperationsCount_m++;
    loadingScreen_m->update(succesedOperationsCount_m);
    loadingScreen_m->draw();
    if(initSatiroTextures(runningTextures,runningTexturesPaths)) succesedOperationsCount_m++;
    loadingScreen_m->update(succesedOperationsCount_m);
    loadingScreen_m->draw();
    if(initSatiroTextures(fallingTextures,fallingTexturesPaths)) succesedOperationsCount_m++;
    loadingScreen_m->update(succesedOperationsCount_m);
    loadingScreen_m->draw();
    if(initSatiroTextures(bulletTextures,bulletTexturesPaths)) succesedOperationsCount_m++;
    loadingScreen_m->update(succesedOperationsCount_m);
    loadingScreen_m->draw();

        //Plants initialization
    if(initTextures(plant1Textures,plant1Path, 89)) succesedOperationsCount_m++;
    loadingScreen_m->update(succesedOperationsCount_m);
    loadingScreen_m->draw();
    generateMipmapTextures(plant1Textures);
    smoothTextures(plant1Textures);
    this->plant1.countOfTextures = 89;
    this->plant1.iterationsTillSwitch = 8;

    if(initTextures(plant2Textures,plant2Path, 89)) succesedOperationsCount_m++;
    loadingScreen_m->update(succesedOperationsCount_m);
    loadingScreen_m->draw();
    generateMipmapTextures(plant2Textures);
    smoothTextures(plant2Textures);
    this->plant2.countOfTextures = 89;
    this->plant2.iterationsTillSwitch = 8;

    if(initTextures(plant3Textures,plant3Path, 89)) succesedOperationsCount_m++;
    loadingScreen_m->update(succesedOperationsCount_m);
    loadingScreen_m->draw();
    generateMipmapTextures(plant3Textures);
    smoothTextures(plant3Textures);
    this->plant3.countOfTextures = 89;
    this->plant3.iterationsTillSwitch = 8;

    if(initTextures(plant4Textures,plant4Path, 59)) succesedOperationsCount_m++;
    loadingScreen_m->update(succesedOperationsCount_m);
    loadingScreen_m->draw();
    generateMipmapTextures(plant4Textures);
    smoothTextures(plant4Textures);
    this->plant4.countOfTextures = 59;
    this->plant4.iterationsTillSwitch = 6;

    if(initTextures(plant5Textures,plant5Path, 59)) succesedOperationsCount_m++;
    loadingScreen_m->update(succesedOperationsCount_m);
    loadingScreen_m->draw();
    generateMipmapTextures(plant5Textures);
    smoothTextures(plant5Textures);
    this->plant5.countOfTextures = 59;
    this->plant5.iterationsTillSwitch = 9;

    if(initTextures(plant6Textures,plant6Path, 59)) succesedOperationsCount_m++;
    loadingScreen_m->update(succesedOperationsCount_m);
    loadingScreen_m->draw();
    generateMipmapTextures(plant6Textures);
    smoothTextures(plant6Textures);
    this->plant6.countOfTextures = 59;
    this->plant6.iterationsTillSwitch = 9;

    if(initTextures(plant7Textures,plant7Path, 59)) succesedOperationsCount_m++;
    loadingScreen_m->update(succesedOperationsCount_m);
    loadingScreen_m->draw();
    generateMipmapTextures(plant7Textures);
    smoothTextures(plant7Textures);
    this->plant7.countOfTextures = 59;
    this->plant7.iterationsTillSwitch = 9;

        //Cat initialization
    if(initTextures(cat1Textures,cat1Path, 2, 2)) succesedOperationsCount_m++;
    loadingScreen_m->update(succesedOperationsCount_m);
    loadingScreen_m->draw();
    generateMipmapTextures(cat1Textures);
    smoothTextures(cat1Textures);
    this->catHelper.countOfTextures = 2;
    this->catHelper.iterationsTillSwitch = 72;

        //jumpPlant initialization
    if(initTextures(jumpPlantTextures,jumpPlantPath,19)) succesedOperationsCount_m++;
    loadingScreen_m->update(succesedOperationsCount_m);
    loadingScreen_m->draw();
    generateMipmapTextures(jumpPlantTextures);
    smoothTextures(jumpPlantTextures);
    this->jumpPlant.countOfTextures = 19;
    this->jumpPlant.iterationsTillSwitch = 5;

        //Ground initialization
    ground1Texture.loadFromFile(ground1Path) ? std::cout << "Texture loaded: images/Ground/mramoric.png" << std::endl : std::cout << "Error loading texture: images/Ground/mramoric.png" << std::endl;

    succesedOperationsCount_m++;
    succesedOperationsCount_m++;
    succesedOperationsCount_m++;

    //Save load info
    if(succesedOperationsCount_m!=allOperations_count_m)
    {
        saveOperationsData();
    }

}

GameData::~GameData()
{

}

bool GameData::initSatiroTextures(std::vector<sf::Texture> &textures, std::vector<std::string> paths)
{
    for (size_t i = 0; i < paths.size(); i++)
    {
        sf::Texture* texture = new sf::Texture();
        if(!texture->loadFromFile(paths[i]))
        {
            std::cout << "Error loading texture: " << paths[i] << std::endl;
            return false;
        }
        else
        {
            std::cout << "Texture loaded: " << paths[i] << std::endl;
        }
        textures.push_back(*texture);
    }
    return true;
}

void GameData::generateMipmapTextures(std::vector<sf::Texture> &texturesArray)
{
    for (auto &i : texturesArray)
    {
        if(i.generateMipmap())
        {
            std::cout << "Mipmap generated\n"; 
        }
        else
        {
            std::cout << "Error while generating mipmap\n";
        }
    }
}

void GameData::smoothTextures(std::vector<sf::Texture> &texturesArray)
{
    for (auto &i : texturesArray)
    {
        i.setSmooth(true);
    }
}

//
void GameData::saveOperationsData()
{
    // Чтение
    std::ifstream in("data/launchSettings.json");
    nlohmann::json j = nlohmann::json::parse(in);
    in.close();  // Закрыть для чтения


    // Изменение
    j["loadingGameAssets_operationsCount"] = succesedOperationsCount_m;
    
    // Запись
    std::ofstream out("data/launchSettings.json");
    out << j.dump(4);
}

void GameData::loadData()
{
    std::fstream f("data/launchSettings.json");
    nlohmann::json j = nlohmann::json::parse(f);
    allOperations_count_m = j["loadingGameAssets_operationsCount"];
}
