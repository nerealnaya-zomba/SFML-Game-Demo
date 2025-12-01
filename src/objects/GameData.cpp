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
    if(initTextures(satiro_dieTextures, satiro_diePath_, 9,2,1)) succesedOperationsCount_m++;
    loadingScreen_m->update(succesedOperationsCount_m);
    loadingScreen_m->draw();
    generateMipmapTextures(satiro_dieTextures);
    this->satiro_die_helper.countOfTextures = 8;
    this->satiro_die_helper.iterationsTillSwitch = 9;
    if(initTextures(satiro_dashTextures, satiro_dashPath_, 7,2,1)) succesedOperationsCount_m++;
    loadingScreen_m->update(succesedOperationsCount_m);
    loadingScreen_m->draw();
    generateMipmapTextures(satiro_dashTextures);
    this->satiro_dash_helper.countOfTextures = 6;
    this->satiro_dash_helper.iterationsTillSwitch = 9;
    if(initTextures(satiro_hurtTextures, satiro_hurtPath_, 3,2,1)) succesedOperationsCount_m++;
    loadingScreen_m->update(succesedOperationsCount_m);
    loadingScreen_m->draw();
    generateMipmapTextures(satiro_hurtTextures);
    this->satiro_hurt_helper.countOfTextures = 2;
    this->satiro_hurt_helper.iterationsTillSwitch = 9;
    if(initTextures(satiro_slideTextures, satiro_slidePath_, 3,2,1)) succesedOperationsCount_m++;
    loadingScreen_m->update(succesedOperationsCount_m);
    loadingScreen_m->draw();
    generateMipmapTextures(satiro_slideTextures);
    this->satiro_slide_helper.countOfTextures = 2;
    this->satiro_slide_helper.iterationsTillSwitch = 9;


        //SkeletonWhite initialization
    if(initTextures(skeletonWhite_idleTextures_, skeletonWhite_idlePath_, 8,2,1)) succesedOperationsCount_m++;
    loadingScreen_m->update(succesedOperationsCount_m);
    loadingScreen_m->draw();
    generateMipmapTextures(skeletonWhite_idleTextures_);
    //smoothTextures(skeletonWhite_idleTextures_);
    this->skeletonWhite_idle_helper.countOfTextures = 7;
    this->skeletonWhite_idle_helper.iterationsTillSwitch = 9;

    if(initTextures(skeletonWhite_walkTextures, skeletonWhite_walkPath_, 10,2,1)) succesedOperationsCount_m++;
    loadingScreen_m->update(succesedOperationsCount_m);
    loadingScreen_m->draw();
    generateMipmapTextures(skeletonWhite_walkTextures);
    //smoothTextures(skeletonWhite_walkTextures);
    this->skeletonWhite_walk_helper.countOfTextures = 9;
    this->skeletonWhite_walk_helper.iterationsTillSwitch = 9;

    if(initTextures(skeletonWhite_hurtTextures, skeletonWhite_hurtPath_, 5,2,1)) succesedOperationsCount_m++;
    loadingScreen_m->update(succesedOperationsCount_m);
    loadingScreen_m->draw();
    generateMipmapTextures(skeletonWhite_hurtTextures);
    //smoothTextures(skeletonWhite_hurtTextures);
    this->skeletonWhite_hurt_helper.countOfTextures = 4;
    this->skeletonWhite_hurt_helper.iterationsTillSwitch = 9;

    if(initTextures(skeletonWhite_dieTextures, skeletonWhite_diePath_, 13,2,1)) succesedOperationsCount_m++;
    loadingScreen_m->update(succesedOperationsCount_m);
    loadingScreen_m->draw();
    generateMipmapTextures(skeletonWhite_dieTextures);
    //smoothTextures(skeletonWhite_dieTextures);
    this->skeletonWhite_die_helper.countOfTextures = 12;
    this->skeletonWhite_die_helper.iterationsTillSwitch = 9;

    if(initTextures(skeletonWhite_attack1Textures, skeletonWhite_attack1Path_, 10,2,1)) succesedOperationsCount_m++;
    loadingScreen_m->update(succesedOperationsCount_m);
    loadingScreen_m->draw();
    generateMipmapTextures(skeletonWhite_attack1Textures);
    //smoothTextures(skeletonWhite_attack1Textures);
    this->skeletonWhite_attack1_helper.countOfTextures = 9;
    this->skeletonWhite_attack1_helper.iterationsTillSwitch = 6;

    if(initTextures(skeletonWhite_attack2Textures, skeletonWhite_attack2Path_, 9,2,1)) succesedOperationsCount_m++;
    loadingScreen_m->update(succesedOperationsCount_m);
    loadingScreen_m->draw();
    generateMipmapTextures(skeletonWhite_attack2Textures);
    //smoothTextures(skeletonWhite_attack2Textures);
    this->skeletonWhite_attack2_helper.countOfTextures = 8;
    this->skeletonWhite_attack2_helper.iterationsTillSwitch = 6;

        //SkeletonYellow initialization
    if(initTextures(skeletonYellow_idleTextures, skeletonYellow_idlePath_, 8,2,1)) succesedOperationsCount_m++;
    loadingScreen_m->update(succesedOperationsCount_m);
    loadingScreen_m->draw();
    generateMipmapTextures(skeletonYellow_idleTextures);
    //smoothTextures(skeletonYellow_idleTextures);
    this->skeletonYellow_idle_helper.countOfTextures = 7;
    this->skeletonYellow_idle_helper.iterationsTillSwitch = 9;

    if(initTextures(skeletonYellow_walkTextures, skeletonYellow_walkPath_, 10,2,1)) succesedOperationsCount_m++;
    loadingScreen_m->update(succesedOperationsCount_m);
    loadingScreen_m->draw();
    generateMipmapTextures(skeletonYellow_walkTextures);
    //smoothTextures(skeletonYellow_walkTextures);
    this->skeletonYellow_walk_helper.countOfTextures = 9;
    this->skeletonYellow_walk_helper.iterationsTillSwitch = 6;

    if(initTextures(skeletonYellow_hurtTextures, skeletonYellow_hurtPath_, 5,2,1)) succesedOperationsCount_m++;
    loadingScreen_m->update(succesedOperationsCount_m);
    loadingScreen_m->draw();
    generateMipmapTextures(skeletonYellow_hurtTextures);
    //smoothTextures(skeletonYellow_hurtTextures);
    this->skeletonYellow_hurt_helper.countOfTextures = 4;
    this->skeletonYellow_hurt_helper.iterationsTillSwitch = 9;

    if(initTextures(skeletonYellow_dieTextures, skeletonYellow_diePath_, 13,2,1)) succesedOperationsCount_m++;
    loadingScreen_m->update(succesedOperationsCount_m);
    loadingScreen_m->draw();
    generateMipmapTextures(skeletonYellow_dieTextures);
    //smoothTextures(skeletonYellow_dieTextures);
    this->skeletonYellow_die_helper.countOfTextures = 12;
    this->skeletonYellow_die_helper.iterationsTillSwitch = 9;

    if(initTextures(skeletonYellow_attack1Textures, skeletonYellow_attack1Path_, 10,2,1)) succesedOperationsCount_m++;
    loadingScreen_m->update(succesedOperationsCount_m);
    loadingScreen_m->draw();
    generateMipmapTextures(skeletonYellow_attack1Textures);
    //smoothTextures(skeletonYellow_attack1Textures);
    this->skeletonYellow_attack1_helper.countOfTextures = 9;
    this->skeletonYellow_attack1_helper.iterationsTillSwitch = 4;

    if(initTextures(skeletonYellow_attack2Textures, skeletonYellow_attack2Path_, 9,2,1)) succesedOperationsCount_m++;
    loadingScreen_m->update(succesedOperationsCount_m);
    loadingScreen_m->draw();
    generateMipmapTextures(skeletonYellow_attack2Textures);
    //smoothTextures(skeletonYellow_attack2Textures);
    this->skeletonYellow_attack2_helper.countOfTextures = 8;
    this->skeletonYellow_attack2_helper.iterationsTillSwitch = 4;
    
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
    // smoothTextures(cat1Textures);
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
