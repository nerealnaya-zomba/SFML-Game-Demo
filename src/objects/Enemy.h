#pragma once
#include<SFML/Graphics.hpp>
#include<map>
#include<iostream>
#include<Defines.h>
#include<Mounting.h>

class Enemy
{
private:
    //Logic variables
    bool isIdle_ = true;
    bool isFalling_ = true;
    float fallingSpeed_ = 0.f;
    float initialWalkSpeed_ = 0.f;
    float speed_ = 0.15f;
    float maxWalkSpeed_ = 4.f;
    float frictionForce_ = 0.1f;

    //Control methods
    void walkLeft();
    void walkRight();
    void jump();
    void fallDown();

    //Textures init-drawing
    void initTextures(std::vector<std::string> paths);

        //texture_first_name - Enter first texture name of texture's collection
    void switchToNextSprite(std::string texture_first_name);

    std::map< std::string,std::vector<sf::Texture> > textures_;

    sf::Texture* errorTexture_;
    sf::RectangleShape* entityRect_;
    sf::Sprite* entitySprite_;
    
    
    

    //PRIVATE Physics methods
    void applyFriction(float& walkSpeed, float friction);

public:
    Enemy();
    virtual ~Enemy();

    //PUBLIC Physics methods
    void updatePhysics();
    void checkRectCollision(std::vector<sf::RectangleShape*> rects);
    void checkGroundCollision(sf::RectangleShape& groundRect);
    
    //PUBLIC Texture methods 
        //By default should contain switchToNextSprite method/methods
    void updateTextures();

    //PUBLIC draw methods
    void draw(sf::RenderWindow& window);


};

