#pragma once
#include<SFML/Graphics.hpp>
#include<vector>
#include<iostream>
#include<string>
#include<Mounting.h>

struct texturesIterHelper
{   
    int iterationCounter{};
    int iterationsTillSwitch{};
    //Points element of textures array, that needs to be assigned to all sprites; 
    int ptrToTexture{};
    //Needed for forward-backward iterations based on goForward
    int countOfTextures{};
    //If countOfTextures == texturesArr.size(), then goForward == false
    //Is used for correct displaying sprites textures(forward-backwards loop)
    bool goForward = true;
};

class Decoration
{
    public:
    Decoration();
    ~Decoration();

    void addDecoration(std::string name,sf::Vector2f position, sf::Vector2f scale, sf::Color color=sf::Color::White);
    void updateTextures();
    void draw(sf::RenderWindow& window);

    private:
    //Points element of textures array, that needs to be assigned to all sprites;  

    //Textures
        //Plants
    std::string plant1Path{"images/decoration/Plant_1/Plant1_"};
    std::string plant2Path{"images/decoration/Plant_2/Plant2_"};
    std::string plant3Path{"images/decoration/Plant_3/Plant3_"};
    std::string plant4Path{"images/decoration/Plant_4/Plant4_"};
    std::string plant5Path{"images/decoration/Plant_5/Plant5_"};
    std::string plant6Path{"images/decoration/Plant_6/Plant6_"};
    std::string plant7Path{"images/decoration/Plant_7/Plant7_"};
    std::vector<sf::Texture> plant1Textures;
    std::vector<sf::Texture> plant2Textures;
    std::vector<sf::Texture> plant3Textures;
    std::vector<sf::Texture> plant4Textures;
    std::vector<sf::Texture> plant5Textures;
    std::vector<sf::Texture> plant6Textures;
    std::vector<sf::Texture> plant7Textures;
    
    std::vector<std::unique_ptr<sf::Sprite>> plant1Sprites;
    std::vector<std::unique_ptr<sf::Sprite>> plant2Sprites;
    std::vector<std::unique_ptr<sf::Sprite>> plant3Sprites;
    std::vector<std::unique_ptr<sf::Sprite>> plant4Sprites;
    std::vector<std::unique_ptr<sf::Sprite>> plant5Sprites;
    std::vector<std::unique_ptr<sf::Sprite>> plant6Sprites;
    std::vector<std::unique_ptr<sf::Sprite>> plant7Sprites;
        //Cat
    std::string cat1Path{"images/decoration/sleepingCat/cat/RetroCatsFree_"};
    std::vector<sf::Texture> cat1Textures;
    std::vector<std::unique_ptr<sf::Sprite>> cat1Sprites;


    void switchToNextSprite(std::vector<std::unique_ptr<sf::Sprite>>& spritesArray, std::vector<sf::Texture>& texturesArray, texturesIterHelper& iterHelper);

    //Optional
    void generateMipmapTextures(std::vector<sf::Texture>& texturesArray);
    void smoothTextures(std::vector<sf::Texture>& texturesArray);

    //IterHelper
        //Plant
    texturesIterHelper plant1;
    texturesIterHelper plant2;
    texturesIterHelper plant3;
    texturesIterHelper plant4;
    texturesIterHelper plant5;
    texturesIterHelper plant6;
    texturesIterHelper plant7;
        //Cat
    texturesIterHelper catHelper;
};      

