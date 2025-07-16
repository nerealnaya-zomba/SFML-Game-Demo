#pragma once
#include<SFML/Graphics.hpp>
#include<vector>
#include<string>
#include<iostream>
#include<Mounting.h>
#include<sfml-headers.h>
#include<Trail.h>
#include<Bullet.h>
#include<list>
#include<algorithm>

class Player {
    public:
    
    //Variables
        //Logic
    bool isIdle = true;
    bool isFalling = true;
    float fallingSpeed = 0.f;
    float initialWalkSpeed = 0.f;
    float speed = 0.15f;
    float maxWalkSpeed = 4.f;
    float frictionForce = 0.1f;
    float bulletSpeed = 5.f;
    
    //Getters
    sf::Vector2f getSpriteScale();

    //Control methods
    void updateControls();
    void walkLeft();
    void walkRight();
    void jump();
    void fallDown();
    //'false' for left  'true' for right
    void shoot(bool direction);

    //Physics methods
    void updatePhysics();
    void checkRectCollision(std::vector<sf::RectangleShape*> rects);
    void checkGroundCollision(sf::RectangleShape& groundRect);
    void moveBullets();

    //Texture methods
    void initTextures(std::vector<sf::Texture>& textures, std::vector<std::string> paths);
    void updateTextures();
    void drawBullets(sf::RenderWindow& window);

    Player();
    virtual ~Player();
    void draw(sf::RenderWindow& window);
    
    //Other
    void drawPlayerTrail(sf::RenderWindow& window);
    sf::RectangleShape* playerRectangle_;
    Bullet* playerBullet_;
    std::list<std::shared_ptr<Bullet>> bullets;

    private:
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
    std::vector<sf::Texture> bulletTextures{};
    std::vector<std::string> bulletTexturesPaths{
        "images/Bullet/blue-bullet-1.png",
        "images/Bullet/blue-bullet-2.png",
        "images/Bullet/blue-bullet-3.png",
        "images/Bullet/blue-bullet-4.png"
    };

    void switchToNextIdleSprite();
    void switchToNextRunningSprite();
    void switchToNextFallingSprite();
    void switchToNextBulletSprite();

    //Rectangles
    
    


    //Sprite
    sf::Sprite* playerSprite;

    //Physics
    void applyFriction(float& walkSpeed, float friction);
    

    //Trail
    Trail* trail;
};