#pragma once
#include<SFML/Graphics.hpp>
#include<vector>
#include<iostream>
#include<string>
#include<Mounting.h>
#include<GameTextures.h>
#include<TexturesIterHelper.h>
#include<windows.h>

class Decoration
{
public:
    Decoration(GameTextures& gameTextures);
    ~Decoration();

    void addDecoration(std::string name,sf::Vector2f position, sf::Vector2f scale, sf::Color color=sf::Color::White);
    void updateTextures();
    void draw(sf::RenderWindow& window);

    //Textures and iters pointers
        //Plants
    std::vector<sf::Texture>* plant1Textures;
    texturesIterHelper* plant1;

    std::vector<sf::Texture>* plant2Textures;
    texturesIterHelper* plant2;

    std::vector<sf::Texture>* plant3Textures;
    texturesIterHelper* plant3;

    std::vector<sf::Texture>* plant4Textures;
    texturesIterHelper* plant4;

    std::vector<sf::Texture>* plant5Textures;
    texturesIterHelper* plant5;

    std::vector<sf::Texture>* plant6Textures;
    texturesIterHelper* plant6;

    std::vector<sf::Texture>* plant7Textures;
    texturesIterHelper* plant7;

    std::vector<sf::Texture>* jumpPlantTextures;
    texturesIterHelper* jumpPlant;
        //Cat
    std::vector<sf::Texture>* cat1Textures;
    texturesIterHelper* catHelper;
    private:
    //Sprites
        //Plants
    std::vector<std::unique_ptr<sf::Sprite>> plant1Sprites;
    
    std::vector<std::unique_ptr<sf::Sprite>> plant2Sprites;
    
    std::vector<std::unique_ptr<sf::Sprite>> plant3Sprites;
    
    std::vector<std::unique_ptr<sf::Sprite>> plant4Sprites;
    
    std::vector<std::unique_ptr<sf::Sprite>> plant5Sprites;
    
    std::vector<std::unique_ptr<sf::Sprite>> plant6Sprites;
    
    std::vector<std::unique_ptr<sf::Sprite>> plant7Sprites;

    std::vector<std::unique_ptr<sf::Sprite>> jumpPlantSprites;
        //Cat
    std::vector<std::unique_ptr<sf::Sprite>> cat1Sprites;
    

    //Switches sprite's texture to next
    void switchToNextSprite(std::vector<std::unique_ptr<sf::Sprite>>& spritesArray, std::vector<sf::Texture>& texturesArray, texturesIterHelper& iterHelper);

    //Optional
    void generateMipmapTextures(std::vector<sf::Texture>& texturesArray);
    void smoothTextures(std::vector<sf::Texture>& texturesArray);

};      

