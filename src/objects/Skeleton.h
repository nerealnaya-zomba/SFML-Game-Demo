#pragma once
#include "Enemy.h"
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include<VisualEffects.h>
#include<SFML/System.hpp>
#include<HealthBar.h>
#include<math.h>

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
        //Draw & display
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
    bool isPlayingHurtAnimation = false;
    bool isPlayingDieAnimation = false;
    skeletonAction action_ = skeletonAction::IDLE;
        //HP bar
    HealthBar* healthbar;
        //Physics
    float fallingSpeed = 0.f;
    float initialWalkSpeed = 0.f;
    float speed{}; // Loads from EnemySettings.json
    float maxWalkSpeed{}; // Loads from EnemySettings.json
    float frictionForce{}; // Loads from EnemySettings.json
    int HP_; // Loads from EnemySettings.json
    sf::Vector2f enemyPos; //In constuctor
        //Pre-load bindings
    sf::Vector2f enemyScale_; // Loads from EnemySettings.json
        //Sprite
    sf::Sprite* skeletonSprite;
        //Rect //NOTE used for physics
    sf::RectangleShape* skeletonRect; 

    //Textures
        //Skeleton white
    std::vector<sf::Texture>* skeletonWhite_idleTextures;
    texturesIterHelper skeletonWhite_idle_helper;
    std::vector<sf::Texture>* skeletonWhite_walkTextures;
    texturesIterHelper skeletonWhite_walk_helper;
    std::vector<sf::Texture>* skeletonWhite_hurtTextures;
    texturesIterHelper skeletonWhite_hurt_helper;
    std::vector<sf::Texture>* skeletonWhite_dieTextures;
    texturesIterHelper skeletonWhite_die_helper;
    std::vector<sf::Texture>* skeletonWhite_attack1Textures;
    texturesIterHelper skeletonWhite_attack1_helper;
    std::vector<sf::Texture>* skeletonWhite_attack2Textures;
    texturesIterHelper skeletonWhite_attack2_helper;
        //Skeleton yellow
    std::vector<sf::Texture>* skeletonYellow_idleTextures;
    texturesIterHelper skeletonYellow_idle_helper;
    std::vector<sf::Texture>* skeletonYellow_walkTextures;
    texturesIterHelper skeletonYellow_walk_helper;
    std::vector<sf::Texture>* skeletonYellow_hurtTextures;
    texturesIterHelper skeletonYellow_hurt_helper;
    std::vector<sf::Texture>* skeletonYellow_dieTextures;
    texturesIterHelper skeletonYellow_die_helper;
    std::vector<sf::Texture>* skeletonYellow_attack1Textures;
    texturesIterHelper skeletonYellow_attack1_helper;
    std::vector<sf::Texture>* skeletonYellow_attack2Textures;
    texturesIterHelper skeletonYellow_attack2_helper;

    //PRIVATE control methods
    void walkLeft();
    void walkRight();

    //PRIVATE AI methods
    void chasePlayer(sf::Vector2f skeletonPos, sf::Vector2f playerPos);


    //PRIVATE action methods
    void onBulletHit();

    //PRIVATE physics methods
    void checkGroundCollision(Ground& ground);
    void checkPlatformCollision(Platform& platforms);
    void checkBulletCollision(Player& player);
    void applyFriction(float& walkSpeed, float friction);

    //PRIVATE texture methods
        //Loop forward-backwards
    bool switchToNextSprite(sf::Sprite* enemySprite,
        std::vector<sf::Texture>& texturesArray, 
        texturesIterHelper& iterHelper, 
        switchSprite_SwitchOption option=switchSprite_SwitchOption::Loop);

    void loadData(); //Loads some variables from EnemySettings.json
public:
    Skeleton(GameData &gameData, sf::RenderWindow &window, Ground& ground, Platform& platform, Player& player, std::string type, sf::Vector2f pos);
    ~Skeleton();
    //PUBLIC Variables
    bool isAlive = true;

    void updateAI();

    void updateControl();

    void updatePhysics();

    void updateTextures();

    void draw();

    //Getters
    sf::RectangleShape& getRect();
    int getHP();
};