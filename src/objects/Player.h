#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <iostream>
#include <Mounting.h>
#include <sfml-headers.h>
#include <Trail.h>
#include <Bullet.h>
#include <list>
#include <algorithm>
#include <GameData.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include <Particle.h>

class Player {
public:
    Player(GameData& gameTextures);
    virtual ~Player();

    // Public variables
    // Player state
    bool isIdle = true;
    bool isFalling = true;
    bool isAlive = true;
    float fallingSpeed = 0.f;
    float initialWalkSpeed = 0.f;
    float speed = 0.15f;                    // Movement acceleration
    float maxWalkSpeed = 4.f;               // Maximum horizontal speed
    float frictionForce = 0.1f;             // Ground friction
    float playerPosX_m{};                   // Initial X position from PlayerConfig.json
    float playerPosY_m{};                   // Initial Y position from PlayerConfig.json
    int HP_{};                               // Health points from PlayerConfig.json
    int DMG_{};                             // Damage value from PlayerConfig.json
    //Player animation state
    bool isPlayingDieAnimation = false;
    
    // Bullet properties
    float bulletSpeed;                      // From PlayerConfig.json
    float bulletMaxDistance_;               // From PlayerConfig.json
    
    // Dash mechanics
    bool isDashAvaiable = false;
    float dashForce{};                      // From PlayerConfig.json
    sf::Clock dash_Clock;                   // Dash cooldown timer
    
    // Damage system
    sf::Clock takeDMG_timer;
    int32_t takeDMG_cooldown = 1000;        // From PlayerConfig.json
    bool takeDMG_isOnCooldown = false;
    
    // Visual effects
    std::vector<Particle> bloodParticles;
    
    // Game objects
    sf::RectangleShape* playerRectangle_;   // Collision rectangle
    Bullet* playerBullet_;                  // Bullet template
    std::list<std::shared_ptr<Bullet>> bullets; // Active bullets

    std::vector<sf::Texture>* satiro_dieTextures;
    texturesIterHelper satiro_die_helper;
    std::vector<sf::Texture>* satiro_dashTextures;
    texturesIterHelper satiro_dash_helper;
    std::vector<sf::Texture>* satiro_hurtTextures;
    texturesIterHelper satiro_hurt_helper;
    std::vector<sf::Texture>* satiro_slideTextures;
    texturesIterHelper satiro_slide_helper;

    // Public methods
    
        // Getters
    sf::Vector2f getSpriteScale();
    sf::Vector2f getCenterPosition();

        // Control methods
    void updateControls();                  // Process player input
    void walkLeft();                        // Move left
    void walkRight();                       // Move right  
    void jump();                            // Jump action
    void fallDown();                        // Force fall
    void dash();                            // Dash ability
    void shoot(bool direction);             // Shoot (false=left, true=right)
    
        // Damage system
    bool takeDMG(int count, sf::Vector2f knockback, bool side);                // Take damage, returns if damage was applied
    void bloodExplode();                    // Create blood particle effect
    
        // Physics methods
    void updatePhysics();                   // Update player physics
    void checkRectCollision(std::vector<std::shared_ptr<sf::RectangleShape>>& rects); // Platform collision
    void checkGroundCollision(sf::RectangleShape& groundRect); // Ground collision
    void moveBullets();                     // Update all active bullets
    void updateParticles();                 // Update particle effects
    
        // Rendering methods
    void initTextures(std::vector<sf::Texture>& textures, std::vector<std::string> paths);
    void updateTextures();                  // Update sprite animations
    void drawBullets(sf::RenderWindow& window); // Draw all bullets
    void draw(sf::RenderWindow& window);    // Draw player
    void drawParticles(sf::RenderWindow& window); // Draw particles
    void drawPlayerTrail(sf::RenderWindow& window); // Draw movement trail

private:
    // Texture arrays
    std::vector<sf::Texture>* idleTextures;     // Idle animation frames
    std::vector<sf::Texture>* runningTextures;  // Running animation frames  
    std::vector<sf::Texture>* fallingTextures;  // Falling animation frames
    std::vector<sf::Texture>* bulletTextures;   // Bullet animation frames

    // Animation methods
    void switchToNextIdleSprite();
    void switchToNextRunningSprite(); 
    void switchToNextFallingSprite();
    void switchToNextBulletSprite();

    // Graphics
    sf::Sprite* playerSprite;               // Main player sprite
    Trail* trail;                           // Movement trail effect

    // Physics
    void applyFriction(float& walkSpeed, float friction); // Apply friction to movement

    // Data persistence
    void saveData();                        // Save player data to file
    void loadData();                        // Load player data from file
};