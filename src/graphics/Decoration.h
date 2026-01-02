#pragma once
#include<SFML/Graphics.hpp>
#include<vector>
#include<iostream>
#include<string>
#include<Mounting.h>
#include<GameData.h>
#include<TexturesIterHelper.h>
#include<windows.h>
#include<unordered_map>
#include<bitset>
#include<GameCamera.h>

class GameCamera;
////////////////////////////////////////////////////////////////////////////////
// Кастомные кеш-функторы.
// Нужны для работы unordered_multimap
////////////////////////////////////////////////////////////////////////////////
struct Vector2fHash{
    std::size_t operator()(const sf::Vector2f& v) const{
        std::size_t h1 = std::hash<float>{}(v.x);
        std::size_t h2 = std::hash<float>{}(v.y);
        return h1 ^ (h2 << 1);
    }
};
struct Vector2fEqual {
    bool operator()(const sf::Vector2f& a, const sf::Vector2f& b) const {
        return a.x == b.x && a.y == b.y;
    }
};
struct Vector2fPairHash {
    std::size_t operator()(const std::pair<sf::Vector2f,sf::Vector2f>& pv) const {
        std::size_t h1 = std::hash<float>{}(pv.second.x);
        std::size_t h2 = std::hash<float>{}(pv.second.y);
        std::size_t h3 = std::hash<float>{}(pv.first.x);
        std::size_t h4 = std::hash<float>{}(pv.first.y);
        return ((h1 ^ (h2 << 1)) ^ (h3 << 2)) ^ (h4 << 3);
    }
};
struct Vector2fPairEqual {
    bool operator()(const std::pair<sf::Vector2f,sf::Vector2f>& pva, const std::pair<sf::Vector2f,sf::Vector2f>& pvb) const {
        return  (pva.second.x == pvb.second.x && pva.second.y == pvb.second.y) && 
                (pva.first.x  == pvb.first.x  && pva.first.y  == pvb.first.y );
    }
};

class Decoration 
{
public: 
    // Запрещаем копирование
    Decoration(const Decoration&) = delete;
    Decoration& operator=(const Decoration&) = delete;

    // Разрешаем перемещение
    Decoration(Decoration&&) = default;
    Decoration& operator=(Decoration&&) = default;

    Decoration(GameData& gameTextures, GameCamera& c);
    ~Decoration();

    void addDecoration(std::string name,sf::Vector2f position, sf::Vector2f scale, sf::Vector2f parallaxFactor, sf::Color color  = sf::Color::White);
    void updateTextures();
    void draw(sf::RenderWindow& window);

    //Textures and iters pointers
        //Plants
    std::vector<sf::Texture>* plant1Textures;
    texturesIterHelper plant1;

    std::vector<sf::Texture>* plant2Textures;
    texturesIterHelper plant2;

    std::vector<sf::Texture>* plant3Textures;
    texturesIterHelper plant3;

    std::vector<sf::Texture>* plant4Textures;
    texturesIterHelper plant4;

    std::vector<sf::Texture>* plant5Textures;
    texturesIterHelper plant5;

    std::vector<sf::Texture>* plant6Textures;
    texturesIterHelper plant6;

    std::vector<sf::Texture>* plant7Textures;
    texturesIterHelper plant7;
    
    std::vector<sf::Texture>* jumpPlantTextures;
    texturesIterHelper jumpPlant;
        //Cat
    std::vector<sf::Texture>* cat1Textures;
    texturesIterHelper catHelper;
        //Portal
    std::vector<sf::Texture>* portalGreenTextures;
    texturesIterHelper portalGreen;
    
    //Static-textures
    std::map<std::string,sf::Texture>* staticTextures;
        
    private:
    // Pointers to internal data
    GameData* data;
    GameCamera* camera;
    //Sprites
        //Plants
    std::unordered_multimap<std::pair<sf::Vector2f,sf::Vector2f>,std::unique_ptr<sf::Sprite>,Vector2fPairHash,Vector2fPairEqual> plant1Sprites;
    
    std::unordered_multimap<std::pair<sf::Vector2f,sf::Vector2f>,std::unique_ptr<sf::Sprite>,Vector2fPairHash,Vector2fPairEqual> plant2Sprites;
    
    std::unordered_multimap<std::pair<sf::Vector2f,sf::Vector2f>,std::unique_ptr<sf::Sprite>,Vector2fPairHash,Vector2fPairEqual> plant3Sprites;
    
    std::unordered_multimap<std::pair<sf::Vector2f,sf::Vector2f>,std::unique_ptr<sf::Sprite>,Vector2fPairHash,Vector2fPairEqual> plant4Sprites;
    
    std::unordered_multimap<std::pair<sf::Vector2f,sf::Vector2f>,std::unique_ptr<sf::Sprite>,Vector2fPairHash,Vector2fPairEqual> plant5Sprites;
    
    std::unordered_multimap<std::pair<sf::Vector2f,sf::Vector2f>,std::unique_ptr<sf::Sprite>,Vector2fPairHash,Vector2fPairEqual> plant6Sprites;
    
    std::unordered_multimap<std::pair<sf::Vector2f,sf::Vector2f>,std::unique_ptr<sf::Sprite>,Vector2fPairHash,Vector2fPairEqual> plant7Sprites;

    std::unordered_multimap<std::pair<sf::Vector2f,sf::Vector2f>,std::unique_ptr<sf::Sprite>,Vector2fPairHash,Vector2fPairEqual> jumpPlantSprites;
        //Cat
    std::unordered_multimap<std::pair<sf::Vector2f,sf::Vector2f>,std::unique_ptr<sf::Sprite>,Vector2fPairHash,Vector2fPairEqual> cat1Sprites;
        //Portal
    std::unordered_multimap<std::pair<sf::Vector2f,sf::Vector2f>,std::unique_ptr<sf::Sprite>,Vector2fPairHash,Vector2fPairEqual> portalGreenSprites;
    
    //Static-sprites
    std::unordered_multimap<std::pair<sf::Vector2f,sf::Vector2f>,std::unique_ptr<sf::Sprite>,Vector2fPairHash,Vector2fPairEqual> staticSprites;

    //Switches sprite's texture to next
    void switchToNextSprite(std::vector<std::unique_ptr<sf::Sprite>>& spritesArray, std::vector<sf::Texture>& texturesArray, texturesIterHelper& iterHelper);
    void switchToNextSprite(
        std::unordered_multimap<std::pair<sf::Vector2f,sf::Vector2f>,std::unique_ptr<sf::Sprite>,Vector2fPairHash,Vector2fPairEqual>& spritesArray, 
        std::vector<sf::Texture>& texturesArray, 
        texturesIterHelper& iterHelper
    );

    void updateParallax();
    void applyParalaxes(
    std::pair<
        const std::pair<sf::Vector2f, sf::Vector2f>,  
        std::unique_ptr<sf::Sprite>
    >& element 
    );

    //Optional
    void generateMipmapTextures(std::vector<sf::Texture>& texturesArray);
    void smoothTextures(std::vector<sf::Texture>& texturesArray);

};      