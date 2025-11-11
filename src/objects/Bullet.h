#pragma once
#include<SFML/Graphics.hpp>

class Bullet
{
private:
    sf::RectangleShape* bulletRect_;
    sf::Texture* bulletTexture_;
    sf::Sprite* bulletSprite_;
    
public:
    float maxDistance_{100.f};
    float distancePassed{};
    Bullet(sf::Vector2f pos, float maxDistance); //maxDistance changes in constructor
    virtual ~Bullet();

    //Setters
    void setSpriteTexture(sf::Texture& texture);
    void setSpriteScale(sf::Vector2f scale);
    //Getters
    sf::RectangleShape& getBulletRect();

    sf::Vector2f getPosition();
    
    sf::Vector2f offsetToMove_{};

    void moveBullet();

    void update();


    void draw(sf::RenderWindow& window);
};


