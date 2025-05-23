#pragma once
#include<SFML/Graphics.hpp>
#include<map>
#include<iostream>
#include<Defines.h>
#include<Mounting.h>

class Entity : public sf::Drawable
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

    //Textures init-drawing
    void initTextures(std::vector<std::string> paths);

        //texture_first_name - Enter first texture name of texture's collection
    void switchToNextSprite(std::string texture_first_name);

    std::map< std::string,std::vector<sf::Texture> > textures_;

    sf::Texture* errorTexture_;
    sf::RectangleShape* entityRect_;
    sf::Sprite* entitySprite_;
    
    //values for debugging
    std::vector<std::string> idleTexturesPaths_{
        "images/satiro-idle-1.png",
        "images/satiro-idle-2.png",
        "images/satiro-idle-3.png",
        "images/satiro-idle-4.png",
        "images/satiro-idle-5.png",
        "images/satiro-idle-6.png",
    };

    



public:

    //Physics methods
    void updatePhysics();
    void checkRectCollision(std::vector<sf::RectangleShape*> rects);
    void checkGroundCollision(sf::RectangleShape& groundRect);
    void applyFriction(float& walkSpeed, float friction);

    Entity();
    virtual ~Entity();

    void updateTextures();

protected:
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
};

