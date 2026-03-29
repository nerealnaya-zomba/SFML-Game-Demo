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
#include<limits>
#include<cmath>

#ifndef GAME_VERBOSE_LOGS
#define GAME_VERBOSE_LOGS 0
#endif

template <typename... Args>
static void debugLog(Args&&... args)
{
#if GAME_VERBOSE_LOGS
    (std::cout << ... << args) << std::endl;
#else
    (void)sizeof...(args);
#endif
}

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
    textures.clear();
    textures.reserve(static_cast<size_t>(texturesCount));

    for (int counter = startCounter; counter < startCounter + texturesCount; counter++) {
        std::ostringstream filename;
        filename << path
                 << std::setw(maxDigits) << std::setfill('0') << counter 
                 << ".png";

        textures.emplace_back();
        if (!textures.back().loadFromFile(filename.str())) {
            textures.pop_back();
            std::cerr << "Failed to load image: " << filename.str() << std::endl;
            return false;
        }
    }
    
    return true;
}

static bool initTextures(std::map<std::string,sf::Texture> &textures, std::string path, 
                         int texturesCount, int maxDigits = 5, int startCounter = 0, bool clearOnReuse = true)
{
    if(clearOnReuse) textures.clear();

    for (int counter = startCounter; counter < startCounter + texturesCount; counter++) {
        std::ostringstream filename;
        filename << path
                 << std::setw(maxDigits) << std::setfill('0') << counter 
                 << ".png";

        std::string only_filename = filename.str().substr(filename.str().find_last_of("/\\") + 1);
        auto [it, inserted] = textures.try_emplace(only_filename);
        if (!it->second.loadFromFile(filename.str())) {
            if (inserted) {
                textures.erase(it);
            }
            std::cerr << "Failed to load image: " << filename.str() << std::endl;
            exit(1);
            return false;
        }
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
            debugLog("Found pattern for ", basePath,
                     ": digits=", digits,
                     ", start=", start,
                     ", count=", count);
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
static bool switchToNextSprite(sf::Sprite* sprite,
    std::vector<sf::Texture>& texturesArray, 
    texturesIterHelper& iterHelper, 
    switchSprite_SwitchOption option)
{
    // Проверка безопасности
    if (!sprite || texturesArray.empty()) {
        return false;
    }

    // Увеличиваем счетчик кадров
    iterHelper.iterationCounter++;
    
    // Проверяем, не пора ли переключить текстуру
    if (iterHelper.iterationCounter < iterHelper.iterationsTillSwitch) {
        return true; // ждем следующий кадр
    }
    
    // Сбрасываем счетчик кадров
    iterHelper.iterationCounter = 0;
    
    // Переключаем текстуру
    if (iterHelper.ptrToTexture < texturesArray.size()) {
        sprite->setTexture(texturesArray[iterHelper.ptrToTexture]);
    }
    
    // Увеличиваем указатель на текстуру
    iterHelper.ptrToTexture++;
    
    // Проверяем, достигли ли конца анимации
    if (iterHelper.ptrToTexture >= texturesArray.size()) {
        iterHelper.ptrToTexture = 0;
        
        if (option == switchSprite_SwitchOption::Single) {
            return false; // Single анимация завершена
        }
        // Для Loop продолжаем с начала
    }
    
    return true; // анимация продолжается
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
    // Проверяет прошло ли intervalMs времени, и возвращает true на успех и false иначе.
    // Перезапускает таймер если вернуло true
static bool checkInterval(sf::Clock& clock, float intervalMs) {
    if (clock.getElapsedTime().asMilliseconds() >= intervalMs) {
        clock.restart();
        return true;
    }
    return false;
}
static bool checkInterval(sf::Clock& clock, int intervalMs) {
    if (clock.getElapsedTime().asMilliseconds() >= intervalMs) {
        clock.restart();
        return true;
    }
    return false;
}
static bool checkInterval(sf::Clock& clock, unsigned int intervalMs) {
    if (clock.getElapsedTime().asMilliseconds() >= intervalMs) {
        clock.restart();
        return true;
    }
    return false;
}
static void substractUnsigned(unsigned int &first, unsigned int second)
{
    if (second>first) {
        first = 0;
    }else
        first -= second;
}
static void substractUnsigned(uint8_t &first, int second)
{
    if (second>first) {
        first = 0;
    }else first = static_cast<uint8_t>(first - second);
}
static bool isEqualFloat(float a, float b, float epsilon = 0.0001f)
{
    return std::fabs(a - b) < epsilon;
}

static bool isEqualDouble(double a, double b, double epsilon = 0.000001)
{
    return std::fabs(a - b) < epsilon;
}
