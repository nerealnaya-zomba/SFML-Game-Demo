#pragma once
#include<SFML/Graphics.hpp>
#include<vector>
#include<string>
#include<iostream>
#include<Mounting.h>
#include<sfml-headers.h>

class Player{
    public:
    
    //Variables
        //Logic
    bool isIdle = true;
    bool isFalling = true;
    float fallingSpeed = 0.f;
    float walkSpeed = 0.f;
    float maxWalkSpeed = 4.f;
    
        //Textures
    std::vector<sf::Texture> idleTextures{};    
    std::vector<std::string> idleTexturesPaths{
        "images/satiro-idle-1.png",
        "images/satiro-idle-2.png",
        "images/satiro-idle-3.png",
        "images/satiro-idle-4.png",
        "images/satiro-idle-5.png",
        "images/satiro-idle-6.png",
    };
    std::vector<sf::Texture> runningTextures{};
    std::vector<std::string>  runningTexturesPaths{
        "images/satiro-running-1.png",
        "images/satiro-running-2.png",
        "images/satiro-running-3.png",
        "images/satiro-running-4.png",
        "images/satiro-running-5.png",
        "images/satiro-running-6.png",
        "images/satiro-running-7.png",
        "images/satiro-running-8.png",
    };
    std::vector<sf::Texture> fallingTextures{};
    std::vector<std::string> fallingTexturesPaths{
        "images/satiro-falling-1.png",
        "images/satiro-falling-2.png",
        "images/satiro-falling-3.png",
        "images/satiro-falling-4.png",
        "images/satiro-falling-5.png",
    };


        //Rectangles
    sf::RectangleShape* playerRectangle;
        //Sprite
    sf::Sprite* playerSprite;

    //Control methods
    void updateControls();
    
    //Physics methods
    void updatePhysics();

    void applyFriction(float& walkSpeed, float friction);
    void walkLeft(float speed);
    void walkRight(float speed);

    //Texture methods
    void initTextures(std::vector<sf::Texture>& textures, std::vector<std::string> paths);
    void switchToNextIdleSprite();
    void switchToNextRunningSprite();
    void switchToNextFallingTexture();
    void updateTextures();
    Player();
    ~Player();
    void drawPlayer(sf::RenderWindow& window);
    
};