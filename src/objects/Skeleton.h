#pragma once
#include "Enemy.h"
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <VisualEffects.h>
#include <SFML/System.hpp>
#include <HealthBar.h>
#include <math.h>

enum skeletonAction {
    WALKLEFT,
    WALKRIGHT,
    IDLE,
    HURT,
    DIE,
    ATTACK1,
    ATTACK2,
};

class Skeleton : public Enemy {
private:
    // External references
    sf::RenderWindow* window;           // For rendering
    Ground* ground_;                    // For ground collision detection
    Platform* platform_;                // For platform collision detection  
    Player* player_;                    // For player interaction

    // Skeleton properties
    std::string type_;                  // "white" or "yellow" skeleton type
    HealthBar* healthbar;               // Health display

    // State flags
    bool isIdle = true;
    bool isFalling = true;
    bool isPlayingHurtAnimation = false;
    bool isPlayingDieAnimation = false;
    
    skeletonAction action_ = skeletonAction::IDLE;

    // Physics properties
    float fallingSpeed = 0.f;
    float initialWalkSpeed = 0.f;
    float speed{};                      // Movement speed from EnemySettings.json
    float maxWalkSpeed{};               // Max speed from EnemySettings.json
    float frictionForce{};              // Friction from EnemySettings.json
    int HP_;                            // Health from EnemySettings.json
    int DMG_;                           // Damage output
    sf::Vector2f knockback_;
    float distanceToMakeAttack{};       // Attack range from EnemySettings.json
    float distanceToHit_byAttack{};     // Hit range from EnemySettings.json
    bool knockbacks{};                  // If attacks cause knockback
    sf::Vector2f enemyPos;              // Initial position from constructor

    // Visual properties
    sf::Vector2f enemyScale_;           // Scale from EnemySettings.json
    sf::Sprite* skeletonSprite;         // Main sprite
    sf::RectangleShape* skeletonRect;   // Collision rectangle

    // Texture arrays for white skeleton
    std::vector<sf::Texture>* skeleton_idleTextures;
    texturesIterHelper skeleton_idle_helper;
    std::vector<sf::Texture>* skeleton_walkTextures;
    texturesIterHelper skeleton_walk_helper;
    std::vector<sf::Texture>* skeleton_hurtTextures;
    texturesIterHelper skeleton_hurt_helper;
    std::vector<sf::Texture>* skeleton_dieTextures;
    texturesIterHelper skeleton_die_helper;
    std::vector<sf::Texture>* skeleton_attack1Textures;
    texturesIterHelper skeleton_attack1_helper;
    std::vector<sf::Texture>* skeleton_attack2Textures;
    texturesIterHelper skeleton_attack2_helper;

    // Texture arrays for yellow skeleton
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

    // Movement control
    void walkLeft();
    void walkRight();

    // AI behavior
    void chasePlayer(sf::Vector2f skeletonPos, sf::Vector2f playerPos);
    void tryAttackPlayer();

    // Action handlers
    void onBulletHit();

    // Physics and collision
    void checkGroundCollision(Ground& ground);
    void checkPlatformCollision(Platform& platforms);
    void checkBulletCollision(Player& player);
    void applyFriction(float& walkSpeed, float friction);

    // Data management
    void loadData(); // Load from EnemySettings.json

public:
    Skeleton(GameData &gameData, sf::RenderWindow &window, Ground& ground, 
             Platform& platform, Player& player, std::string type, sf::Vector2f pos);
    ~Skeleton();

    bool isAlive = true;
    // Core update methods
    void updateAI();        // Artificial intelligence
    void updateControl();   // Movement control
    void updatePhysics();   // Physics simulation
    void updateTextures();  // Animation updates
    void draw();           // Rendering

    // Getters
    sf::RectangleShape& getRect();
    int getHP();
};