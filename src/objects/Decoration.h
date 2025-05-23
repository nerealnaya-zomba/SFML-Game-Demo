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
    //Textures
        //Plants
    std::vector<sf::Texture> plant1Textures;
    std::string plant1Path{"images/decoration/Plant_1/Plant1_"};
    std::vector<std::unique_ptr<sf::Sprite>> plant1Sprites;
    texturesIterHelper plant1;
    std::vector<sf::Texture> plant2Textures;
    std::string plant2Path{"images/decoration/Plant_2/Plant2_"};
    std::vector<std::unique_ptr<sf::Sprite>> plant2Sprites;
    texturesIterHelper plant2;
    std::vector<sf::Texture> plant3Textures;
    std::string plant3Path{"images/decoration/Plant_3/Plant3_"};
    std::vector<std::unique_ptr<sf::Sprite>> plant3Sprites;
    texturesIterHelper plant3;
    std::vector<sf::Texture> plant4Textures;
    std::string plant4Path{"images/decoration/Plant_4/Plant4_"};
    std::vector<std::unique_ptr<sf::Sprite>> plant4Sprites;
    texturesIterHelper plant4;
    std::vector<sf::Texture> plant5Textures;
    std::string plant5Path{"images/decoration/Plant_5/Plant5_"};
    std::vector<std::unique_ptr<sf::Sprite>> plant5Sprites;
    texturesIterHelper plant5;
    std::vector<sf::Texture> plant6Textures;
    std::string plant6Path{"images/decoration/Plant_6/Plant6_"};
    std::vector<std::unique_ptr<sf::Sprite>> plant6Sprites;
    texturesIterHelper plant6;
    std::vector<sf::Texture> plant7Textures;
    std::string plant7Path{"images/decoration/Plant_7/Plant7_"};
    std::vector<std::unique_ptr<sf::Sprite>> plant7Sprites;
    texturesIterHelper plant7;
    std::vector<sf::Texture> jumpPlantTextures;
    std::string jumpPlantPath{"images/decoration/PlantJump/JumpPlant_"};
    std::vector<std::unique_ptr<sf::Sprite>> jumpPlantSprites;
    texturesIterHelper jumpPlant;
        //Cat
    std::string cat1Path{"images/decoration/sleepingCat/cat/RetroCatsFree_"};
    std::vector<sf::Texture> cat1Textures;
    std::vector<std::unique_ptr<sf::Sprite>> cat1Sprites;
    texturesIterHelper catHelper;

    //Switches sprite's texture to next
    void switchToNextSprite(std::vector<std::unique_ptr<sf::Sprite>>& spritesArray, std::vector<sf::Texture>& texturesArray, texturesIterHelper& iterHelper);

    //Optional
    void generateMipmapTextures(std::vector<sf::Texture>& texturesArray);
    void smoothTextures(std::vector<sf::Texture>& texturesArray);

};      

