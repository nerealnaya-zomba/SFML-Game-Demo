#include "Decoration.h"

Decoration::Decoration()
{
    //Plants
    initTextures(plant1Textures,plant1Path, 89);
    generateMipmapTextures(plant1Textures);
    smoothTextures(plant1Textures);

    initTextures(plant2Textures,plant2Path, 89);
    generateMipmapTextures(plant2Textures);
    smoothTextures(plant2Textures);

    initTextures(plant3Textures,plant3Path, 89);
    generateMipmapTextures(plant3Textures);
    smoothTextures(plant3Textures);

    this->plant1to3.countOfTextures = 89;
    this->plant1to3.iterationsTillSwitch = 8;

    initTextures(plant4Textures,plant4Path, 59);
    generateMipmapTextures(plant4Textures);
    smoothTextures(plant4Textures);

    initTextures(plant5Textures,plant5Path, 59);
    generateMipmapTextures(plant5Textures);
    smoothTextures(plant5Textures);

    initTextures(plant6Textures,plant6Path, 59);
    generateMipmapTextures(plant6Textures);
    smoothTextures(plant6Textures);

    initTextures(plant7Textures,plant7Path, 59);
    generateMipmapTextures(plant7Textures);
    smoothTextures(plant7Textures);

    this->plant4to7.countOfTextures = 59;
    this->plant4to7.iterationsTillSwitch = 9;

    //Cat
    initTextures(cat1Textures,cat1Path, 2, 2);
    generateMipmapTextures(cat1Textures);
    smoothTextures(cat1Textures);

    this->catHelper.countOfTextures = 2;
    this->catHelper.iterationsTillSwitch = 72;
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

}

void Decoration::switchToNextSprite(std::vector<std::unique_ptr<sf::Sprite>>& spritesArray, std::vector<sf::Texture>& texturesArray, texturesIterHelper& iterHelper)
{
    for (auto &i : spritesArray)
    {
        i->setTexture(texturesArray.at(iterHelper.ptrToTexture));
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
    //Plant1
    if(plant1to3.iterationCounter<plant1to3.iterationsTillSwitch)
    {
        plant1to3.iterationCounter++;
    }
    else
    {
        //Forward-backward logic
        if(plant1to3.ptrToTexture == plant1to3.countOfTextures)
        {
            plant1to3.goForward = false;
        }
        else if(plant1to3.ptrToTexture == 0)
        {
            plant1to3.goForward = true;
        }

        //Switch sprites
        switchToNextSprite(plant1Sprites,plant1Textures,plant1to3);
        switchToNextSprite(plant2Sprites,plant2Textures,plant1to3);
        switchToNextSprite(plant3Sprites,plant3Textures,plant1to3);

        //Forward-backward logic
        if(plant1to3.goForward)
        {
            plant1to3.ptrToTexture++;
        }
        else
        {
            plant1to3.ptrToTexture--;
        }

        //reset iteration counter after all switches
        plant1to3.iterationCounter = 0;
    }

    //Tall plants
    if(plant4to7.iterationCounter<plant4to7.iterationsTillSwitch)
    {
        plant4to7.iterationCounter++;
    }
    else
    {
        //Forward-backward logic
        if(plant4to7.ptrToTexture == plant4to7.countOfTextures)
        {
            plant4to7.goForward = false;
        }
        else if(plant4to7.ptrToTexture == 0)
        {
            plant4to7.goForward = true;
        }

        //Switch sprites
        switchToNextSprite(plant4Sprites,plant4Textures,plant4to7);
        switchToNextSprite(plant5Sprites,plant5Textures,plant4to7);
        switchToNextSprite(plant6Sprites,plant6Textures,plant4to7);
        switchToNextSprite(plant7Sprites,plant7Textures,plant4to7);

        //Forward-backward logic
        if(plant4to7.goForward)
        {
            plant4to7.ptrToTexture++;
        }
        else
        {
            plant4to7.ptrToTexture--;
        }

        //reset iteration counter after all switches
        plant4to7.iterationCounter = 0;
    }

    if(catHelper.iterationCounter<catHelper.iterationsTillSwitch)
    {
        catHelper.iterationCounter++;
    }
    else
    {
        //Forward-backward logic
        if(catHelper.ptrToTexture == catHelper.countOfTextures)
        {
            catHelper.goForward = false;
        }
        else if(catHelper.ptrToTexture == 0)
        {
            catHelper.goForward = true;
        }

        //Switch sprites
        switchToNextSprite(cat1Sprites,cat1Textures,catHelper);

        //Forward-backward logic
        if(catHelper.goForward)
        {
            catHelper.ptrToTexture++;
        }
        else
        {
            catHelper.ptrToTexture--;
        }

        //reset iteration counter after all switches
        catHelper.iterationCounter = 0;
    }
    
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
    
}
