#pragma once

#include "Enemy.h"
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <VisualEffects.h>
#include <SFML/System.hpp>
#include <HealthBar.h>
#include <math.h>

// Анимационные состояния скелета
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
    // Внешние ссылки
    sf::RenderWindow* window;
    Ground* ground_;
    Platform* platform_;  
    Player* player_;

    // Основные свойства
    std::string type_;              // "white" или "yellow"
    HealthBar* healthbar;
    
    // Флаги состояний
    bool isIdle = true;
    bool isFalling = true;
    bool isPlayingHurtAnimation = false;
    bool isPlayingDieAnimation = false;
    bool isPlayerOutOfReach = false;
    
    skeletonAction action_ = IDLE;

    // Физические параметры
    float fallingSpeed = 0.f;
    float initialWalkSpeed = 0.f;
    float speed;
    float maxWalkSpeed;
    float frictionForce;
    int HP_;
    int DMG_;
    sf::Vector2f knockback_;
    float distanceToMakeAttack;
    float distanceToHit_byAttack;
    bool knockbacks;
    sf::Vector2f enemyPos;

    // Графика
    sf::Vector2f enemyScale_;
    sf::Sprite* skeletonSprite;
    sf::RectangleShape* skeletonRect;

    // Текстуры белого скелета
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

    // Текстуры желтого скелета
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

    // Движение
    void walkLeft();
    void walkRight();

    // ИИ
    void chasePlayer(sf::Vector2f skeletonPos, sf::Vector2f playerPos);
    void patrol();
    void makeRandomPatrolVariables();
    
    // Система патрулирования
    enum PatrolState { PATROL_EXPLORING_LEFT, PATROL_EXPLORING_RIGHT, PATROL_PATROLLING };
    enum ExplorationState { EXPLORE_NONE, EXPLORE_LEFT, EXPLORE_RIGHT, PATROLLING };
    
    ExplorationState explorationState = EXPLORE_NONE;
    sf::Clock deadEndCheckTimer;
    sf::Clock directionSwitchTimer;
    
    float exploreStartPos = 0.0f;
    float leftBound = 0.0f;
    float rightBound = 0.0f;
    bool leftExplored = false;
    bool rightExplored = false;
    bool recentlySwitchedDirection = false;
    bool isFirstEnter = true;
    bool makeRandomStart = false;
    float currentSkeletonPos = 0.0f;
    
    // Константы ИИ
    const float TIME_TO_CHECK_DEADEND = 300.0f;
    const float MIN_DISTANCE_FOR_DEADEND = 10.0f;
    const float DIRECTION_SWITCH_COOLDOWN = 300.0f;
    const float DIRECTION_SWITCH_OFFSET = 100.0f;
    const float PATROL_SWITCH_DELAY = 1000.0f;
    
    // Таймеры
    sf::Clock isPlayerOutOfReachClock;

    // Атака
    void tryAttackPlayer();
    void onBulletHit();

    // Физика и коллизии
    void checkGroundCollision(Ground& ground);
    void checkPlatformCollision(Platform& platforms);
    void checkBulletCollision(Player& player);
    void applyFriction(float& walkSpeed, float friction);

    // Данные
    void loadData();

public:
    Skeleton(GameData &gameData, sf::RenderWindow &window, Ground& ground, 
             Platform& platform, Player& player, std::string type, sf::Vector2f pos);
    ~Skeleton();

    bool isAlive = true;
    
    void updateAI();
    void updateControl();
    void updatePhysics();
    void updateTextures();
    void draw();

    sf::RectangleShape& getRect();
    int getHP();
};