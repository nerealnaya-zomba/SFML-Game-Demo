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
#include<GameTextures.h>

class Player {
    public:

    Player(GameTextures& gameTextures);
    virtual ~Player();

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
    void draw(sf::RenderWindow& window);
    
    //Other
    void drawPlayerTrail(sf::RenderWindow& window);
    sf::RectangleShape* playerRectangle_;
    Bullet* playerBullet_;
    std::list<std::shared_ptr<Bullet>> bullets;

    private:
    //Textures
    std::vector<sf::Texture>* idleTextures;    

    std::vector<sf::Texture>* runningTextures;

    std::vector<sf::Texture>* fallingTextures;

    std::vector<sf::Texture>* bulletTextures;


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