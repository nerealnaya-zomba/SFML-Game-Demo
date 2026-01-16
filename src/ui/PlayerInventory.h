#pragma once
#include<SFML/Graphics.hpp>
#include<Shop.h>
#include<Item.h>
#include<Player.h>


class PlayerInventory{
private:
    Item::Stats additionalPlayerStats;

    std::vector<std::unique_ptr<Item>> items;
public:

    int getHealth();
    int getBulletSpeed();
    int getBulletDistance();
    int getShootSpeedCooldownReduction();
    int getInitialSpeed();
    int getMaxSpeed();
    int getDamage();

    void addItem(std::unique_ptr<Item>&& item);

    PlayerInventory();
    ~PlayerInventory() = default;
};