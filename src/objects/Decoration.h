#pragma once
#include<SFML/Graphics.hpp>
#include<vector>
#include<iostream>
#include<string>

class Decoration
{
    public:
    Decoration();
    ~Decoration();

    
    private:
    std::vector<std::string> paths{};
    std::vector<sf::Texture> textures;
    
    void initTextures(std::vector<sf::Texture> &textures, std::vector<std::string> paths);

};