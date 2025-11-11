#pragma once
#include "Enemy.h"  // кавычки вместо <>
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

enum skeletonAction{
    WALKLEFT,
    WALKRIGHT,
    IDLE,
    HURT,
    DIE,
    ATTACK1,
    ATTACK2,
};

class Skeleton : public Enemy{ //Ошибка: expected class name
private:
    //References
        //Draw&display(window)
    sf::RenderWindow* window;
        //Ground
        Ground* ground_; //NOTE used for physics
        //Platforms
        Platform* platform_; //NOTE used for physics
        //Player
        Player* player_; //NOTE used for physics
    //Variables
        //Selected skeleton type
    std::string type_;
        //Actions
    bool isIdle = true;
    bool isFalling = true;
    skeletonAction action_ = skeletonAction::IDLE;
        //Physics
    float fallingSpeed = 0.f;
    float initialWalkSpeed = 0.f;
    float speed = 0.15f; 
    float maxWalkSpeed = 4.f;
    float frictionForce = 0.1f;
    sf::Vector2f enemyPos = {WINDOW_WIDTH/2,WINDOW_HEIGHT/2};
        //Pre-load bindings
    sf::Vector2f enemyScale_; // Loads from EnemySettings.json
        //Sprite
    sf::Sprite* skeletonSprite;
        //Rect //NOTE used for physics
    sf::RectangleShape* skeletonRect; 

    //Textures
        //Skeleton white
    std::vector<sf::Texture>* skeletonWhite_idleTextures;
    texturesIterHelper* skeletonWhite_idle_helper;
    std::vector<sf::Texture>* skeletonWhite_walkTextures;
    texturesIterHelper* skeletonWhite_walk_helper;
    std::vector<sf::Texture>* skeletonWhite_hurtTextures;
    texturesIterHelper* skeletonWhite_hurt_helper;
    std::vector<sf::Texture>* skeletonWhite_dieTextures;
    texturesIterHelper* skeletonWhite_die_helper;
    std::vector<sf::Texture>* skeletonWhite_attack1Textures;
    texturesIterHelper* skeletonWhite_attack1_helper;
    std::vector<sf::Texture>* skeletonWhite_attack2Textures;
    texturesIterHelper* skeletonWhite_attack2_helper;
        //Skeleton yellow
    std::vector<sf::Texture>* skeletonYellow_idleTextures;
    texturesIterHelper* skeletonYellow_idle_helper;
    std::vector<sf::Texture>* skeletonYellow_walkTextures;
    texturesIterHelper* skeletonYellow_walk_helper;
    std::vector<sf::Texture>* skeletonYellow_hurtTextures;
    texturesIterHelper* skeletonYellow_hurt_helper;
    std::vector<sf::Texture>* skeletonYellow_dieTextures;
    texturesIterHelper* skeletonYellow_die_helper;
    std::vector<sf::Texture>* skeletonYellow_attack1Textures;
    texturesIterHelper* skeletonYellow_attack1_helper;
    std::vector<sf::Texture>* skeletonYellow_attack2Textures;
    texturesIterHelper* skeletonYellow_attack2_helper;

    //PRIVATE physics methods
    void checkGroundCollision(Ground& ground);
    void checkPlatformCollision(Platform& platforms);
    void checkBulletCollision(Player& player);
    void applyFriction(float& walkSpeed, float friction);

    //PRIVATE texture methods
    void switchToNextSprite(sf::Sprite* enemySprite, std::vector<sf::Texture>& texturesArray, texturesIterHelper& iterHelper);

    void loadData(); //Loads some variables from EnemySettings.json
public:
    Skeleton(GameData &gameData, sf::RenderWindow &window, Ground& ground, Platform& platform, Player& player, std::string type);
    ~Skeleton();

    void updateAI();

    void updatePhysics();

    void updateTextures();

    void draw();

};

