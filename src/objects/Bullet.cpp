#include<Bullet.h>

void Bullet::setOffSetToMove(sf::Vector2f offset)
{
    this->offsetToMove_ = offset;
    this->maxReduction = 1.f;
}

Bullet::Bullet(sf::Vector2f pos, float maxDistance, GameData &gamedata)
{
    maxDistance_=maxDistance;
    bulletRect_ = new sf::RectangleShape();
    bulletRect_->setFillColor(sf::Color::Green);
    bulletRect_->setSize({30.f,30.f});
    bulletRect_->setPosition(pos);
    attachTexture(gamedata.bulletTextures,this->bulletTextures_,gamedata.satiro_bullet_helper,this->satiro_bullet_helper);
    bulletSprite_ = new sf::Sprite(bulletTextures_->at(0));
    bulletSprite_->setOrigin(bulletSprite_->getGlobalBounds().getCenter());
    bulletSprite_->setPosition(bulletRect_->getGlobalBounds().getCenter());
}

Bullet::~Bullet()
{
    delete bulletRect_;
    delete bulletSprite_;
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

void Bullet::destroyBullet()
{
}

void Bullet::moveBullet()
{
    if(isSheduledToBeDestroyed) return;

    if(distancePassed>=(maxDistance_/4)*2) speedReduction();

    distancePassed+=std::abs(offsetToMove_.x);
    bulletRect_->move(offsetToMove_);
    bulletSprite_->move(offsetToMove_);
}

void Bullet::speedReduction()
{
    if(offsetToMove_.x > maxReduction) {
        offsetToMove_.x -= speedReductionValue;
        if(offsetToMove_.x < maxReduction) offsetToMove_.x = maxReduction;
    }
    else if(offsetToMove_.x < -maxReduction) {
        offsetToMove_.x += speedReductionValue;
        if(offsetToMove_.x > -maxReduction) offsetToMove_.x = -maxReduction;
    }
}

void Bullet::update()
{
    moveBullet();

    //Остаточные частицы
    if(!makeParticles_isOnCooldown){   
        makeAfterParticles();
        makeParticles_isOnCooldown = true;
        makeParticles_clock.restart();
    } else if(makeParticles_clock.getElapsedTime().asMilliseconds() >= makeParticles_cooldown && !isSheduledToBeDestroyed){
        makeParticles_isOnCooldown = false;
        makeParticles_clock.stop();
    }
    
    //Создаем частицы после смерти только единожды
    if(!isMakedDeathParticles && isSheduledToBeDestroyed){
        makeDeathParticles();
        isMakedDeathParticles = true;       
    }

    //Помечаем если пролетела максимальное расстояние
    if(distancePassed>=maxDistance_ && !isSheduledToBeDestroyed)
    {
        isSheduledToBeDestroyed = true;
    }

    updateParticles();

    if(particles.size() == 0) canBeDeleted = true;
}

void Bullet::updateTextures()
{
    if(!isSheduledToBeDestroyed) switchToNextSprite(this->bulletSprite_,*this->bulletTextures_,satiro_bullet_helper,switchSprite_SwitchOption::Loop);
}

void Bullet::makeAfterParticles()
{
    sf::Vector2f bulletPos = this->bulletRect_->getGlobalBounds().getCenter();

    for (int i = 0; i < 5; i++) {
        sf::Color particleColor = sf::Color(127, 255, 212);
        float playerRectDownSide = bulletPos.y+(bulletRect_->getSize().y/2);
        float playerRectUpSide = bulletPos.y-(bulletRect_->getSize().y/2);
        float particleSpeed = random(-150,150);
        float particleSize = 1.f;
        float particleGravity = 0.f;
        float accelerationDamping = 0.8f;
        float particleLifeTime = 0.5f;

        particles.emplace_back(
        sf::Vector2f(bulletPos.x,random(playerRectUpSide,playerRectDownSide)),
        sf::Vector2f(particleSpeed, 0.f),
        sf::Vector2f(random(-60,60), random(-60,60)),
        sf::Color(particleColor),
        particleSize,
        particleGravity,
        accelerationDamping,
        particleLifeTime
        );
    }
}

void Bullet::makeDeathParticles()
{
    sf::Vector2f bulletPos = this->bulletRect_->getPosition();
    sf::Color explodeColor = {127, 255, 212};
    for (int i = 0; i < 40; i++) {
        // Угол от 0 до 360 градусов
        float angle = random(0.f, 360.f) * 3.14159f / 180.f;
        float speed = random(50.f, 150.f);
        
        sf::Vector2f velocity(
            std::cos(angle) * speed,
            std::sin(angle) * speed
        );
        
        // Случайная позиция в пределах пули
        float x = random(bulletPos.x, bulletPos.x + bulletRect_->getSize().x);
        float y = random(bulletPos.y, bulletPos.y + bulletRect_->getSize().y);
        
        // Случайное ускорение
        float accel = random(-30.f, 30.f);
        
        particles.emplace_back(
            sf::Vector2f(x, y),
            velocity,
            sf::Vector2f(random(-accel, accel), random(-accel, accel)),
            explodeColor,
            1.f,
            0.f,
            0.8f,
            1.f
        );
    }
}

void Bullet::updateParticles()
{
    for (auto& particle : particles) {
        particle.update();
    }
    particles.erase(
        std::remove_if(particles.begin(), particles.end(),
            [](const Particle& p) { return !p.getIsAlive(); }),
        particles.end()
    );
}

void Bullet::draw(sf::RenderWindow &window)
{
    //window.draw(*bulletRect_);
    if(!isSheduledToBeDestroyed) window.draw(*bulletSprite_);
    for (auto& particle : particles) {
        particle.draw(window);
    }
    
}

