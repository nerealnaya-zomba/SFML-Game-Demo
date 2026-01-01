#include "Decoration.h"

Decoration::Decoration(GameData& gameTextures)
{
    //Plants
    attachTexture(gameTextures.plant1Textures, this->plant1Textures, gameTextures.plant1,this->plant1);
    attachTexture(gameTextures.plant2Textures, this->plant2Textures, gameTextures.plant2,this->plant2);
    attachTexture(gameTextures.plant3Textures, this->plant3Textures, gameTextures.plant3,this->plant3);
    attachTexture(gameTextures.plant4Textures, this->plant4Textures, gameTextures.plant4,this->plant4);
    attachTexture(gameTextures.plant5Textures, this->plant5Textures, gameTextures.plant5,this->plant5);
    attachTexture(gameTextures.plant6Textures, this->plant6Textures, gameTextures.plant6,this->plant6);
    attachTexture(gameTextures.plant7Textures, this->plant7Textures, gameTextures.plant7,this->plant7);
    //Cat
    attachTexture(gameTextures.jumpPlantTextures, this->jumpPlantTextures, gameTextures.jumpPlant, this->jumpPlant);

    //jumpPlant
    attachTexture(gameTextures.cat1Textures, this->cat1Textures, gameTextures.catHelper, this->catHelper);

    //portalGreen
    attachTexture(gameTextures.portalGreenTextures, this->portalGreenTextures,gameTextures.portalGreen, this->portalGreen);

    //Static-textures
    attachTexture(gameTextures.allStaticTextures,this->staticTextures);
}

Decoration::~Decoration()
{
}

void Decoration::addDecoration(std::string name,sf::Vector2f position, sf::Vector2f scale, sf::Color color)
{
    if(name == "plant1")
    {
        //Error handling
        if(plant1Textures == nullptr || plant1Textures->empty())
        {
            MessageBox(NULL,"Texture not loaded or empty!", "Error", MB_ICONERROR);
            exit(EXIT_FAILURE);
        } 
        auto sprite = std::make_unique<sf::Sprite>(plant1Textures->at(0));
        sprite->setOrigin(sprite->getGlobalBounds().getCenter());
        sprite->setPosition(position);
        sprite->setScale(scale);
        sprite->setColor(color);
        plant1Sprites.push_back(std::move(sprite));
    }
    else if(name == "plant2")
    {
        //Error handling
        if(plant2Textures == nullptr || plant2Textures->empty())
        {
            MessageBox(NULL,"Texture not loaded or empty!", "Error", MB_ICONERROR);
            exit(EXIT_FAILURE);
        } 
        auto sprite = std::make_unique<sf::Sprite>(plant2Textures->at(0));
        sprite->setOrigin(sprite->getGlobalBounds().getCenter());
        sprite->setPosition(position);
        sprite->setScale(scale);
        sprite->setColor(color);
        plant2Sprites.push_back(std::move(sprite));
    }
    else if(name == "plant3")
    {
        //Error handling
        if(plant3Textures == nullptr || plant3Textures->empty())
        {
            MessageBox(NULL,"Texture not loaded or empty!", "Error", MB_ICONERROR);
            exit(EXIT_FAILURE);
        } 
        auto sprite = std::make_unique<sf::Sprite>(plant3Textures->at(0));
        sprite->setOrigin(sprite->getGlobalBounds().getCenter());
        sprite->setPosition(position);
        sprite->setScale(scale);
        sprite->setColor(color);
        plant3Sprites.push_back(std::move(sprite));
    }
    else if(name == "plant4")
    {
        //Error handling
        if(plant4Textures == nullptr || plant4Textures->empty())
        {
            MessageBox(NULL,"Texture not loaded or empty!", "Error", MB_ICONERROR);
            exit(EXIT_FAILURE);
        } 
        auto sprite = std::make_unique<sf::Sprite>(plant4Textures->at(0));
        sprite->setOrigin(sprite->getGlobalBounds().getCenter());
        sprite->setPosition(position);
        sprite->setScale(scale);
        sprite->setColor(color);
        plant4Sprites.push_back(std::move(sprite));
    }
    else if(name == "plant5")
    {
        //Error handling
        if(plant5Textures == nullptr || plant5Textures->empty())
        {
            MessageBox(NULL,"Texture not loaded or empty!", "Error", MB_ICONERROR);
            exit(EXIT_FAILURE);
        } 
        auto sprite = std::make_unique<sf::Sprite>(plant5Textures->at(0));
        sprite->setOrigin(sprite->getGlobalBounds().getCenter());
        sprite->setPosition(position);
        sprite->setScale(scale);
        sprite->setColor(color);
        plant5Sprites.push_back(std::move(sprite));
    }
    else if(name == "plant6")
    {
        //Error handling
        if(plant6Textures == nullptr || plant6Textures->empty())
        {
            MessageBox(NULL,"Texture not loaded or empty!", "Error", MB_ICONERROR);
            exit(EXIT_FAILURE);
        } 
        auto sprite = std::make_unique<sf::Sprite>(plant6Textures->at(0));
        sprite->setOrigin(sprite->getGlobalBounds().getCenter());
        sprite->setPosition(position);
        sprite->setScale(scale);
        sprite->setColor(color);
        plant6Sprites.push_back(std::move(sprite));
    }
    else if(name == "plant7")
    {
        //Error handling
        if(plant7Textures == nullptr || plant7Textures->empty())
        {
            MessageBox(NULL,"Texture not loaded or empty!", "Error", MB_ICONERROR);
            exit(EXIT_FAILURE);
        } 
        auto sprite = std::make_unique<sf::Sprite>(plant7Textures->at(0));
        sprite->setOrigin(sprite->getGlobalBounds().getCenter());
        sprite->setPosition(position);
        sprite->setScale(scale);
        sprite->setColor(color);
        plant7Sprites.push_back(std::move(sprite));
    }
    else if(name == "cat")
    {
        //Error handling
        if(cat1Textures == nullptr || cat1Textures->empty())
        {
            MessageBox(NULL,"Texture not loaded or empty!", "Error", MB_ICONERROR);
            exit(EXIT_FAILURE);
        } 
        auto sprite = std::make_unique<sf::Sprite>(cat1Textures->at(0));
        sprite->setOrigin(sprite->getGlobalBounds().getCenter());
        sprite->setPosition(position);
        sprite->setScale(scale);
        sprite->setColor(color);
        cat1Sprites.push_back(std::move(sprite));
    }
    else if(name == "jumpPlant")
    {
        //Error handling
        if(jumpPlantTextures == nullptr || jumpPlantTextures->empty())
        {
            MessageBox(NULL,"Texture not loaded or empty!", "Error", MB_ICONERROR);
            exit(EXIT_FAILURE);
        } 
        auto sprite = std::make_unique<sf::Sprite>(jumpPlantTextures->at(0));
        sprite->setOrigin(sprite->getGlobalBounds().getCenter());
        sprite->setPosition(position);
        sprite->setScale(scale);
        sprite->setColor(color);
        jumpPlantSprites.push_back(std::move(sprite));
    }
    else if(name == "portalGreen")
    {
        //Error handling
        if(portalGreenTextures == nullptr || portalGreenTextures->empty())
        {
            MessageBox(NULL,"Texture not loaded or empty!", "Error", MB_ICONERROR);
            exit(EXIT_FAILURE);
        } 
        auto sprite = std::make_unique<sf::Sprite>(portalGreenTextures->at(0));
        sprite->setOrigin(sprite->getGlobalBounds().getCenter());
        sprite->setPosition(position);
        sprite->setScale(scale);
        sprite->setColor(color);
        portalGreenSprites.push_back(std::move(sprite));
    }
    //Static-textures assertion
    else{
        try{
            auto sprite = std::make_unique<sf::Sprite>(staticTextures->at(name));
            sprite->setOrigin(sprite->getGlobalBounds().getCenter());
            sprite->setPosition(position);
            sprite->setScale(scale);
            sprite->setColor(color);
            staticSprites.push_back(std::move(sprite));
        }
        catch(std::out_of_range& ex){
            std::cout << ex.what() << std::endl;
            
            exit(1);
        }
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

    switchToNextSprite(plant1Sprites,*plant1Textures,plant1);
    switchToNextSprite(plant2Sprites,*plant2Textures,plant2);
    switchToNextSprite(plant3Sprites,*plant3Textures,plant3);



    switchToNextSprite(plant4Sprites,*plant4Textures,plant4);
    switchToNextSprite(plant5Sprites,*plant5Textures,plant5);
    switchToNextSprite(plant6Sprites,*plant6Textures,plant6);
    switchToNextSprite(plant7Sprites,*plant7Textures,plant7);



    switchToNextSprite(cat1Sprites,*cat1Textures,catHelper);

    switchToNextSprite(jumpPlantSprites,*jumpPlantTextures,jumpPlant);
    std::for_each(portalGreenSprites.begin(), portalGreenSprites.end(),
    [this](const std::unique_ptr<sf::Sprite>& spritePtr) {
        gameUtils::switchToNextSprite(
            spritePtr.get(),           // Получаем сырой указатель из unique_ptr
            *portalGreenTextures,      // Разыменовываем указатель на вектор
            portalGreen,               // Итератор-хелпер
            switchSprite_SwitchOption::Single
        );
    });
    
    

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
    for (auto &&i : portalGreenSprites)
    {
        window.draw(*i);
    }
    for (auto &&i : staticSprites)
    {
        window.draw(*i);
    }
    
    
}

