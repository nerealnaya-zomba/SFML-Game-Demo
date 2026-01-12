#pragma once
#include<SFML/Graphics.hpp>
#include<GameData.h>
#include<Player.h>
#include<Mounting.h>
#include<Interactive.h>
#include<Shop.h>
#include<math.h>

bool switchToNextSpritePingPong(sf::Sprite* sprite,
    std::vector<sf::Texture>& texturesArray, 
    texturesIterHelper& iterHelper);

class Shop;

const sf::Keyboard::Scancode keyToOpenShop = sf::Keyboard::Scancode::Enter;
const sf::Vector2f BASE_SHOP_OFFSET = {0.f,-200.f};

class Trader : public InteractiveObject
{
private:
    Player* player;

    std::unique_ptr<Shop> shop;

    std::vector<sf::Texture>* traderTextures;
    texturesIterHelper trader_helper;

    const sf::Event::KeyPressed* keyPressed;

    sf::Clock time;

    void updateTextures();

    void checkIsInInteractionArea();

    void lookAtPlayerSide();

    void ySmoothFloating();

public:
    Trader(GameData& data, Player& p, sf::Vector2f& pos);
    ~Trader() = default;

    void draw(sf::RenderWindow& window)         override;
    void update()                               override;
    void handleEvent(const sf::Event& event)    override;

};

bool switchToNextSpritePingPong(sf::Sprite* sprite,
    std::vector<sf::Texture>& texturesArray, 
    texturesIterHelper& iterHelper)
{
    // Проверка безопасности
    if (!sprite || texturesArray.empty() || iterHelper.countOfTextures <= 0) {
        return false;
    }

    // Проверяем валидность текущего индекса
    if (iterHelper.ptrToTexture >= static_cast<int>(texturesArray.size())) {
        iterHelper.ptrToTexture = 0;
    }
    
    // Проверяем, что текстура по текущему индексу существует
    if (iterHelper.ptrToTexture < 0 || 
        iterHelper.ptrToTexture >= static_cast<int>(texturesArray.size())) {
        return false;
    }

    // Увеличиваем счетчик кадров
    iterHelper.iterationCounter++;
    
    // Проверяем, не пора ли переключить текстуру
    if (iterHelper.iterationCounter < iterHelper.iterationsTillSwitch) {
        return true;
    }
    
    // Сбрасываем счетчик кадров
    iterHelper.iterationCounter = 0;
    
    // Устанавливаем текущую текстуру
    sprite->setTexture(texturesArray[iterHelper.ptrToTexture]);
    
    // PingPong логика
    if (iterHelper.countOfTextures <= 1) {
        return true; // Одна текстура - ничего не делаем
    }
    
    if (iterHelper.goForward) {
        if (iterHelper.ptrToTexture >= iterHelper.countOfTextures - 1) {
            iterHelper.goForward = false;
            // Не выходим за границы
            iterHelper.ptrToTexture = std::max(0, iterHelper.countOfTextures - 2);
        } else {
            iterHelper.ptrToTexture++;
        }
    } else {
        if (iterHelper.ptrToTexture <= 0) {
            iterHelper.goForward = true;
            // Не выходим за границы
            iterHelper.ptrToTexture = std::min(1, iterHelper.countOfTextures - 1);
        } else {
            iterHelper.ptrToTexture--;
        }
    }
    
    // Гарантируем, что индекс в пределах массива текстур
    if (iterHelper.ptrToTexture < 0) {
        iterHelper.ptrToTexture = 0;
    }
    if (iterHelper.ptrToTexture >= static_cast<int>(texturesArray.size())) {
        iterHelper.ptrToTexture = texturesArray.size() - 1;
    }
    
    return true;
}