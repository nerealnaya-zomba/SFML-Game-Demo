#pragma once
#include<SFML/Graphics.hpp>
#include<Item.h>
#include<vector>

class Shop
{
private:
    std::vector<std::shared_ptr<Item>> items;

    
    
public:

    void update();
    void draw();
    
}; 