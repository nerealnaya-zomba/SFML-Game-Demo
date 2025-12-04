#pragma once
#include<SFML/Graphics.hpp>
#include<Particle.h>
#include<Mounting.h>
#include<algorithm>
#include<GameData.h>
#include<vector>
#include<TexturesIterHelper.h>

class Bullet
{
private:
    sf::RectangleShape* bulletRect_;
    std::vector<sf::Texture>* bulletTextures_;
    texturesIterHelper satiro_bullet_helper;
    sf::Sprite* bulletSprite_;
    
public:
    float maxDistance_{100.f};
    float distancePassed{};
    Bullet(sf::Vector2f pos, float maxDistance, GameData& gamedata); //maxDistance changes in constructor
    virtual ~Bullet();

    //Setters
    void setSpriteTexture(sf::Texture& texture);
    void setSpriteScale(sf::Vector2f scale);
    //Getters
    sf::RectangleShape& getBulletRect();

    sf::Vector2f getPosition();
    
    sf::Vector2f offsetToMove_{};

    bool canBeDeleted = false;
    bool isSheduledToBeDestroyed = false;
    bool isMakedDeathParticles = false;

    //Control
    void destroyBullet();

    //Physics
    void moveBullet();
    void update(); 

    //Texture
    void updateTextures();

    //Particles
    std::vector<Particle> particles;
    void makeAfterParticles();
    void makeDeathParticles();
    void updateParticles();
    sf::Clock makeParticles_clock;
    float makeParticles_cooldown = 50; //Milliseconds
    bool makeParticles_isOnCooldown = false;


    void draw(sf::RenderWindow& window);
};


