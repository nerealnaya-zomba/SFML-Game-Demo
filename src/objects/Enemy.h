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

class Enemy
{
private:
    //TODO(may fill this up) Variables 


    void switchTexture(std::vector<std::unique_ptr<sf::Sprite>>& spritesArray, std::vector<sf::Texture>& texturesArray, texturesIterHelper& iterHelper);
public:
    Enemy(GameData& gameData);
    virtual ~Enemy();
};