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

enum PatrolState {
    PATROL_EXPLORING_LEFT,   // Исследуем левую границу
    PATROL_EXPLORING_RIGHT,  // Исследуем правую границу
    PATROL_PATROLLING        // Патрулируем между границами
};

enum ExplorationState {
    EXPLORE_NONE,      // Не исследовали
    EXPLORE_LEFT,      // Исследуем левую сторону
    EXPLORE_RIGHT,     // Исследуем правую сторону
    PATROLLING         // Патрулируем между найденными границами
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
        //Physics
    bool isIdle = true;
    bool isFalling = true;
        //Animation
    bool isPlayingHurtAnimation = false;
    bool isPlayingDieAnimation = false;
        //AI
    bool isPlayerOutOfReach = false;
    
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
    void patrol();
    void makeRandomPatrolVariables();
    
    sf::Clock switchTimer;
    // Состояние исследования
    ExplorationState explorationState = EXPLORE_NONE;

    // Таймеры для детекции тупиков
    sf::Clock exploreTimer;
    sf::Clock deadEndCheckTimer;
    sf::Clock directionSwitchTimer;

    // Стартовая позиция для измерения движения
    float exploreStartPos = 0.0f;

    // Границы патрулирования
    float leftBound = 0.0f;
    float rightBound = 0.0f;

    // Флаги исследованных сторон
    bool leftExplored = false;
    bool rightExplored = false;

    // Защита от быстрого переключения направления
    bool recentlySwitchedDirection = false;

    // Константы
    const float TIME_TO_CHECK_DEADEND = 300.0f;    // Время проверки тупика (мс)
    const float MIN_DISTANCE_FOR_DEADEND = 10.0f;   // Минимальное расстояние для детекции тупика
    const float DIRECTION_SWITCH_COOLDOWN = 300.0f; // Задержка перед сменой направления
    const float DIRECTION_SWITCH_OFFSET = 100.0f;     // Отступ от границы для смены направления
    // Состояния
    bool isFirstEnter = true;
    bool isEncounteredNewDeadEnd = false;
    bool makeRandomStart = false;
    
    // Границы патрулирования
    float patrolLeftBound;
    float patrolRightBound;
    
    // Позиции для детекции застревания
    float pastSkeletonPos = 0.0f;
    float currentSkeletonPos = 0.0f;
    PatrolState patrolState = PATROL_PATROLLING;
    // Таймеры
    sf::Clock updateClock;
    sf::Clock isPlayerOutOfReachClock;
    sf::Clock patrolDeadEndClock;
    
    // Параметры (в миллисекундах)
    const float UPDATE_INTERVAL = 50.0f;
    const float DEADEND_DETECTION_TIME = 500.0f;
    const float PATROL_SWITCH_DELAY = 1000.0f;
    const float MIN_MOVEMENT_THRESHOLD = 5.0f; // пикселей
    
    // Дистанции
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