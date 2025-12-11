#pragma once
#include<SFML/Graphics.hpp>
#include<map>
#include<iostream>
#include<Defines.h>
#include<Mounting.h>
#include<GameData.h>
#include<nlohmann/json.hpp>
#include<TexturesIterHelper.h>
#include<fstream>

// Только предварительные объявления вместо include
class Ground;
class Platform; 
class Player;

class Enemy
{
private:
    void switchTexture(std::vector<std::unique_ptr<sf::Sprite>>& spritesArray, std::vector<sf::Texture>& texturesArray, texturesIterHelper& iterHelper);
public:
    Enemy(GameData& gameData);
    virtual ~Enemy();
};