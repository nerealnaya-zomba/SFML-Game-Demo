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
    // Player bullet textures
    if(loadTexture(bulletTextures, bulletTexturesPath_, satiro_bullet_helper, 7)) succesedOperationsCount_m++;
    loadingScreen_m->update(succesedOperationsCount_m);
    loadingScreen_m->draw();
    generateMipmapTextures(bulletTextures);
    
    // Player animation textures
    if(loadTexture(satiro_dieTextures, satiro_diePath_, satiro_die_helper, 20)) succesedOperationsCount_m++;
    loadingScreen_m->update(succesedOperationsCount_m);
    loadingScreen_m->draw();
    generateMipmapTextures(satiro_dieTextures);
    
    if(loadTexture(satiro_dashTextures, satiro_dashPath_, satiro_dash_helper, 9)) succesedOperationsCount_m++;
    loadingScreen_m->update(succesedOperationsCount_m);
    loadingScreen_m->draw();
    generateMipmapTextures(satiro_dashTextures);
    
    if(loadTexture(satiro_hurtTextures, satiro_hurtPath_, satiro_hurt_helper, 15)) succesedOperationsCount_m++;
    loadingScreen_m->update(succesedOperationsCount_m);
    loadingScreen_m->draw();
    generateMipmapTextures(satiro_hurtTextures);
    
    if(loadTexture(satiro_slideTextures, satiro_slidePath_, satiro_slide_helper, 9)) succesedOperationsCount_m++;
    loadingScreen_m->update(succesedOperationsCount_m);
    loadingScreen_m->draw();
    generateMipmapTextures(satiro_slideTextures);

    if(loadTexture(satiro_jumpTextures, satiro_jumpPath_, satiro_jump_helper, 9)) succesedOperationsCount_m++;
    loadingScreen_m->update(succesedOperationsCount_m);
    loadingScreen_m->draw();
    generateMipmapTextures(satiro_jumpTextures);

    if(loadTexture(satiro_landingTextures, satiro_landingPath_, satiro_landing_helper, 9)) succesedOperationsCount_m++;
    loadingScreen_m->update(succesedOperationsCount_m);
    loadingScreen_m->draw();
    generateMipmapTextures(satiro_landingTextures);

    // White skeleton textures
    if(loadTexture(skeletonWhite_idleTextures_, skeletonWhite_idlePath_, skeletonWhite_idle_helper, 9)) succesedOperationsCount_m++;
    loadingScreen_m->update(succesedOperationsCount_m);
    loadingScreen_m->draw();
    generateMipmapTextures(skeletonWhite_idleTextures_);

    if(loadTexture(skeletonWhite_walkTextures, skeletonWhite_walkPath_, skeletonWhite_walk_helper, 9)) succesedOperationsCount_m++;
    loadingScreen_m->update(succesedOperationsCount_m);
    loadingScreen_m->draw();
    generateMipmapTextures(skeletonWhite_walkTextures);

    if(loadTexture(skeletonWhite_hurtTextures, skeletonWhite_hurtPath_, skeletonWhite_hurt_helper, 9)) succesedOperationsCount_m++;
    loadingScreen_m->update(succesedOperationsCount_m);
    loadingScreen_m->draw();
    generateMipmapTextures(skeletonWhite_hurtTextures);

    if(loadTexture(skeletonWhite_dieTextures, skeletonWhite_diePath_, skeletonWhite_die_helper, 9)) succesedOperationsCount_m++;
    loadingScreen_m->update(succesedOperationsCount_m);
    loadingScreen_m->draw();
    generateMipmapTextures(skeletonWhite_dieTextures);

    if(loadTexture(skeletonWhite_attack1Textures, skeletonWhite_attack1Path_, skeletonWhite_attack1_helper, 6)) succesedOperationsCount_m++;
    loadingScreen_m->update(succesedOperationsCount_m);
    loadingScreen_m->draw();
    generateMipmapTextures(skeletonWhite_attack1Textures);

    if(loadTexture(skeletonWhite_attack2Textures, skeletonWhite_attack2Path_, skeletonWhite_attack2_helper, 6)) succesedOperationsCount_m++;
    loadingScreen_m->update(succesedOperationsCount_m);
    loadingScreen_m->draw();
    generateMipmapTextures(skeletonWhite_attack2Textures);

    // Yellow skeleton textures
    if(loadTexture(skeletonYellow_idleTextures, skeletonYellow_idlePath_, skeletonYellow_idle_helper, 9)) succesedOperationsCount_m++;
    loadingScreen_m->update(succesedOperationsCount_m);
    loadingScreen_m->draw();
    generateMipmapTextures(skeletonYellow_idleTextures);

    if(loadTexture(skeletonYellow_walkTextures, skeletonYellow_walkPath_, skeletonYellow_walk_helper, 6)) succesedOperationsCount_m++;
    loadingScreen_m->update(succesedOperationsCount_m);
    loadingScreen_m->draw();
    generateMipmapTextures(skeletonYellow_walkTextures);

    if(loadTexture(skeletonYellow_hurtTextures, skeletonYellow_hurtPath_, skeletonYellow_hurt_helper, 9)) succesedOperationsCount_m++;
    loadingScreen_m->update(succesedOperationsCount_m);
    loadingScreen_m->draw();
    generateMipmapTextures(skeletonYellow_hurtTextures);

    if(loadTexture(skeletonYellow_dieTextures, skeletonYellow_diePath_, skeletonYellow_die_helper, 9)) succesedOperationsCount_m++;
    loadingScreen_m->update(succesedOperationsCount_m);
    loadingScreen_m->draw();
    generateMipmapTextures(skeletonYellow_dieTextures);

    if(loadTexture(skeletonYellow_attack1Textures, skeletonYellow_attack1Path_, skeletonYellow_attack1_helper, 4)) succesedOperationsCount_m++;
    loadingScreen_m->update(succesedOperationsCount_m);
    loadingScreen_m->draw();
    generateMipmapTextures(skeletonYellow_attack1Textures);

    if(loadTexture(skeletonYellow_attack2Textures, skeletonYellow_attack2Path_, skeletonYellow_attack2_helper, 4)) succesedOperationsCount_m++;
    loadingScreen_m->update(succesedOperationsCount_m);
    loadingScreen_m->draw();
    generateMipmapTextures(skeletonYellow_attack2Textures);
    
    // Plants textures
    if(loadTexture(plant1Textures, plant1Path, plant1, 8)) succesedOperationsCount_m++;
    loadingScreen_m->update(succesedOperationsCount_m);
    loadingScreen_m->draw();
    generateMipmapTextures(plant1Textures);
    smoothTextures(plant1Textures);

    if(loadTexture(plant2Textures, plant2Path, plant2, 8)) succesedOperationsCount_m++;
    loadingScreen_m->update(succesedOperationsCount_m);
    loadingScreen_m->draw();
    generateMipmapTextures(plant2Textures);
    smoothTextures(plant2Textures);

    if(loadTexture(plant3Textures, plant3Path, plant3, 8)) succesedOperationsCount_m++;
    loadingScreen_m->update(succesedOperationsCount_m);
    loadingScreen_m->draw();
    generateMipmapTextures(plant3Textures);
    smoothTextures(plant3Textures);

    if(loadTexture(plant4Textures, plant4Path, plant4, 6)) succesedOperationsCount_m++;
    loadingScreen_m->update(succesedOperationsCount_m);
    loadingScreen_m->draw();
    generateMipmapTextures(plant4Textures);
    smoothTextures(plant4Textures);

    if(loadTexture(plant5Textures, plant5Path, plant5, 9)) succesedOperationsCount_m++;
    loadingScreen_m->update(succesedOperationsCount_m);
    loadingScreen_m->draw();
    generateMipmapTextures(plant5Textures);
    smoothTextures(plant5Textures);

    if(loadTexture(plant6Textures, plant6Path, plant6, 9)) succesedOperationsCount_m++;
    loadingScreen_m->update(succesedOperationsCount_m);
    loadingScreen_m->draw();
    generateMipmapTextures(plant6Textures);
    smoothTextures(plant6Textures);

    if(loadTexture(plant7Textures, plant7Path, plant7, 9)) succesedOperationsCount_m++;
    loadingScreen_m->update(succesedOperationsCount_m);
    loadingScreen_m->draw();
    generateMipmapTextures(plant7Textures);
    smoothTextures(plant7Textures);
    
    // Cat decoration
    if(loadTexture(cat1Textures, cat1Path, catHelper, 72)) succesedOperationsCount_m++;
    loadingScreen_m->update(succesedOperationsCount_m);
    loadingScreen_m->draw();
    generateMipmapTextures(cat1Textures);

    // Jump plant
    if(loadTexture(jumpPlantTextures, jumpPlantPath, jumpPlant, 5)) succesedOperationsCount_m++;
    loadingScreen_m->update(succesedOperationsCount_m);
    loadingScreen_m->draw();
    generateMipmapTextures(jumpPlantTextures);
    smoothTextures(jumpPlantTextures);
    
    // Portal green
    if(loadTexture(portalGreenTextures, portalGreenPath, portalGreen, 5)) succesedOperationsCount_m++;
    loadingScreen_m->update(succesedOperationsCount_m);
    loadingScreen_m->draw();
    generateMipmapTextures(portalGreenTextures);
    smoothTextures(portalGreenTextures);

    //PortalBlue
    int portalBluePauseTillSwitch = 2;
    if(loadTexture(portalBlue1Textures, PortalBlue1Path, portalBlue1Helper, portalBluePauseTillSwitch)) succesedOperationsCount_m++;
    loadingScreen_m->update(succesedOperationsCount_m);
    loadingScreen_m->draw();
    generateMipmapTextures(portalBlue1Textures);
    smoothTextures(portalBlue1Textures);

    if(loadTexture(portalBlue2Textures, PortalBlue2Path, portalBlue2Helper, portalBluePauseTillSwitch)) succesedOperationsCount_m++;
    loadingScreen_m->update(succesedOperationsCount_m);
    loadingScreen_m->draw();
    generateMipmapTextures(portalBlue2Textures);
    smoothTextures(portalBlue2Textures);

    if(loadTexture(portalBlue3Textures, PortalBlue3Path, portalBlue3Helper, portalBluePauseTillSwitch)) succesedOperationsCount_m++;
    loadingScreen_m->update(succesedOperationsCount_m);
    loadingScreen_m->draw();
    generateMipmapTextures(portalBlue3Textures);
    smoothTextures(portalBlue3Textures);

    if(loadTexture(portalBlue4Textures, PortalBlue4Path, portalBlue4Helper, portalBluePauseTillSwitch)) succesedOperationsCount_m++;
    loadingScreen_m->update(succesedOperationsCount_m);
    loadingScreen_m->draw();
    generateMipmapTextures(portalBlue4Textures);
    smoothTextures(portalBlue4Textures);

    if(loadTexture(portalBlue5Textures, PortalBlue5Path, portalBlue5Helper, portalBluePauseTillSwitch)) succesedOperationsCount_m++;
    loadingScreen_m->update(succesedOperationsCount_m);
    loadingScreen_m->draw();
    generateMipmapTextures(portalBlue5Textures);
    smoothTextures(portalBlue5Textures);

    if(loadTexture(portalBlue6Textures, PortalBlue6Path, portalBlue6Helper, portalBluePauseTillSwitch)) succesedOperationsCount_m++;
    loadingScreen_m->update(succesedOperationsCount_m);
    loadingScreen_m->draw();
    generateMipmapTextures(portalBlue6Textures);
    smoothTextures(portalBlue6Textures);

    if(loadTexture(portalBlue7Textures, PortalBlue7Path, portalBlue7Helper, portalBluePauseTillSwitch)) succesedOperationsCount_m++;
    loadingScreen_m->update(succesedOperationsCount_m);
    loadingScreen_m->draw();
    generateMipmapTextures(portalBlue7Textures);
    smoothTextures(portalBlue7Textures);

    if(loadTexture(portalBlue8Textures, PortalBlue8Path, portalBlue8Helper, portalBluePauseTillSwitch)) succesedOperationsCount_m++;
    loadingScreen_m->update(succesedOperationsCount_m);
    loadingScreen_m->draw();
    generateMipmapTextures(portalBlue8Textures);
    smoothTextures(portalBlue8Textures);

    //Static-textures
        //Mossy
    if(loadTexture(allStaticTextures,MossyBackgroundDecorationsPath, false)) succesedOperationsCount_m++;
    loadingScreen_m->update(succesedOperationsCount_m);
    loadingScreen_m->draw();

    if(loadTexture(allStaticTextures,MossyDecorationsHazardsPath, false)) succesedOperationsCount_m++;
    loadingScreen_m->update(succesedOperationsCount_m);
    loadingScreen_m->draw();

    if(loadTexture(allStaticTextures,MossyHangingPlantsPath, false)) succesedOperationsCount_m++;
    loadingScreen_m->update(succesedOperationsCount_m);
    loadingScreen_m->draw();

    if(loadTexture(allStaticTextures,MossyHillsPath, false)) succesedOperationsCount_m++;
    loadingScreen_m->update(succesedOperationsCount_m);
    loadingScreen_m->draw();

    if(loadTexture(allStaticTextures,MossyTileSetPath, false)) succesedOperationsCount_m++;
    loadingScreen_m->update(succesedOperationsCount_m);
    loadingScreen_m->draw();
    generateMipmapTextures(allStaticTextures);
    smoothTextures(allStaticTextures);

    // Ground initialization
        // TileSetGreen
    if(loadTexture(TileSetGreenTextures,groundTileSetGreenPath, true)) succesedOperationsCount_m++;
    loadingScreen_m->update(succesedOperationsCount_m);
    loadingScreen_m->draw();
    generateMipmapTextures(TileSetGreenTextures);
    smoothTextures(TileSetGreenTextures);

    //Background initialization
    if(loadTexture(backgroundTextures,backgroundPath, true)) succesedOperationsCount_m++;
    loadingScreen_m->update(succesedOperationsCount_m);
    loadingScreen_m->draw();
    generateMipmapTextures(backgroundTextures);
    smoothTextures(backgroundTextures);

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

bool GameData::loadTexture(std::vector<sf::Texture> &textures, std::string path, 
                           texturesIterHelper& helper, int pauseTillSwitch = -1)
{
    TextureSequenceInfo seq = analyzeTextureSequence(path);
    
    if (seq.count == 0) {
        std::cerr << "ERROR: No textures found for path: " << path << std::endl;
        exit(1);
        return false;
    }
    
    // Отладочный вывод
    std::cout << "Loading textures: " << path 
              << ", start=" << seq.startCounter 
              << ", count=" << seq.count 
              << ", digits=" << seq.maxDigits << std::endl;
    
    helper.countOfTextures = seq.count - 1;
    helper.iterationsTillSwitch = pauseTillSwitch;

    // ОЧЕНЬ ВАЖНО: передаем количество текстур, а не последний номер
    return initTextures(textures, path, seq.count, seq.maxDigits, seq.startCounter);
}
bool GameData::loadTexture(std::map<std::string,sf::Texture> &textures, std::string path, bool clearOnReuse = true)
{
    TextureSequenceInfo seq = analyzeTextureSequence(path);
    
    if (seq.count == 0) {
        std::cerr << "ERROR: No textures found for path: " << path << std::endl;
        return false;
    }
    
    // Отладочный вывод
    std::cout << "Loading textures: " << path 
              << ", start=" << seq.startCounter 
              << ", count=" << seq.count 
              << ", digits=" << seq.maxDigits << std::endl;
    
    // ОЧЕНЬ ВАЖНО: передаем количество текстур, а не последний номер
    if(clearOnReuse) return initTextures(textures, path, seq.count, seq.maxDigits, seq.startCounter);
    else return initTextures(textures, path, seq.count, seq.maxDigits, seq.startCounter, false);
}

void GameData::generateMipmapTextures(std::map<std::string, sf::Texture> &texturesArray)
{
    for (auto &i : texturesArray)
    {
        if(i.second.generateMipmap())
        {
            std::cout << "Mipmap generated\n"; 
        }
        else
        {
            std::cout << "Error while generating mipmap\n";
        }
    }
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

void GameData::smoothTextures(std::map<std::string, sf::Texture> &texturesArray)
{
    for (auto &i : texturesArray)
    {
        i.second.setSmooth(true);
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
