#include "Decoration.h"

Decoration::Decoration()
{
    //Plants
    initTextures(plant1Textures,plant1Path, 89);
    generateMipmapTextures(plant1Textures);
    smoothTextures(plant1Textures);
    this->plant1.countOfTextures = 89;
    this->plant1.iterationsTillSwitch = 8;

    initTextures(plant2Textures,plant2Path, 89);
    generateMipmapTextures(plant2Textures);
    smoothTextures(plant2Textures);
    this->plant2.countOfTextures = 89;
    this->plant2.iterationsTillSwitch = 8;

    initTextures(plant3Textures,plant3Path, 89);
    generateMipmapTextures(plant3Textures);
    smoothTextures(plant3Textures);
    this->plant3.countOfTextures = 89;
    this->plant3.iterationsTillSwitch = 8;

    initTextures(plant4Textures,plant4Path, 59);
    generateMipmapTextures(plant4Textures);
    smoothTextures(plant4Textures);
    this->plant4.countOfTextures = 59;
    this->plant4.iterationsTillSwitch = 6;

    initTextures(plant5Textures,plant5Path, 59);
    generateMipmapTextures(plant5Textures);
    smoothTextures(plant5Textures);
    this->plant5.countOfTextures = 59;
    this->plant5.iterationsTillSwitch = 9;

    initTextures(plant6Textures,plant6Path, 59);
    generateMipmapTextures(plant6Textures);
    smoothTextures(plant6Textures);
    this->plant6.countOfTextures = 59;
    this->plant6.iterationsTillSwitch = 9;

    initTextures(plant7Textures,plant7Path, 59);
    generateMipmapTextures(plant7Textures);
    smoothTextures(plant7Textures);
    this->plant7.countOfTextures = 59;
    this->plant7.iterationsTillSwitch = 9;

    //Cat
    initTextures(cat1Textures,cat1Path, 2, 2);
    generateMipmapTextures(cat1Textures);
    smoothTextures(cat1Textures);
    this->catHelper.countOfTextures = 2;
    this->catHelper.iterationsTillSwitch = 72;

    //jumpPlant
    initTextures(jumpPlantTextures,jumpPlantPath,19);
    generateMipmapTextures(jumpPlantTextures);
    smoothTextures(jumpPlantTextures);
    this->jumpPlant.countOfTextures = 19;
    this->jumpPlant.iterationsTillSwitch = 5;
}

Decoration::~Decoration()
{
}

void Decoration::addDecoration(std::string name,sf::Vector2f position, sf::Vector2f scale, sf::Color color)
{
    if(name == "plant1")
    {
        auto sprite = std::make_unique<sf::Sprite>(plant1Textures.at(0));
        sprite->setOrigin(sprite->getGlobalBounds().getCenter());
        sprite->setPosition(position);
        sprite->setScale(scale);
        sprite->setColor(color);
        plant1Sprites.push_back(std::move(sprite));
    }
    else if(name == "plant2")
    {
        auto sprite = std::make_unique<sf::Sprite>(plant2Textures.at(0));
        sprite->setOrigin(sprite->getGlobalBounds().getCenter());
        sprite->setPosition(position);
        sprite->setScale(scale);
        sprite->setColor(color);
        plant2Sprites.push_back(std::move(sprite));
    }
    else if(name == "plant3")
    {
        auto sprite = std::make_unique<sf::Sprite>(plant3Textures.at(0));
        sprite->setOrigin(sprite->getGlobalBounds().getCenter());
        sprite->setPosition(position);
        sprite->setScale(scale);
        sprite->setColor(color);
        plant3Sprites.push_back(std::move(sprite));
    }
    else if(name == "plant4")
    {
        auto sprite = std::make_unique<sf::Sprite>(plant4Textures.at(0));
        sprite->setOrigin(sprite->getGlobalBounds().getCenter());
        sprite->setPosition(position);
        sprite->setScale(scale);
        sprite->setColor(color);
        plant4Sprites.push_back(std::move(sprite));
    }
    else if(name == "plant5")
    {
        auto sprite = std::make_unique<sf::Sprite>(plant5Textures.at(0));
        sprite->setOrigin(sprite->getGlobalBounds().getCenter());
        sprite->setPosition(position);
        sprite->setScale(scale);
        sprite->setColor(color);
        plant5Sprites.push_back(std::move(sprite));
    }
    else if(name == "plant6")
    {
        auto sprite = std::make_unique<sf::Sprite>(plant6Textures.at(0));
        sprite->setOrigin(sprite->getGlobalBounds().getCenter());
        sprite->setPosition(position);
        sprite->setScale(scale);
        sprite->setColor(color);
        plant6Sprites.push_back(std::move(sprite));
    }
    else if(name == "plant7")
    {
        auto sprite = std::make_unique<sf::Sprite>(plant7Textures.at(0));
        sprite->setOrigin(sprite->getGlobalBounds().getCenter());
        sprite->setPosition(position);
        sprite->setScale(scale);
        sprite->setColor(color);
        plant7Sprites.push_back(std::move(sprite));
    }
    else if(name == "cat")
    {
        auto sprite = std::make_unique<sf::Sprite>(cat1Textures.at(0));
        sprite->setOrigin(sprite->getGlobalBounds().getCenter());
        sprite->setPosition(position);
        sprite->setScale(scale);
        sprite->setColor(color);
        cat1Sprites.push_back(std::move(sprite));
    }
    else if(name == "jumpPlant")
    {
        auto sprite = std::make_unique<sf::Sprite>(jumpPlantTextures.at(0));
        sprite->setOrigin(sprite->getGlobalBounds().getCenter());
        sprite->setPosition(position);
        sprite->setScale(scale);
        sprite->setColor(color);
        jumpPlantSprites.push_back(std::move(sprite));
    }

}

void Decoration::switchToNextSprite(std::vector<std::unique_ptr<sf::Sprite>>& spritesArray, std::vector<sf::Texture>& texturesArray, texturesIterHelper& iterHelper)
{
    
    if(iterHelper.iterationCounter<iterHelper.iterationsTillSwitch)
    {
        iterHelper.iterationCounter++;
    }
    else
    {
        //Forward-backward logic
        if(iterHelper.ptrToTexture == iterHelper.countOfTextures)
        {
            iterHelper.goForward = false;
        }
        else if(iterHelper.ptrToTexture == 0)
        {
            iterHelper.goForward = true;
        }

        //Switch sprites
        for (auto &i : spritesArray)
        {
            i->setTexture(texturesArray.at(iterHelper.ptrToTexture));
        }

        //Forward-backward logic
        if(iterHelper.goForward)
        {
            iterHelper.ptrToTexture++;
        }
        else
        {
            iterHelper.ptrToTexture--;
        }

        //reset iteration counter after all switches
        iterHelper.iterationCounter = 0;
    }
}



void Decoration::generateMipmapTextures(std::vector<sf::Texture> &texturesArray)
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

void Decoration::smoothTextures(std::vector<sf::Texture> &texturesArray)
{
    for (auto &i : texturesArray)
    {
        i.setSmooth(true);
    }
    
}

void Decoration::updateTextures()
{

    switchToNextSprite(plant1Sprites,plant1Textures,plant1);
    switchToNextSprite(plant2Sprites,plant2Textures,plant2);
    switchToNextSprite(plant3Sprites,plant3Textures,plant3);



    switchToNextSprite(plant4Sprites,plant4Textures,plant4);
    switchToNextSprite(plant5Sprites,plant5Textures,plant5);
    switchToNextSprite(plant6Sprites,plant6Textures,plant6);
    switchToNextSprite(plant7Sprites,plant7Textures,plant7);



    switchToNextSprite(cat1Sprites,cat1Textures,catHelper);

    switchToNextSprite(jumpPlantSprites,jumpPlantTextures,jumpPlant);

}

void Decoration::draw(sf::RenderWindow &window)
{
    for (auto &i : plant1Sprites)
    {
        window.draw(*i);
    }
    for (auto &i : plant2Sprites)
    {
        window.draw(*i);
    }
    for (auto &i : plant3Sprites)
    {
        window.draw(*i);
    }
    for (auto &i : plant4Sprites)
    {
        window.draw(*i);
    }
    for (auto &i : plant5Sprites)
    {
        window.draw(*i);
    }
    for (auto &i : plant6Sprites)
    {
        window.draw(*i);
    }
    for (auto &i : plant7Sprites)
    {
        window.draw(*i);
    }
    for (auto &i : cat1Sprites)
    {
        window.draw(*i);
    }
    for (auto &i : jumpPlantSprites)
    {
        window.draw(*i);
    }
    
}
