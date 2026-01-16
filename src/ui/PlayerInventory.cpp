#include<PlayerInventory.h>

PlayerInventory::PlayerInventory()
{
}

int PlayerInventory::getHealth()
{
    return 0;
}

int PlayerInventory::getBulletSpeed()
{
    return 0;
}

int PlayerInventory::getBulletDistance()
{
    return 0;
}

int PlayerInventory::getShootSpeedCooldownReduction()
{
    return 0;
}

int PlayerInventory::getInitialSpeed()
{
    return 0;
}

int PlayerInventory::getMaxSpeed()
{
    return 0;
}

int PlayerInventory::getDamage()
{
    return 0;
}

void PlayerInventory::addItem(std::unique_ptr<Item>&& item)
{
    items.push_back(std::move(item));
}