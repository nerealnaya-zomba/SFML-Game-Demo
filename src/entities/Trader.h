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

const sf::Keyboard::Scancode keyToOpenShop          = sf::Keyboard::Scancode::Enter;
const sf::Vector2f BASE_SHOP_OFFSET                 = {0.f,-200.f};

class Trader : public InteractiveObject
{
private:
    Player* player;

    std::unique_ptr<Shop> shop;

    std::vector<sf::Texture>* traderTextures;
    texturesIterHelper trader_helper;

    sf::Clock time;
    sf::RectangleShape dialogueShadow;
    sf::RectangleShape dialogueBack;
    sf::RectangleShape dialogueAccent;
    sf::Text dialogueTitleText;
    sf::Text dialogueBodyText;
    bool shouldDrawDialogue_ = false;

    void updateTextures();

    void checkIsInInteractionArea();

    void lookAtPlayerSide();

    void ySmoothFloating();
    void updateDialogue();

public:
    Trader(GameData& data, Player& p, sf::Vector2f& pos);
    ~Trader() = default;

    void draw(sf::RenderWindow& window)         override;
    void drawOverlay(sf::RenderWindow& window)  override;
    void update()                               override;
    bool handleEvent(const sf::Event& event)    override;
    bool blocksPlayerInput() const              override;

    //Getters
    bool isShopOpened();
    
};
