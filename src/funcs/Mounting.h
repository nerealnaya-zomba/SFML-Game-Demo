#pragma once
#include <SFML/Graphics.hpp>
#include<string>
#include<iostream>
#include<filesystem>
#include<vector>
#include<TexturesIterHelper.h>

static void setRectangleOriginToMiddle(sf::RectangleShape& rect)
{
    rect.setOrigin({rect.getSize().x/2,rect.getSize().y/2});
}
static void setSpriteOriginToMiddle(sf::Sprite& sprite)
{
    sprite.setOrigin({sprite.getLocalBounds().size.x/2, sprite.getLocalBounds().size.y/2});
}
static void setTextOriginToMiddle(sf::Text& text)
{
    text.setOrigin({text.getLocalBounds().size.x/2, text.getLocalBounds().size.y/2});
}
//Loading
static bool initTextures(std::vector<sf::Texture> &textures, std::string path, int texturesCount, int maxDigits = 5, int counterA = 0)
{
    sf::Texture* texture = new sf::Texture;
    std::string basePath = path;
    int counter = counterA;
    
    while (counter<=texturesCount) {
        // Форматируем число с ведущими нулями (00000, 00001, ...)
        std::ostringstream filename;
        filename << basePath 
                << std::setw(maxDigits) << std::setfill('0') << counter 
                << ".png";

        // Пытаемся загрузить текстуру
        
        if (!texture->loadFromFile(filename.str())) 
        {
            return false;
        }
        textures.push_back(*texture);
        std::cout << "Loaded: " << filename.str() << std::endl;
        counter++;

    }
    return true;
}

namespace fs = std::filesystem;

static std::vector<std::string> find_files(const std::string& path, const std::string& mask) {
    std::vector<std::string> result;
    for (const auto& entry : fs::directory_iterator(path)) {
        if (entry.is_regular_file() && entry.path().filename().string().find(mask) != std::string::npos) {
            result.push_back(entry.path().filename().string());
        }
    }
    return result;
}

//Texture
    //Attaching
        // "*&" means that we use reference to pointer. If there would be only "*", then that would mean we get copy of that pointer, not changing its own value.
static void attachTexture(std::vector<sf::Texture>& fromTexture, std::vector<sf::Texture>*& toTexture) 
{
    toTexture = &fromTexture;
}
static void attachTexture(std::vector<sf::Texture>& fromTexture, std::vector<sf::Texture>*& toTexture, texturesIterHelper& fromIter, texturesIterHelper& toIter)
{
    toTexture = &fromTexture;
    toIter = fromIter;
}
static void attachTexture(sf::Texture& fromTexture, sf::Texture*& toTexture) 
{
    toTexture = &fromTexture;
}
    //Using
enum switchSprite_SwitchOption
{
    Single,
    Loop
};
static bool switchToNextSprite(sf::Sprite* enemySprite,
        std::vector<sf::Texture>& texturesArray, 
        texturesIterHelper& iterHelper, 
        switchSprite_SwitchOption option)
{
    if(iterHelper.iterationCounter < iterHelper.iterationsTillSwitch)
    {
        iterHelper.iterationCounter++;
        return true; // анимация еще идет
    }

    // Переключаем текстуру
    enemySprite->setTexture(texturesArray.at(iterHelper.ptrToTexture));
    iterHelper.ptrToTexture++;
    iterHelper.iterationCounter = 0;

    // Достигли конца анимации
    if(iterHelper.ptrToTexture >= iterHelper.countOfTextures)
    {
        iterHelper.ptrToTexture = 0;
        
        if(option == switchSprite_SwitchOption::Single)
            return false; // Single анимация ЗАВЕРШЕНА
        // Loop продолжается автоматически
    }

    return true; // анимация еще идет
}
//Other
static int random(int min, int max) {
    return rand() % (max - min + 1) + min;
}
