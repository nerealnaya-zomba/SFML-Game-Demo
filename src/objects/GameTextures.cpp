#include "GameTextures.h"

GameTextures::GameTextures()
{
    //Satiro Textures initialization
    if(initSatiroTextures(idleTextures,idleTexturesPaths)) succesedOperationsCount_m++;
    if(initSatiroTextures(runningTextures,runningTexturesPaths)) succesedOperationsCount_m++;
    if(initSatiroTextures(fallingTextures,fallingTexturesPaths)) succesedOperationsCount_m++;
    if(initSatiroTextures(bulletTextures,bulletTexturesPaths)) succesedOperationsCount_m++;

    //Plants
    if(initTextures(plant1Textures,plant1Path, 89)) succesedOperationsCount_m++;
    generateMipmapTextures(plant1Textures);
    smoothTextures(plant1Textures);
    this->plant1.countOfTextures = 89;
    this->plant1.iterationsTillSwitch = 8;

    if(initTextures(plant2Textures,plant2Path, 89)) succesedOperationsCount_m++;
    generateMipmapTextures(plant2Textures);
    smoothTextures(plant2Textures);
    this->plant2.countOfTextures = 89;
    this->plant2.iterationsTillSwitch = 8;

    if(initTextures(plant3Textures,plant3Path, 89)) succesedOperationsCount_m++;
    generateMipmapTextures(plant3Textures);
    smoothTextures(plant3Textures);
    this->plant3.countOfTextures = 89;
    this->plant3.iterationsTillSwitch = 8;

    if(initTextures(plant4Textures,plant4Path, 59)) succesedOperationsCount_m++;
    generateMipmapTextures(plant4Textures);
    smoothTextures(plant4Textures);
    this->plant4.countOfTextures = 59;
    this->plant4.iterationsTillSwitch = 6;

    if(initTextures(plant5Textures,plant5Path, 59)) succesedOperationsCount_m++;
    generateMipmapTextures(plant5Textures);
    smoothTextures(plant5Textures);
    this->plant5.countOfTextures = 59;
    this->plant5.iterationsTillSwitch = 9;

    if(initTextures(plant6Textures,plant6Path, 59)) succesedOperationsCount_m++;
    generateMipmapTextures(plant6Textures);
    smoothTextures(plant6Textures);
    this->plant6.countOfTextures = 59;
    this->plant6.iterationsTillSwitch = 9;

    if(initTextures(plant7Textures,plant7Path, 59)) succesedOperationsCount_m++;
    generateMipmapTextures(plant7Textures);
    smoothTextures(plant7Textures);
    this->plant7.countOfTextures = 59;
    this->plant7.iterationsTillSwitch = 9;

    //Cat
    if(initTextures(cat1Textures,cat1Path, 2, 2)) succesedOperationsCount_m++;
    generateMipmapTextures(cat1Textures);
    smoothTextures(cat1Textures);
    this->catHelper.countOfTextures = 2;
    this->catHelper.iterationsTillSwitch = 72;

    //jumpPlant
    if(initTextures(jumpPlantTextures,jumpPlantPath,19)) succesedOperationsCount_m++;
    generateMipmapTextures(jumpPlantTextures);
    smoothTextures(jumpPlantTextures);
    this->jumpPlant.countOfTextures = 19;
    this->jumpPlant.iterationsTillSwitch = 5;
}

GameTextures::~GameTextures()
{

}

bool GameTextures::initSatiroTextures(std::vector<sf::Texture> &textures, std::vector<std::string> paths)
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

void GameTextures::generateMipmapTextures(std::vector<sf::Texture> &texturesArray)
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

void GameTextures::smoothTextures(std::vector<sf::Texture> &texturesArray)
{
    for (auto &i : texturesArray)
    {
        i.setSmooth(true);
    }
}