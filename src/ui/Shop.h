#pragma once
#include<SFML/Graphics.hpp>
#include<Item.h>
#include<Player.h>
#include<GameData.h>
#include<vector>
#include<Interactive.h>

class Shop : InteractiveObject
{
private:
    Player* player;
    GameData* data;

    std::vector<std::shared_ptr<Item>> items;

    bool isOpened;
    
public:
    Shop(GameData& d, Player& p, sf::Vector2f pos);
    ~Shop() = default;
    void open();
    void close();
    

    void draw(sf::RenderWindow& window)         override;
    void update()                               override;
    void handleEvent(const sf::Event& event)    override;
}; 