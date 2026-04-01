#pragma once

#include "Enemy.h"
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <VisualEffects.h>
#include <Particle.h>
#include <SFML/System.hpp>
#include <HealthBar.h>
#include <math.h>
#include <enemyPortal.h>
#include <memory>

class GameLevel;
class EnemyManager;
class Bullet;

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

enum class SkeletonAwarenessState
{
    Patrol,
    Alert,
    Search
};

class Skeleton : public Enemy {
private:
    struct VisualRing
    {
        sf::Vector2f position{};
        sf::Color color{255, 255, 255, 255};
        float radius = 8.f;
        float maxRadius = 48.f;
        float growth = 2.f;
        float thickness = 2.f;
        float alpha = 180.f;
    };

    // Внешние ссылки
    sf::RenderWindow* window;
    Ground* ground_;
    Platform* platform_;  
    Player* player_;
    GameLevel* gameLevel;
    GameData* gameData;
    EnemyManager* enemyManager = nullptr;

    // Портал
    std::unique_ptr<enemyPortal> portal;

    // Основные свойства
    std::string type_;              // "white" или "yellow"
    std::unique_ptr<HealthBar> healthbar;
    
    // Флаги состояний
    bool isIdle = true;
    bool isFalling = true;
    bool isPlayingHurtAnimation = false;
    bool isPlayingDieAnimation = false;
    bool isPlayerOutOfReach = false;
    bool hasDroppedGold = false;
    bool hasDetectedPlayer = false;
    bool hasLastKnownPlayerPos = false;
    bool attackDamageApplied = false;
    bool deathEffectPlayed = false;
    bool isPatrolPaused = false;
    bool hasBlockedChaseLeft_ = false;
    bool hasBlockedChaseRight_ = false;
    
    skeletonAction action_ = IDLE;
    skeletonAction pendingPatrolAction_ = WALKRIGHT;
    SkeletonAwarenessState awarenessState_ = SkeletonAwarenessState::Patrol;

    // Физические параметры
    float fallingSpeed = 0.f;
    float initialWalkSpeed = 0.f;
    float speed;
    float maxWalkSpeed;
    float baseMaxWalkSpeed = 0.f;
    float frictionForce;
    int HP_;
    int DMG_;
    sf::Vector2f knockback_;
    float distanceToMakeAttack;
    float distanceToHit_byAttack;
    bool knockbacks;
    sf::Vector2f enemyPos;
    sf::Vector2f lastKnownPlayerPos{};
    float blockedChaseLeftX_ = 0.f;
    float blockedChaseRightX_ = 0.f;

    // Визуальные эффекты
    std::vector<Particle> effectParticles_;
    std::vector<VisualRing> effectRings_;

    // Графика
    sf::Vector2f enemyScale_;
    std::unique_ptr<sf::Sprite> skeletonSprite;
    std::unique_ptr<sf::RectangleShape> skeletonRect;

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
    void resetAllThatHeKnows();
    void clearAggroState();
    void registerBlockedChaseBoundary(bool blockedLeft);
    void updateBlockedChaseBoundaries(float playerX);
    bool isPlayerBeyondBlockedChaseBoundary(float playerX) const;
    
    // Система патрулирования
    enum PatrolState { PATROL_EXPLORING_LEFT, PATROL_EXPLORING_RIGHT, PATROL_PATROLLING };
    enum ExplorationState { EXPLORE_NONE, EXPLORE_LEFT, EXPLORE_RIGHT, PATROLLING };
    
    ExplorationState explorationState = EXPLORE_NONE;
    sf::Clock deadEndCheckTimer;
    sf::Clock directionSwitchTimer;
    sf::Clock patrolPauseTimer;
    sf::Clock AFKTimeTimer;                     // Отсчитывает время, сколько скелет находится в AFK
    sf::Clock AFKPastPosUpdateTimer;            // Нужен для обновления прошлой позиции скелета
    sf::Clock blackoutTimer;                    // Таймер для амнезии
    sf::Clock aggroMemoryClock;
    sf::Clock attackCooldownClock;
    sf::Clock patrolEffectClock;
    sf::Clock deathSmokeClock;
    sf::Clock alertPulseClock;
    
    float exploreStartPos           = 0.0f;
    float leftBound                 = 0.0f;
    float rightBound                = 0.0f;
    bool leftExplored               = false;
    bool rightExplored              = false;
    bool recentlySwitchedDirection  = false;
    bool isFirstEnter               = true;
    bool makeRandomStart            = false;
    float currentSkeletonPos        = 0.0f;
    float afk_past_pos              = 0.0f;     // Значение обновляемое каждые AFK_BEFORE_UPDATE_TIME миллисекунд
    float afk_current_pos           = 0.0f;     // Значение обновляемое каждый кадр
    float afk_detect_difference     = 2.0f;     // Разница которая должна быть между afk_past_pos и afk_current_pos чтобы НЕ заметить что скелет в АФК // NOTE 2.f хз почему, просто так. но Много лучше не ставить, и слишком мало тож

    
    // Константы ИИ
    const float TIME_TO_CHECK_DEADEND       = 300.0f;
    const float MIN_DISTANCE_FOR_DEADEND    = 10.0f;
    const float DIRECTION_SWITCH_COOLDOWN   = 300.0f;
    const float DIRECTION_SWITCH_OFFSET     = 100.0f;
    const float PATROL_SWITCH_DELAY         = 1000.0f;
    const float PATROL_EDGE_PAUSE           = 420.0f;

    const float MAX_AFK_TIME                = 1400.0f;
    const float AFK_BEFORE_UPDATE_TIME      = 200.f;
    const float timeToResetALLThatHeKnows   = 10000.f;  // Стереть все, что знает скелет через это время. Нужно чтобы скелет не контролировал маленький участок территории всю жизнь

    const float ALERT_SPEED_MULTIPLIER      = 1.22f;
    const float ATTACK_COOLDOWN_MS          = 1050.f;
    const float AGGRO_MEMORY_MS             = 2600.f;
    const float PATROL_EFFECT_INTERVAL_MS   = 180.f;
    const float BLOCKED_CHASE_RELEASE_MARGIN = 28.f;

    float alertDistance_                    = 0.f;
    float loseAggroDistance_                = 0.f;
    float verticalAlertTolerance_           = 0.f;
    float verticalAttackTolerance_          = 0.f;

    // Таймеры
    sf::Clock isPlayerOutOfReachClock;

    // Атака
    void tryAttackPlayer();
    void onBulletHit(const Bullet& bullet, bool splashHit = false);
    void beginAttack(skeletonAction attackAction);
    void updateVisualEffects();
    void drawVisualEffects();
    void drawAttackTelegraph();
    void spawnPatrolEffect();
    void spawnNoticeEffect();
    void spawnAttackEffect(bool heavyAttack);
    void spawnAttackImpactEffect();
    void spawnDeathEffect();
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
    void pushRing(
        const sf::Vector2f& position,
        const sf::Color& color,
        float radius,
        float maxRadius,
        float growth,
        float thickness,
        float alpha
    );
    sf::Vector2f getCenterPosition() const;
    float getFacingDirection() const;

    // Физика и коллизии
    void checkGroundCollision(Ground& ground);
    void checkPlatformCollision(Platform& platforms);
    void checkBulletCollision(Player& player);
    void applyFriction(float& walkSpeed, float friction);

    // Данные
    void loadData();

public:
    Skeleton(GameData &gameData, EnemyManager& em, GameLevel& gl, sf::RenderWindow &window, Ground& ground, 
             Platform& platform, Player& player, std::string type, sf::Vector2f pos);
    ~Skeleton();

    bool isAlive = true;
    
    void updateAI();            //FIXME Работает как то глупо. AIшка какашку написала, нужно вручную.
    void updateControl();
    void updatePhysics();
    void updateTextures();
    void draw();

    // Getters
    sf::RectangleShape& getRect();
    int getHP();
    sf::Vector2f getPosition();

    //Setters
    void attachPlayer(Player& p);
};
