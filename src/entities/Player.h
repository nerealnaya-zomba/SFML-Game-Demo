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
#include<LevelPortal.h>

class GameLevelManager;
class LevelPortal;

const int portalExistTime = 2000;
const sf::Vector2f BASE_PORTAL_SPEED_OF_OPENING = {0.01f,0.01f};
const sf::Vector2f BASE_PORTAL_SPEED_OF_CLOSING = {0.01f,0.01f};
const int BASE_PORTAL_CALL_COOLDOWN = 1000;
const int BASE_PORTAL_EXIST_TIME = 2000;
const float BASE_OFFSET_TO_CREATE_PORTAL = 200.f;
const sf::Keyboard::Key BASE_PORTAL_CALL_KEY = sf::Keyboard::Key::R;

class Player {
public:
    Player(GameData& gameTextures, GameLevelManager& m);
    virtual ~Player();

    GameData* gameTextures;
    GameLevelManager* levelManager;

    // Public variables
    // Player state
    bool isIdle    = true;
    bool isFalling = true;
    bool isAlive   = true;
    bool isFliesUp = false;
    bool isJumped  = false;
    float fallingSpeed = 0.f;
    float initialWalkSpeed = 0.f;
    float speed = 0.15f;                    // Movement acceleration
    float maxWalkSpeed = 4.f;               // Maximum horizontal speed
    float frictionForce = 0.1f;             // Ground friction
    float playerPosX_m{};                   // Initial X position from PlayerConfig.json
    float playerPosY_m{};                   // Initial Y position from PlayerConfig.json
    int HP_{};                              // Health points from PlayerConfig.json
    int DMG_{};                             // Damage value from PlayerConfig.json

    // Таймеры для кнопок
    sf::Clock shootTimer;
    sf::Clock jumpTimer;
    sf::Clock dashTimer;
    
    // Задержки (в секундах)
    float ButtonRepeat_shootCooldown{};
    float ButtonRepeat_jumpCooldown{};
    float ButtonRepeat_dashCooldown{};
    
    // Флаги готовности
    bool canShoot = true;
    bool canJump = true;
    bool canDash = true;
    
    //Player animation state
    bool isPlayingDieAnimation = false;
    bool isPlayingHurtAnimation = false;
    bool isPlayingDashAnimation = false;
    
    // Bullet properties
    float bulletSpeed;                      // From PlayerConfig.json
    float bulletMaxDistance_;               // From PlayerConfig.json
    float bulletSpeedReduction;             // From PlayerConfig.json
    // Dash mechanics
    bool isDashOnCooldown = false;
    float dashForce{};                      // From PlayerConfig.json
    float dashCooldown{};                   // From PlayerConfig.json
    sf::Clock dash_Clock;                   // Dash cooldown timer
    
    // Damage system
    sf::Clock takeDMG_timer;
    int32_t takeDMG_cooldown{};             // From PlayerConfig.json
    bool takeDMG_isOnCooldown = false;
    
    // Visual effects
    std::vector<Particle> particles;
    
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
    std::vector<sf::Texture>* satiro_jumpTextures;
    texturesIterHelper satiro_jump_helper;
    std::vector<sf::Texture>* satiro_landingTextures;
    texturesIterHelper satiro_landing_helper;

    // Public methods
        // Getters
    sf::Vector2f getSpriteScale();
    sf::Vector2f getCenterPosition();

        // Setters
    void attachGameLevelManager(GameLevelManager& m);

        // Control methods
    void updateControls();                  // Process player input
    void walkLeft();                        // Move left
    void walkRight();                       // Move right  
    void jump();                            // Jump action
    void fallDown();                        // Force fall
    void dash();                            // Dash ability
    void shoot(bool direction);             // Shoot (false=left, true=right)
    
        //Dash particles
    void dashParticles();

        // Damage system
    bool takeDMG(int count, sf::Vector2f knockback, bool side);                       // Take damage, returns if damage was applied
    void bloodExplode();                                                              // Create blood particle effect
    
        // Physics methods
    void updatePhysics();                                                             // Update player physics
    void checkPlatformRectCollision(std::vector<std::shared_ptr<sf::RectangleShape>>& rects); // Platform collision
    void checkGroundCollision(sf::RectangleShape& groundRect);                        // Ground collision
    void moveBullets();                                                               // Update all active bullets
    void updateParticles();                                                           // Update particle effects
    
        // Rendering methods
    void initTextures(std::vector<sf::Texture>& textures, std::vector<std::string> paths);
    void updateTextures();                          // Update sprite animations
    void drawBullets(sf::RenderWindow& window);     // Draw all bullets
    void draw(sf::RenderWindow& window);            // Draw player
    void drawParticles(sf::RenderWindow& window);   // Draw particles
    void drawPlayerTrail(sf::RenderWindow& window); // Draw movement trail

private:
    // Texture arrays
    std::vector<sf::Texture>* idleTextures;     // Idle animation frames
    std::vector<sf::Texture>* runningTextures;  // Running animation frames  
    std::vector<sf::Texture>* fallingTextures;  // Falling animation frames

    // Animation methods
    void switchToNextIdleSprite();
    void switchToNextRunningSprite(); 
    void switchToNextFallingSprite();

    // Used objects
    sf::Sprite* playerSprite;               // Main player sprite
    Trail* trail;                           // Movement trail effect

    // LevelPortal
    LevelPortal* portal;
    sf::Clock portalCallOpenCooldownClock;
    sf::Clock portalCallCloseCooldownClock;
    int portalCallCooldown = BASE_PORTAL_CALL_COOLDOWN;
    int portalExistTime    = BASE_PORTAL_EXIST_TIME;
    sf::Keyboard::Key portalCallKey = BASE_PORTAL_CALL_KEY;
    bool isPortalOnCooldown = false;

    // Physics
    void applyFriction(float& walkSpeed, float friction); // Apply friction to movement

    // Data persistence
    void saveData();                        // Save player data to file
    void loadData();                        // Load player data from file
};