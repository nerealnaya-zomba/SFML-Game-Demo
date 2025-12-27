#pragma once
#include <SFML/Graphics.hpp>
#include <TexturesIterHelper.h>
#include <Mounting.h>
#include <GameLoadingScreen.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#include <map>

// Central resource manager for textures and game data
////////////////////////////////////////////////////// NOTE Гайд по загрузке текстур
// Как загрузить текстуру: 
// Если нужен texturesIterHelper(для зацикленного переключения текстур):
//  — Создать std::string{<название_первой_текстуры>_} путь
//  — Создать std::vector<sf::Texture> и название
//  — Создать texturesIterHelper
//  — В конструкторе дописать loadTexture(<std::vector<sf::Textures>>, <std::string>, texturesIterHelper , <ЗадержкаДоПереключенияНаСледующуюТекстуру>)
// 
// Если нужен вектор со статичными текстурами, и ключами доступа по их названию в директории:
//  — Создать std::string{<название_первой_текстуры>_}
//  — Создать std::map<std::string,sf::Texture> и название
//  — В конструкторе дописать loadTexture(<std::vector<sf::Textures>> название, <std::string> путь)
//////////////////////////////////////////////////////
class GameData
{
public:
    GameData(sf::RenderWindow* window, sf::Font* font);
    ~GameData();

    // Public texture collections

    // Player (Satiro) textures
    std::vector<sf::Texture> idleTextures{};            // Idle animation frames
    std::vector<sf::Texture> runningTextures{};         // Running animation frames
    std::vector<sf::Texture> fallingTextures{};         // Falling animation frames
    std::vector<sf::Texture> bulletTextures{};          // Bullet animation frames
    texturesIterHelper satiro_bullet_helper;
    std::vector<sf::Texture> satiro_dieTextures{};      // Bullet animation frames
    texturesIterHelper satiro_die_helper;
    std::vector<sf::Texture> satiro_dashTextures{};     // Bullet animation frames
    texturesIterHelper satiro_dash_helper;
    std::vector<sf::Texture> satiro_hurtTextures{};     // Bullet animation frames
    texturesIterHelper satiro_hurt_helper;
    std::vector<sf::Texture> satiro_slideTextures{};    // Bullet animation frames
    texturesIterHelper satiro_slide_helper;

    // White skeleton textures with animation helpers
    std::vector<sf::Texture> skeletonWhite_idleTextures_{};
    texturesIterHelper skeletonWhite_idle_helper;
    std::vector<sf::Texture> skeletonWhite_walkTextures{};
    texturesIterHelper skeletonWhite_walk_helper;
    std::vector<sf::Texture> skeletonWhite_hurtTextures{};
    texturesIterHelper skeletonWhite_hurt_helper;
    std::vector<sf::Texture> skeletonWhite_dieTextures{};
    texturesIterHelper skeletonWhite_die_helper;
    std::vector<sf::Texture> skeletonWhite_attack1Textures{};
    texturesIterHelper skeletonWhite_attack1_helper;
    std::vector<sf::Texture> skeletonWhite_attack2Textures{};
    texturesIterHelper skeletonWhite_attack2_helper;

    // Yellow skeleton textures with animation helpers
    std::vector<sf::Texture> skeletonYellow_idleTextures{};
    texturesIterHelper skeletonYellow_idle_helper;
    std::vector<sf::Texture> skeletonYellow_walkTextures{};
    texturesIterHelper skeletonYellow_walk_helper;
    std::vector<sf::Texture> skeletonYellow_hurtTextures{};
    texturesIterHelper skeletonYellow_hurt_helper;
    std::vector<sf::Texture> skeletonYellow_dieTextures{};
    texturesIterHelper skeletonYellow_die_helper;
    std::vector<sf::Texture> skeletonYellow_attack1Textures{};
    texturesIterHelper skeletonYellow_attack1_helper;
    std::vector<sf::Texture> skeletonYellow_attack2Textures{};
    texturesIterHelper skeletonYellow_attack2_helper;

    // Decoration textures - plants
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

    // Cat decoration
    std::vector<sf::Texture> cat1Textures;
    texturesIterHelper catHelper;

    // Ground texture
    sf::Texture mramoric;
    std::map<std::string,sf::Texture> TileSetGreenTextures;

private:
    // Texture loading and processing
    int standartIterationsTillSwitch = 7;
    bool initSatiroTextures(std::vector<sf::Texture>& textures, std::vector<std::string> paths);
    bool loadTexture(std::vector<sf::Texture> &textures,std::string path, texturesIterHelper& helper, int pauseTillSwitch);
    bool loadTexture(std::map<std::string,sf::Texture> &textures, std::string path);
    void generateMipmapTextures(std::vector<sf::Texture>& texturesArray);
    void smoothTextures(std::vector<sf::Texture>& texturesArray);

    // Loading system variables
    int succesedOperationsCount_m{};    // Tracks completed loading operations
    int allOperations_count_m{};        // Total operations from launchSettings.json
    LoadingScreen* loadingScreen_m;     // Loading screen display

    // Data persistence
    void saveOperationsData();
    void loadData();

    // Texture file paths
    
    // Player (Satiro) texture paths
    std::vector<std::string> idleTexturesPaths{
        "images/satiro-idle-1.png",
        "images/satiro-idle-2.png",
        "images/satiro-idle-3.png",
        "images/satiro-idle-4.png",
        "images/satiro-idle-5.png",
        "images/satiro-idle-6.png",
    };
    std::vector<std::string> runningTexturesPaths{
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
    std::string bulletTexturesPath_{"images/Bullet/blue-bullet-"};
    std::string satiro_diePath_{"images/satiro-die_"};
    std::string satiro_dashPath_{"images/satiro-dash_"};
    std::string satiro_hurtPath_{"images/satiro-hurt_"};
    std::string satiro_slidePath_{"images/satiro-slide_"};

    // White skeleton texture paths
    std::string skeletonWhite_idlePath_{"images/creatures/enemies/skeleton_white/idle/skeleton-idle_"};
    std::string skeletonWhite_walkPath_{"images/creatures/enemies/skeleton_white/walk/Skeleton_White_Walk_"};
    std::string skeletonWhite_hurtPath_{"images/creatures/enemies/skeleton_white/hurt/Skeleton_White_Hurt_"};
    std::string skeletonWhite_diePath_{"images/creatures/enemies/skeleton_white/die/Skeleton_White_Die_"};
    std::string skeletonWhite_attack1Path_{"images/creatures/enemies/skeleton_white/attack1/Skeleton_White_Attack1_"};
    std::string skeletonWhite_attack2Path_{"images/creatures/enemies/skeleton_white/attack2/Skeleton_White_Attack2_"};

    // Yellow skeleton texture paths
    std::string skeletonYellow_idlePath_{"images/creatures/enemies/skeleton_yellow/idle/Skeleton_Yellow_Idle_"};
    std::string skeletonYellow_walkPath_{"images/creatures/enemies/skeleton_yellow/walk/Skeleton_Yellow_Walk_"};
    std::string skeletonYellow_hurtPath_{"images/creatures/enemies/skeleton_yellow/hurt/Skeleton_Yellow_Hurt_"};
    std::string skeletonYellow_diePath_{"images/creatures/enemies/skeleton_yellow/die/Skeleton_Yellow_Die_"};
    std::string skeletonYellow_attack1Path_{"images/creatures/enemies/skeleton_yellow/attack1/Skeleton_Yellow_Attack1_"};
    std::string skeletonYellow_attack2Path_{"images/creatures/enemies/skeleton_yellow/attack2/Skeleton_Yellow_Attack2_"};

    // Decoration texture paths
    std::string plant1Path{"images/decoration/Plant_1/Plant1_"};
    std::string plant2Path{"images/decoration/Plant_2/Plant2_"};
    std::string plant3Path{"images/decoration/Plant_3/Plant3_"};
    std::string plant4Path{"images/decoration/Plant_4/Plant4_"};
    std::string plant5Path{"images/decoration/Plant_5/Plant5_"};
    std::string plant6Path{"images/decoration/Plant_6/Plant6_"};
    std::string plant7Path{"images/decoration/Plant_7/Plant7_"};
    std::string jumpPlantPath{"images/decoration/PlantJump/JumpPlant_"};
    std::string cat1Path{"images/decoration/sleepingCat/cat/RetroCatsFree_"};
    std::string ground1Path{"images/Ground/mramoric.png"};
    std::string groundTileSetGreenPath{"images/Ground/TileSetGreen/TileSetGreen_"};
};