#pragma once
#include<SFML/Graphics.hpp>
#include<map>
#include<iostream>
#include<Defines.h>
#include<Mounting.h>
#include<GameData.h>
#include<nlohmann/json.hpp>
#include<TexturesIterHelper.h>

class Enemy
{
private:
    //Variables
        //Logic
            //Player
    bool isIdle = true;
    bool isFalling = true;
    float fallingSpeed = 0.f;
    float initialWalkSpeed = 0.f;
    float speed = 0.15f; //TODO Remove unnecessary for this class variables
    float maxWalkSpeed = 4.f;
    float frictionForce = 0.1f;
    int playerPosX_m{};
    int playerPosY_m{};

    void switchTexture(std::vector<std::unique_ptr<sf::Sprite>>& spritesArray, std::vector<sf::Texture>& texturesArray, texturesIterHelper& iterHelper); //TODO should use iterHelper. Make texture arrays at first
public:
    Enemy(GameData& gameData);
    virtual ~Enemy();
};