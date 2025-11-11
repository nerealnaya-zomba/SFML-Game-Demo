#include<Bullet.h>

Bullet::Bullet(sf::Vector2f pos, float maxDistance)
{
    maxDistance_=maxDistance;
    bulletRect_ = new sf::RectangleShape();
    bulletRect_->setFillColor(sf::Color::Green);
    bulletRect_->setSize({30.f,30.f});
    bulletRect_->setPosition(pos);
    bulletTexture_= new sf::Texture("images/Bullet/blue-bullet-1.png");
    bulletSprite_ = new sf::Sprite(*bulletTexture_);
    bulletSprite_->setOrigin(bulletSprite_->getGlobalBounds().getCenter());
    bulletSprite_->setPosition(bulletRect_->getGlobalBounds().getCenter());
}

Bullet::~Bullet()
{
    delete bulletRect_;
    delete bulletSprite_;
    delete bulletTexture_;
}

void Bullet::setSpriteTexture(sf::Texture& texture)
{
    bulletSprite_->setTexture(texture);
}

void Bullet::setSpriteScale(sf::Vector2f scale)
{
    bulletSprite_->setScale(scale);
}

sf::RectangleShape &Bullet::getBulletRect()
{
    return *this->bulletRect_;
}

sf::Vector2f Bullet::getPosition()
{
    return bulletRect_->getPosition();
}

void Bullet::moveBullet()
{
    distancePassed+=std::abs(offsetToMove_.x);
    bulletRect_->move(offsetToMove_);
    bulletSprite_->move(offsetToMove_);
}

void Bullet::update()
{
    
}

void Bullet::draw(sf::RenderWindow &window)
{
    //window.draw(*bulletRect_);
    window.draw(*bulletSprite_);
}

