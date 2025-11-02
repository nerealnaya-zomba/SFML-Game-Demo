#pragma once
#include <SFML/Graphics.hpp>
#include<TexturesIterHelper.h>
#include<Mounting.h>
#include<GameLoadingScreen.h>
#include<iostream>

class GameTextures
{
public:
    GameTextures(sf::RenderWindow* window,sf::Font* font);
    ~GameTextures();


    
    //Textures
        //Satiro
    std::vector<sf::Texture> idleTextures{};    

    std::vector<sf::Texture> runningTextures{};

    std::vector<sf::Texture> fallingTextures{};

        //Satiro bullet
    std::vector<sf::Texture> bulletTextures{};
    
        //Plants
    std::vector<sf::Texture> plant1Textures;
    texturesIterHelper plant1;

    std::vector<sf::Texture> plant2Textures;
    texturesIterHelper plant2;

    std::vector<sf::Texture> plant3Textures;
    texturesIterHelper plant3;

    std::vector<sf::Texture> plant4Textures;
    texturesIterHelper plant4;

    std::vector<sf::Texture> plant5Textures;
    texturesIterHelper plant5;

    std::vector<sf::Texture> plant6Textures;
    texturesIterHelper plant6;

    std::vector<sf::Texture> plant7Textures;
    texturesIterHelper plant7;

    std::vector<sf::Texture> jumpPlantTextures;
    texturesIterHelper jumpPlant;
        //Cat
    std::vector<sf::Texture> cat1Textures;
    texturesIterHelper catHelper;

private:
    bool initSatiroTextures(std::vector<sf::Texture> &textures, std::vector<std::string> paths);
    void generateMipmapTextures(std::vector<sf::Texture> &texturesArray);
    void smoothTextures(std::vector<sf::Texture> &texturesArray);

    //Variables
        //Logic
    int succesedOperationsCount_m{};
    int operations_count_m = 13; // Change this value always when adding new textures
        //Menu
    LoadingScreen* loadingScreen_m;

    //Textures paths
        //Satiro 
    std::vector<std::string> idleTexturesPaths{
        "images/satiro-idle-1.png",
        "images/satiro-idle-2.png",
        "images/satiro-idle-3.png",
        "images/satiro-idle-4.png",
        "images/satiro-idle-5.png",
        "images/satiro-idle-6.png",
    };
    std::vector<std::string>  runningTexturesPaths{
        "images/satiro-running-1.png",
        "images/satiro-running-2.png",
        "images/satiro-running-3.png",
        "images/satiro-running-4.png",
        "images/satiro-running-5.png",
        "images/satiro-running-6.png",
        "images/satiro-running-7.png",
        "images/satiro-running-8.png",
    };
    std::vector<std::string> fallingTexturesPaths{
        "images/satiro-falling-1.png",
        "images/satiro-falling-2.png",
        "images/satiro-falling-3.png",
        "images/satiro-falling-4.png",
        "images/satiro-falling-5.png",
    };
        //Satiro bullet
    std::vector<std::string> bulletTexturesPaths{
        "images/Bullet/blue-bullet-1.png",
        "images/Bullet/blue-bullet-2.png",
        "images/Bullet/blue-bullet-3.png",
        "images/Bullet/blue-bullet-4.png"
    };
        //Plants
    std::string plant1Path{"images/decoration/Plant_1/Plant1_"};
    std::string plant2Path{"images/decoration/Plant_2/Plant2_"};
    std::string plant3Path{"images/decoration/Plant_3/Plant3_"};
    std::string plant4Path{"images/decoration/Plant_4/Plant4_"};
    std::string plant5Path{"images/decoration/Plant_5/Plant5_"};
    std::string plant6Path{"images/decoration/Plant_6/Plant6_"};
    std::string plant7Path{"images/decoration/Plant_7/Plant7_"};
    std::string jumpPlantPath{"images/decoration/PlantJump/JumpPlant_"};
        //Cat
    std::string cat1Path{"images/decoration/sleepingCat/cat/RetroCatsFree_"};
};