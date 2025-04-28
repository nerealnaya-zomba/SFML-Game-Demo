#pragma once
#include<SFML/Graphics.hpp>
#include<vector>
#include<string>

class Player{
    //Variables
    std::vector<sf::Texture> idleTextures{};    


    Player();
    ~Player();

    //Fill vector
    
    void initTextures(std::vector<sf::Texture> textures, std::vector<std::string> paths);






};