#pragma once
#include <SFML/Graphics.hpp>
#include<string>
#include<iostream>
#include<filesystem>
#include<vector>
#include<TexturesIterHelper.h>
#include <fstream> 
#include <sstream>
#include <iomanip>
#include <map>

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
static bool initTextures(std::vector<sf::Texture> &textures, std::string path, 
                         int texturesCount, int maxDigits = 5, int startCounter = 0)
{
    // Очищаем вектор на случай повторного использования
    textures.clear();
    
    std::string basePath = path;
    
    for (int counter = startCounter; counter < startCounter + texturesCount; counter++) {
        std::ostringstream filename;
        filename << basePath 
                 << std::setw(maxDigits) << std::setfill('0') << counter 
                 << ".png";

        sf::Texture texture;
        if (!texture.loadFromFile(filename.str())) {
            std::cerr << "Failed to load image: " << filename.str() << std::endl;
            return false;
        }
        textures.push_back(texture);
    }
    
    return true;
}

static bool initTextures(std::map<std::string,sf::Texture> &textures, std::string path, 
                         int texturesCount, int maxDigits = 5, int startCounter = 0, bool clearOnReuse = true)
{
    // Очищаем вектор на случай повторного использования
    if(clearOnReuse) textures.clear();
    
    std::string basePath = path;
    
    for (int counter = startCounter; counter < startCounter + texturesCount; counter++) {
        std::ostringstream filename;
        filename << basePath 
                 << std::setw(maxDigits) << std::setfill('0') << counter 
                 << ".png";

        sf::Texture texture;
        if (!texture.loadFromFile(filename.str())) {
            std::cerr << "Failed to load image: " << filename.str() << std::endl;
            return false;
        }
        std::string only_filename = filename.str().substr(filename.str().find_last_of("/\\") + 1);
        textures.emplace(only_filename,texture);
    }
    
    return true;
}

static int countTexturesInFolder(std::string path, int maxDigits = 5, int startCounter = 0)
{
    int count = 0;
    int counter = startCounter;
    
    while (true) {
        // Форматируем имя файла с ведущими нулями
        std::ostringstream filename;
        filename << path 
                << std::setw(maxDigits) << std::setfill('0') << counter 
                << ".png";

        // Проверяем существует ли файл
        std::ifstream file(filename.str()); 
        if (!file.good()) {
            break; // Файл не найден - заканчиваем подсчет
        }
        
        count++;
        counter++;
    }
    
    return count;
}

static void analyzeTextureSequence(std::string path, int& maxDigits, int& startCounter, int& count) 
{
    maxDigits = 0;
    startCounter = 0;
    count = 0;
    
    // Определяем максимальные digits - ищем паттерн в пути
    // Пример: ".../skeleton-idle_00000.png" -> digits=5
    std::size_t lastUnderscore = path.find_last_of('_');
    if (lastUnderscore != std::string::npos) {
        std::string suffix = path.substr(lastUnderscore + 1);
        maxDigits = suffix.length();
    }
    
    // Пробуем найти startCounter
    int counter = 0;
    int foundStart = -1;
    
    // Сканируем от 0 до 99999
    for (int i = 0; i < 100000; i++) {
        std::ostringstream filename;
        filename << path 
                << std::setw(maxDigits) << std::setfill('0') << i 
                << ".png";

        std::ifstream file(filename.str());
        if (file.good()) {
            if (foundStart == -1) {
                foundStart = i; // Первый найденный файл
            }
            count++;
        } 
        else if (foundStart != -1) {
            break; // Нашли начало, но файл отсутствует - закончили
        }
    }
    
    if (foundStart != -1) {
        startCounter = foundStart;
    }
}

// Или одна функция которая всё возвращает в структуре:
struct TextureSequenceInfo {
    int maxDigits;
    int startCounter;
    int count;
};

static TextureSequenceInfo analyzeTextureSequence(std::string path)
{
    TextureSequenceInfo info{5, 0, 0}; // Значения по умолчанию
    
    // Убираем .png если есть
    if (path.size() > 4 && path.substr(path.size() - 4) == ".png") {
        path = path.substr(0, path.size() - 4);
    }
    
    // ПРОСТОЙ ПОДХОД: пробуем разные форматы, пока не найдем файлы
    const std::vector<std::pair<int, int>> patterns = {
        {5, 0}, // 00000, 00001 (растения, скелетоны)
        {2, 1}, // 01, 02 (пули)
        {4, 0}, // 0000, 0001
        {3, 0}, // 000, 001
        {2, 0}, // 00, 01
        {1, 1}  // 1, 2, 3
    };
    
    for (const auto& pattern : patterns) {
        int digits = pattern.first;
        int start = pattern.second;
        int count = 0;
        
        // Убираем цифры из конца пути, если они есть
        std::string basePath = path;
        while (!basePath.empty() && std::isdigit(basePath.back())) {
            basePath.pop_back();
        }
        
        // Проверяем файлы
        bool foundAtLeastOne = false;
        for (int i = start; i < start + 1000; i++) {
            std::ostringstream filename;
            filename << basePath 
                     << std::setw(digits) << std::setfill('0') << i 
                     << ".png";
            
            std::ifstream file(filename.str());
            if (file.good()) {
                count++;
                foundAtLeastOne = true;
            } else {
                if (foundAtLeastOne) {
                    break; // Нашли последовательность файлов, которая прервалась
                } else {
                    break; // Не нашли первый файл с этим форматом
                }
            }
        }
        
        if (count > 0) {
            info.maxDigits = digits;
            info.startCounter = start;
            info.count = count;
            
            // Отладочный вывод
            std::cout << "Found pattern for " << basePath 
                      << ": digits=" << digits 
                      << ", start=" << start 
                      << ", count=" << count << std::endl;
            return info;
        }
    }
    
    std::cerr << "WARNING: No textures found for path: " << path << std::endl;
    return info; // Возвращаем значения по умолчанию
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
static void attachTexture(std::map<std::string,sf::Texture>& fromTexture, std::map<std::string,sf::Texture>*& toTexture) 
{
    toTexture = &fromTexture;
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
namespace gameUtils
{
static bool switchToNextSprite(sf::Sprite* Sprite,
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
    Sprite->setTexture(texturesArray.at(iterHelper.ptrToTexture));
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
}

//Other
    //Random
static int random(int min, int max) {
    return min + rand() % (max - min + 1);
}
static float random(float min, float max) {
    return min + static_cast<float>(rand()) / RAND_MAX * (max - min);
}
static bool checkInterval(sf::Clock& clock, float intervalMs) {
    if (clock.getElapsedTime().asMilliseconds() >= intervalMs) {
        clock.restart();
        return true;
    }
    return false;
}