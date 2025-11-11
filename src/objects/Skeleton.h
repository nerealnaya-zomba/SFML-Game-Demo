#pragma once
#include<Enemy.h>
#include<Ground.h>
#include<Platform.h>

class Skeleton : public Enemy{
private:
    //References
        //Draw&display(window)
    sf::RenderWindow* window;
        //Ground
        Ground* ground_; //NOTE at least, used for physics
        //Platforms
        Platform* platform_; //NOTE at least, used for physics

    //Variables
        //Selected type
    std::string type_;
        //Logic
    bool isIdle = true;
    bool isFalling = true;
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
    void applyFriction(float& walkSpeed, float friction);

    //PRIVATE texture methods
    void switchToNextSprite(sf::Sprite* enemySprite, std::vector<sf::Texture>& texturesArray, texturesIterHelper& iterHelper);

    void loadData(); //Loads some variables from EnemySettings.json
public:
    Skeleton(GameData &gameData, sf::RenderWindow &window, Ground& ground, Platform& platform, std::string type);
    ~Skeleton();

    void updateAI();

    void updatePhysics();

    void updateTextures();

    void draw();

};