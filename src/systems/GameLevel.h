#pragma once
#include<iostream>
#include<SFML/Graphics.hpp>
#include<Background.h>
#include<Platform.h>
#include<Decoration.h>
#include<Ground.h>
#include<vector>
#include<memory.h>
#include<map>
#include<fstream>
#include<EnemyManager.h>
#include<Player.h>

class Background;
class Decoration;
class EnemyManager;
class GameLevelManager;
class Player;

const std::string LEVELS_FOLDER = "data/levelData/";

//////////////////////////////////////////////////
// Определяет экземпляр уровня с объектами, врагами, спавнерами
//////////////////////////////////////////////////
class GameLevel
{
private:
    sf::Vector2i size;                                          // Длинна и ширина уровня
    std::shared_ptr< Platform   > platforms;                    // Платформы
    std::shared_ptr< Decoration > decorations;                  // Декорации
    std::shared_ptr< Ground     > ground;                       // Пол
    std::vector<std::shared_ptr< Background >> background;      // Фон
    

    Player* player;
    GameData* data;
    GameCamera* camera;
    GameLevelManager* levelManager;
    EnemyManager*  enemyManager;
    sf::RenderWindow* window;

    sf::Vector2f playerSpawnPos;

    //////////////////////////////////////////////////
    // Определяет, нужно ли сбрасывать состояние объектов на уровне.
    // После первой загрузки уровня, становится false.
    //////////////////////////////////////////////////
    bool doResetToBase = true;

    //////////////////////////////////////////////////
    // Если уровень константный, то любые изменения на нем не фиксируются.
    // Уровень сбрасывается к изначальному состоянию каждую загрузку
    //////////////////////////////////////////////////
    bool isConstant = true;

    ////////////////////////////////////////////////////
    // Инициализация объектов
    ////////////////////////////////////////////////////
    void initializePlatforms(const nlohmann::json& data);
    void initializeDecorations(const nlohmann::json& data);
    void initializeBackground(const nlohmann::json& data);
    void initializeGround(const nlohmann::json& data);
    void initializeEnemyManager(const nlohmann::json& data);
    ////////////////////////////////////////////////////
public:

    GameLevel(GameData& d, Player& p, GameCamera& c, GameLevelManager& m, sf::RenderWindow& w, const std::string& fileNamePath);
    ~GameLevel();

    //////////////////Variables///////////////////////
    std::string levelName;
    //////////////////////////////////////////////////

    ////////////////////////////////////////////////////
    // Обновляет уровень на который указывает итератор
    ////////////////////////////////////////////////////
    void update();           
    ////////////////////////////////////////////////////
    // Обновляет уровень на который указывает итератор
    ////////////////////////////////////////////////////
    void updatePlatforms();     // NOTE Не реализовано т.к. у объекта нет update()
    void updateDecorations();   
    void updateBackgrounds();   
    void updateGrounds();       // NOTE Не реализовано т.к. у объекта нет update()
    void updateEnemyManager();
    ////////////////////////////////////////////////////                

    ////////////////////////////////////////////////////
    // Рисует уровень на который указывает итератор
    ////////////////////////////////////////////////////
    void draw();
    ////////////////////////////////////////////////////
    // Рисует уровень на который указывает итератор
    ////////////////////////////////////////////////////
    void drawPlatforms();
    void drawDecorations();
    void drawBackgrounds();
    void drawGrounds();
    void drawEnemyManager();
    ////////////////////////////////////////////////////

    //////////////////////////////////////////////////
    // Должен подгружать данные из json файла.
    // Данные ето позиции и тип платформ, декораций, земли, фона. А также название уровня.
    //
    // Если doResetToBase==true, подгружает данные о уровне из основного файла, а если нет - из файла сохранения уровня(хранится в json)
    //////////////////////////////////////////////////
    void loadLevelData(const std::string& fileName);       

    //////////////////////////////////////////////////
    // Прибраться на уровне. Удалить все объекты.
    //////////////////////////////////////////////////
    void clearLevel();

    //////////////////////////////////////////////////
    // Должен сохранять некоторые состояние объектов на уровне.
    // У сохраняемого объекта должен быть тип "save=true".
    //////////////////////////////////////////////////
    void saveLevelData();       // IMPLEMENTME

    //////////////////////////////////////////////////
    // Должен менять переменную doResetToBase на true.
    //////////////////////////////////////////////////
    void resetTobase();         // IMPLEMENTME

    //Getters
    sf::Vector2i getLevelSize() const;
    std::vector<std::shared_ptr<sf::RectangleShape>>& getPlatformRects();
    sf::RectangleShape& getGroundRect();
    sf::Vector2f getPlayerSpawnPos();

    // Setters
    void attachPlayer(Player& p);
};

//////////////////////////////////////////////////////
// Управляет игровыми уровнями
//////////////////////////////////////////////////////
class GameLevelManager
{
private:
    // Указатели на внешние объекты
    Player* player;
    GameData* data;
    GameCamera* camera;
    sf::RenderWindow* window;

    const std::string levelsFolder;

    //////////////////////////////////////////////////
    // string - Название уровня, подгружается из GameLevel
    // GameLevel - Экземляр уровня
    //////////////////////////////////////////////////
    std::map<std::string, std::shared_ptr< GameLevel >> levels;

    std::map<std::string, std::shared_ptr< GameLevel >>::iterator levelIt;

    void initializeLevels(const std::string levelsFolder);   

public:
    GameLevelManager(GameData &d, GameCamera& c,sf::RenderWindow& w, const std::string& lF);
    ~GameLevelManager();

    ////////////////////////////////////////////////////
    // Перемещает итератор на уровень с ключем <name> в std::map levels
    ////////////////////////////////////////////////////
    bool goToLevel(std::optional<std::string> levelName);

    ////////////////////////////////////////////////////
    // Обновляет уровень на который указывает итератор
    ////////////////////////////////////////////////////
    void update();
    ////////////////////////////////////////////////////
    // Обновляет уровень на который указывает итератор
    ////////////////////////////////////////////////////
    void updatePlatforms();     // NOTE Не реализовано т.к. у объекта нет update()
    void updateDecorations();   
    void updateBackgrounds();   
    void updateGrounds();       // NOTE Не реализовано т.к. у объекта нет update()
    void updateEnemyManager();
    ////////////////////////////////////////////////////       

    ////////////////////////////////////////////////////
    // Отрисовывает уровень на который указывает итератор
    ////////////////////////////////////////////////////
    void draw();    
    ////////////////////////////////////////////////////
    // Отрисовывает уровень на который указывает итератор
    ////////////////////////////////////////////////////
    void drawPlatforms();
    void drawDecorations();
    void drawBackgrounds();
    void drawGrounds();
    void drawEnemyManager();
    ////////////////////////////////////////////////////

    //Getters
    sf::Vector2i getCurrentLevelSize() const;
    std::vector<std::shared_ptr<sf::RectangleShape>>& getPlatformRects();
    sf::RectangleShape& getGroundRect();

    // Setters
    void attachPlayer(Player& p);
};