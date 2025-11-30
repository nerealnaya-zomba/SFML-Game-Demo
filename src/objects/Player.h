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
#include<GameData.h>
#include<nlohmann/json.hpp>
#include<fstream>

class Player {
    public:

    Player(GameData& gameTextures);
    virtual ~Player();

    //Variables
        //Logic
            //Player
    bool isIdle = true;
    bool isFalling = true;
    float fallingSpeed = 0.f;
    float initialWalkSpeed = 0.f;
    float speed = 0.15f;
    float maxWalkSpeed = 4.f;
    float frictionForce = 0.1f;
    float playerPosX_m{}; //PlayerConfig.json
    float playerPosY_m{}; //PlayerConfig.json
    int HP{}; //PlayerConfig.json
    int DMG_{}; //PlayerConfig.json
            //Bullet
    float bulletSpeed; //PlayerConfig.json
    float bulletMaxDistance_; //PlayerConfig.json
            //Dash
    bool isDashAvaiable = false;
    float dashForce{}; //PlayerConfig.json
    sf::Clock dash_Clock;

    //Getters
    sf::Vector2f getSpriteScale();

    //Control methods
    void updateControls();
    void walkLeft();
    void walkRight();
    void jump();
    void fallDown();
    void dash();
        //'false' for left  'true' for right
    void shoot(bool direction);
        //Damage
    bool takeDMG(int count);
    sf::Clock takeDMG_timer;
    int32_t takeDMG_cooldown = 1000; //PlayerConfig.json //[ ] add in playerconfg
    bool takeDMG_isOnCooldown = false;
    
    //Physics methods
    void updatePhysics();
    void checkRectCollision(std::vector<std::shared_ptr<sf::RectangleShape>>& rects);
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


    //Sprite
    sf::Sprite* playerSprite;

    //Physics
    void applyFriction(float& walkSpeed, float friction);
    

    //Trail
    Trail* trail;

    //Save-load data methods
    void saveData();
    void loadData();

};