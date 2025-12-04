#pragma once
#include <SFML/Graphics.hpp>
#include <Particle.h>
#include <Mounting.h>
#include <algorithm>
#include <GameData.h>
#include <vector>
#include <TexturesIterHelper.h>

class Bullet
{
private:
    // Graphics components
    sf::RectangleShape* bulletRect_;           // Collision rectangle (hitbox)
    std::vector<sf::Texture>* bulletTextures_; // Animation texture frames
    texturesIterHelper satiro_bullet_helper;   // Animation control helper
    sf::Sprite* bulletSprite_;                 // Visual representation
    
public:
    // Bullet properties
    float maxDistance_{100.f};                 // Maximum travel distance
    float distancePassed{};                    // Current distance traveled
    
    // State flags
    bool canBeDeleted = false;                 // Safe to remove from memory
    bool isSheduledToBeDestroyed = false;      // Marked for destruction (hit something)
    bool isMakedDeathParticles = false;        // Death particles already created
    
    // Movement
    sf::Vector2f offsetToMove_{};              // Movement per frame
    
    // Particle system
    std::vector<Particle> particles;           // Visual effect particles
    sf::Clock makeParticles_clock;             // Timer for particle cooldown
    float makeParticles_cooldown = 50;         // Cooldown in milliseconds
    bool makeParticles_isOnCooldown = false;   // Cooldown state

    // Constructor & Destructor
    Bullet(sf::Vector2f startPosition, float maxDistance, GameData& gamedata);
    virtual ~Bullet();

    // Setters
    void setSpriteTexture(sf::Texture& texture); // Change sprite texture
    void setSpriteScale(sf::Vector2f scale);     // Change sprite scale

    // Getters
    sf::RectangleShape& getBulletRect();         // Get collision rectangle
    sf::Vector2f getPosition();                  // Get current position

    // Physics & movement
    void moveBullet();                           // Update bullet position
    void update();                               // Main update method (physics + particles)

    // Animation
    void updateTextures();                       // Update sprite animation frames

    // Particle effects
    void makeAfterParticles();                   // Create trail particles
    void makeDeathParticles();                   // Create explosion particles on death
    void updateParticles();                      // Update all particles

    // Cleanup
    void destroyBullet();                        // Prepare bullet for deletion //NOTE Useless method. Does nothing.

    // Rendering
    void draw(sf::RenderWindow& window);         // Draw bullet and particles
};