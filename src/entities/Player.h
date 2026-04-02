#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <iostream>
#include <Mounting.h>
#include <sfml-headers.h>
#include <Trail.h>
#include <Bullet.h>
#include <Item.h>
#include <list>
#include <algorithm>
#include <GameData.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include <Particle.h>
#include <LevelPortal.h>
#include <CampaignProgress.h>
#include <ChooseDestinationMenu.h>

class GameLevelManager;
class LevelPortal;
class ScreenTransition;
class GameCamera;

const sf::Vector2f BASE_PORTAL_SPEED_OF_OPENING = {0.01f,0.01f};
const sf::Vector2f BASE_PORTAL_SPEED_OF_CLOSING = {0.01f,0.01f};

const int BASE_PORTAL_CALL_COOLDOWN         = 5000;
const int BASE_PORTAL_EXIST_TIME            = 10000;
const float BASE_OFFSET_TO_CREATE_PORTAL    = 200.f;

// Controls
    // Portal
    const sf::Keyboard::Key BASE_PORTAL_CALL_KEY                        = sf::Keyboard::Key::R;
    // ChooseDestinationMenu
    const sf::Keyboard::Scan BASE_CHOOSEDESTINATIONMENU_OPEN_CLOSE_KEY   = sf::Keyboard::Scan::E;
    const sf::Keyboard::Scan BASE_CHOOSEDESTINATIONMENU_SELECT_KEY       = sf::Keyboard::Scan::Enter;
    const sf::Keyboard::Scan BASE_CHOOSEDESTINATIONMENU_MOVELEFT_KEY     = sf::Keyboard::Scan::Q;
    const sf::Keyboard::Scan BASE_CHOOSEDESTINATIONMENU_MOVERIGHT_KEY    = sf::Keyboard::Scan::W;
    const sf::Keyboard::Scan BASE_WEAPON_SWITCH_PREVIOUS_KEY             = sf::Keyboard::Scan::A;
    const sf::Keyboard::Scan BASE_WEAPON_SWITCH_NEXT_KEY                 = sf::Keyboard::Scan::S;

class Player {
public:
    struct OwnedItem {
        std::string iconName;
        std::string displayName;
        Item::Quality quality = Item::COMMON;
        Item::Category category = Item::Category::Upgrade;
        int price = 0;
        Item::Stats stats{};
        Item::WeaponStats weaponStats{};
        std::string description;
    };

    Player(GameData& gameTextures, GameLevelManager& m, GameCamera& c, sf::RenderWindow& w);
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
    bool isControlsBlocked = false;
    float fallingSpeed = 0.f;
    float initialWalkSpeed = 0.f;
    float speed = 0.15f;                    // Movement acceleration
    float maxWalkSpeed = 4.f;               // Maximum horizontal speed
    float frictionForce = 0.1f;             // Ground friction
    float playerPosX_m{};                   // Initial X position from  PlayerConfig.json
    float playerPosY_m{};                   // Initial Y position from  PlayerConfig.json
    int maxHP{};                            // Max health points from   PlayerConfig.json
    int HP_{};                              // Health points from       PlayerConfig.json
    int maxEnergy{};                        // Max energy points from   PlayerConfig.json
    int energy{};                           // Energy points from       PlayerConfig.json
    int energyGain{};                       // Energy gain from         PlayerConfig.json
    int shootCost{};                        // Shoot cost from          PlayerConfig.json
    int DMG_{};                             // Damage value from        PlayerConfig.json

    // Таймеры для кнопок
    sf::Clock shootTimer;
    sf::Clock jumpTimer;
    sf::Clock dashTimer;
    sf::Clock weaponSwitchTimer;
    
    // Задержки (в секундах)
    int ButtonRepeat_shootCooldown{};
    int ButtonRepeat_jumpCooldown{};
    int ButtonRepeat_dashCooldown{};
    int ButtonRepeat_weaponSwitchCooldown{140};
    
    // Флаги готовности
    bool canShoot = true;
    bool canJump = true;
    bool canDash = true;
    bool canSwitchWeapon = true;
    
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
    int dashCooldown{};                     // From PlayerConfig.json
    sf::Clock dash_Clock;                   // Dash cooldown timer
    
    // Damage system
    sf::Clock takeDMG_timer;
    int32_t takeDMG_cooldown{};             // From PlayerConfig.json
    bool takeDMG_isOnCooldown = false;
    
    // Visual effects
    std::vector<Particle> particles;
    
    // Game objects
    sf::RectangleShape* playerRectangle_;   // Collision rectangle
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
    sf::Clock& getShootClock();
    int& getShootCooldown();
    sf::Clock& getDashClock();
    int& getDashCooldown();
    ScreenTransition& getScreenTransition();
    bool isCDMenuOpened();
    int getHP();
    int getMaxHP();
    int getEnergy();
    int getMaxEnergy();
    int getGold() const;
    int getDamageValue() const;
    int getShootCostValue() const;
    int getShootCooldownValue() const;
    int getEnergyGainValue() const;
    float getBulletSpeedValue() const;
    float getBulletRangeValue() const;
    float getAccelerationValue() const;
    float getMaxWalkSpeedValue() const;
    float getDashForceValue() const;
    int getDashCooldownValue() const;
    float getJumpPowerValue() const;
    int getExtraJumpCountValue() const;
    int getSlowFallPercentValue() const;
    bool canAfford(int amount) const;
    const std::vector<OwnedItem>& getInventory() const;
    bool ownsItem(const std::string& iconName) const;
    sf::Vector2f getFeetPosition() const;
    int takeAllGold();
    std::string getCurrentWeaponName() const;
    std::string getCurrentWeaponIconName() const;
    Item::Quality getCurrentWeaponQuality() const;
    const std::vector<OwnedItem>& getWeapons() const;
    CampaignObjectiveSnapshot getCampaignSnapshot() const;
    const CampaignProgress& getCampaignProgress() const;
    std::string getCampaignBoonTitle() const;
    bool isLevelUnlocked(const std::string& levelName) const;
    std::vector<std::string> getUnlockedLevelNames(const std::vector<std::string>& levelNames) const;
    std::string getLevelUnlockHint(const std::string& levelName) const;

        // Setters
    void attachGameLevelManager(GameLevelManager& m);
    void setPosition(sf::Vector2f pos);
    void addGold(int amount);
    bool spendGold(int amount);
    bool tryPurchaseItem(const Item& item);
    void restoreVitalResources();
    void resetProgress();
    void teleportToSupportPoint(const sf::Vector2f& supportPoint);
    void forceKill();
    void respawnAt(sf::Vector2f pos);
    void notifyLevelEntered(const std::string& levelName);

        // Control methods
    void updateControls();                  // Process player input
    void walkLeft();                        // Move left
    void walkRight();                       // Move right  
    void jump();                            // Jump action
    void fallDown();                        // Force fall
    void dash();                            // Dash ability
    bool shoot(bool direction);             // Shoot (false=left, true=right)
    
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
    void updateParticles(); 
    void updateEnergy();                                                              // Update particle effects
    
        // Rendering methods
    void updateTextures();                          // Update sprite animations
    void drawBullets(sf::RenderWindow& window);     // Draw all bullets
    void draw(sf::RenderWindow& window);            // Draw player
    void drawParticles(sf::RenderWindow& window);   // Draw particles
    void drawPlayerTrail(sf::RenderWindow& window); // Draw movement trail

    // Block/Unblock controls
    void blockControls();
    void unblockControls();
private:
    struct VisualRing {
        sf::Vector2f position{};
        sf::Color color{255, 255, 255, 255};
        float radius = 10.f;
        float maxRadius = 56.f;
        float growth = 2.f;
        float thickness = 2.f;
        float alpha = 180.f;
    };

    // Texture arrays
    std::vector<sf::Texture>* idleTextures;     // Idle animation frames
    std::vector<sf::Texture>* runningTextures;  // Running animation frames  
    std::vector<sf::Texture>* fallingTextures;  // Falling animation frames

    // Animation methods
    void switchToNextIdleSprite();
    void switchToNextRunningSprite(); 
    void switchToNextFallingSprite();

    ////////////////////////////////////////////////////////////////
    // Used objects
    GameCamera* camera = nullptr;
    sf::Sprite* playerSprite;               // Main player sprite
        // Movement trail effect
        Trail* trail;                           
        // LevelPortal
        LevelPortal* portal;
            // ChooseDestinationMenu for LevelPortal
            ChooseDestinationMenu CDMenu;
        // Screen transition effect on level change
        std::shared_ptr<ScreenTransition> transition;
    ////////////////////////////////////////////////////////////////

    std::vector<VisualRing> effectRings_;
    bool deathEffectPlayed_ = false;
    bool wasInTeleportArea_ = false;

    sf::Clock runEffectClock_;
    sf::Clock teleportEffectClock_;
    sf::Clock criticalEffectClock_;
    sf::Clock landingEffectClock_;

    const float RUN_EFFECT_INTERVAL_MS = 145.f;
    const float TELEPORT_EFFECT_INTERVAL_MS = 90.f;
    const float CRITICAL_EFFECT_INTERVAL_MS = 240.f;
    const float LANDING_EFFECT_COOLDOWN_MS = 140.f;
    const float CRITICAL_HP_RATIO = 0.35f;

    void pushRing(
        const sf::Vector2f& position,
        const sf::Color& color,
        float radius,
        float maxRadius,
        float growth,
        float thickness,
        float alpha
    );
    void spawnParticleBurst(
        const sf::Vector2f& origin,
        const sf::Color& color,
        int count,
        float minSpeed,
        float maxSpeed,
        float radius,
        float gravity,
        float lifetime
    );
    void spawnRunEffect();
    void spawnJumpEffect();
    void spawnLandingEffect(float impactStrength);
    void spawnShootEffect(bool direction);
    void spawnDashBurst();
    void spawnTeleportEffect(bool enteringPortal);
    void spawnDeathEffect();
    void spawnCriticalHealthEffect();
    void spawnWeaponSwitchEffect();
    void updateRingEffects();
    void triggerCameraImpact(const sf::Vector2f& direction, float impulseStrength, float trauma, float zoomPunch = 0.02f);
    float getFacingDirection() const;
    void initializeDefaultWeapon();
    const OwnedItem& getCurrentWeapon() const;
    void switchWeapon(int direction);
    void applyCurrentWeaponStats();
    bool canPerformJump() const;
    void restoreAirJumps();
    std::shared_ptr<Bullet> createProjectile(const Bullet::Config& config, const sf::Vector2f& startPosition, const sf::Vector2f& speed);

    ////////////////////////////////////////////////////////////
    // LevelPortal methods and fields  
    ////////////////////////////////////////////////////////////
    sf::Clock portalCooldownClock; // For checking cooldown time
    sf::Clock portalCallOpenCooldownClock;
    sf::Clock portalCallCloseCooldownClock;
    int portalCallCooldown = BASE_PORTAL_CALL_COOLDOWN;
    int portalExistTime    = BASE_PORTAL_EXIST_TIME;
    sf::Keyboard::Key portalCallKey = BASE_PORTAL_CALL_KEY;
    bool isPortalOnCooldown = false;
    void portalUpdate();
    void tryOpenPortal();
public:
    sf::Clock& getPortalClock();
    int& getPortalCooldown();
    void setPortalDestination(std::optional<std::string> levelName);
private:
    ////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////
    // ChooseDestinationMenu methods and fields
    ////////////////////////////////////////////////////////////
public:
    void chooseDestinationMenuDraw(sf::RenderWindow& w);     
    void chooseDestinationMenuUpdate();
    void chooseDestinationMenuHandleEvents(const sf::Event& ev);
private:
    ////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////
    // ScreenTransition methods and fields  
    ////////////////////////////////////////////////////////////
public:
    void drawTransition();
    void playFadeInAnimation();
private:
    void updateTransition();
////////////////////////////////////////////////////////////

    // Physics
    void applyFriction(float& walkSpeed, float friction); // Apply friction to movement

    // Data persistence
    void saveData();                        // Save player data to file
    void loadData();                        // Load player data from file
    void loadProgressData();
    void recalculateStatsFromInventory();
    void restoreOwnedItem(const OwnedItem& itemData);

    int gold_ = 0;
    int baseHP_ = 0;
    int baseMaxEnergy_ = 0;
    int baseEnergyGain_ = 0;
    int baseShootCost_ = 0;
    int baseDMG_ = 0;
    float baseBulletSpeed_ = 0.f;
    float baseBulletMaxDistance_ = 0.f;
    int baseShootCooldown_ = 0;
    float baseAcceleration_ = 0.f;
    float baseMaxWalkSpeed_ = 0.f;
    float baseDashForce_ = 0.f;
    int baseDashCooldown_ = 0;
    float baseJumpImpulse_ = 5.5f;
    float jumpImpulse_ = 5.5f;
    float baseGravity_ = 0.1f;
    float gravity_ = 0.1f;
    int maxAirJumps_ = 0;
    int airJumpsRemaining_ = 0;
    bool jumpKeyWasDown_ = false;
    Item::Stats inventoryStatsBonus_{};
    std::vector<OwnedItem> inventory_;
    std::vector<OwnedItem> arsenal_;
    std::size_t currentWeaponIndex_ = 0;
    CampaignProgress campaignProgress_{};
};
