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

class Decoration 
{
public: 
    // Запрещаем копирование
    Decoration(const Decoration&) = delete;
    Decoration& operator=(const Decoration&) = delete;

    // Разрешаем перемещение
    Decoration(Decoration&&) = default;
    Decoration& operator=(Decoration&&) = default;

    Decoration(GameData& gameTextures);
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
    //Sprites
        //Plants
    std::unordered_multimap<sf::Vector2f,std::unique_ptr<sf::Sprite>,Vector2fHash,Vector2fEqual> plant1Sprites;
    
    std::unordered_multimap<sf::Vector2f,std::unique_ptr<sf::Sprite>,Vector2fHash,Vector2fEqual> plant2Sprites;
    
    std::unordered_multimap<sf::Vector2f,std::unique_ptr<sf::Sprite>,Vector2fHash,Vector2fEqual> plant3Sprites;
    
    std::unordered_multimap<sf::Vector2f,std::unique_ptr<sf::Sprite>,Vector2fHash,Vector2fEqual> plant4Sprites;
    
    std::unordered_multimap<sf::Vector2f,std::unique_ptr<sf::Sprite>,Vector2fHash,Vector2fEqual> plant5Sprites;
    
    std::unordered_multimap<sf::Vector2f,std::unique_ptr<sf::Sprite>,Vector2fHash,Vector2fEqual> plant6Sprites;
    
    std::unordered_multimap<sf::Vector2f,std::unique_ptr<sf::Sprite>,Vector2fHash,Vector2fEqual> plant7Sprites;

    std::unordered_multimap<sf::Vector2f,std::unique_ptr<sf::Sprite>,Vector2fHash,Vector2fEqual> jumpPlantSprites;
        //Cat
    std::unordered_multimap<sf::Vector2f,std::unique_ptr<sf::Sprite>,Vector2fHash,Vector2fEqual> cat1Sprites;
        //Portal
    std::unordered_multimap<sf::Vector2f,std::unique_ptr<sf::Sprite>,Vector2fHash,Vector2fEqual> portalGreenSprites;
    
    //Static-sprites
    std::unordered_multimap<sf::Vector2f,std::unique_ptr<sf::Sprite>,Vector2fHash,Vector2fEqual> staticSprites;

    //Switches sprite's texture to next
    void switchToNextSprite(std::vector<std::unique_ptr<sf::Sprite>>& spritesArray, std::vector<sf::Texture>& texturesArray, texturesIterHelper& iterHelper);
    void switchToNextSprite(
        std::unordered_multimap<sf::Vector2f,std::unique_ptr<sf::Sprite>,Vector2fHash,Vector2fEqual>& spritesArray, 
        std::vector<sf::Texture>& texturesArray, 
        texturesIterHelper& iterHelper
    );

    //Optional
    void generateMipmapTextures(std::vector<sf::Texture>& texturesArray);
    void smoothTextures(std::vector<sf::Texture>& texturesArray);

};      

