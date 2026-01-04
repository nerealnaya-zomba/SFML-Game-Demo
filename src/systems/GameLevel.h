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

class Background;
class Decoration;

//////////////////////////////////////////////////
// Определяет экземпляр уровня с объектами, врагами, спавнерами
//////////////////////////////////////////////////
class GameLevel
{
private:
    sf::Vector2f size;                                          // Длинна и ширина уровня
    std::shared_ptr< Platform   > platforms;                    // Платформы
    std::shared_ptr< Decoration > decorations;                  // Декорации
    std::vector<std::shared_ptr< Background >> background;      // Фон
    std::vector<std::shared_ptr< Ground     >> ground;          // Пол

    Player* player;
    GameData* data;
    GameCamera* camera;
    GameLevelManager* manager;

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
    ////////////////////////////////////////////////////
public:

    GameLevel(GameData& d, Player& p, GameCamera& c, GameLevelManager& m, const std::string& fileNamePath);
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
    ////////////////////////////////////////////////////                

    ////////////////////////////////////////////////////
    // Рисует уровень на который указывает итератор
    ////////////////////////////////////////////////////
    void draw(sf::RenderWindow& window);
    ////////////////////////////////////////////////////
    // Рисует уровень на который указывает итератор
    ////////////////////////////////////////////////////
    void drawPlatforms(sf::RenderWindow& window);
    void drawDecorations(sf::RenderWindow& window);
    void drawBackgrounds(sf::RenderWindow& window);
    void drawGrounds(sf::RenderWindow& window);
    ////////////////////////////////////////////////////

    //////////////////////////////////////////////////
    // Должен подгружать данные из json файла.
    // Данные ето позиции и тип платформ, декораций, земли, фона. А также название уровня.
    //
    // Если doResetToBase==true, подгружает данные о уровне из основного файла, а если нет - из файла сохранения уровня(хранится в json)
    //////////////////////////////////////////////////
    void loadLevelData(const std::string& fileName);       

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
    sf::Vector2f getLevelSize() const;
    std::vector<std::shared_ptr<sf::RectangleShape>>& getPlatformRects();
    sf::RectangleShape& getGroundRect();
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

    //////////////////////////////////////////////////
    // string - Название уровня, подгружается из GameLevel
    // GameLevel - Экземляр уровня
    //////////////////////////////////////////////////
    std::map<std::string, std::shared_ptr< GameLevel >> levels;

    std::map<std::string, std::shared_ptr< GameLevel >>::iterator levelIt;

    void initializeLevels(const std::string levelsFolder);   

public:
    GameLevelManager(GameData &d, Player &p, GameCamera& c, const std::string& levelsFolder);
    ~GameLevelManager();

    ////////////////////////////////////////////////////
    // Перемещает итератор на уровень с ключем <name> в std::map levels
    ////////////////////////////////////////////////////
    void goToLevel(std::string levelName);

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
    ////////////////////////////////////////////////////       

    ////////////////////////////////////////////////////
    // Отрисовывает уровень на который указывает итератор
    ////////////////////////////////////////////////////
    void draw(sf::RenderWindow& window);    
    ////////////////////////////////////////////////////
    // Отрисовывает уровень на который указывает итератор
    ////////////////////////////////////////////////////
    void drawPlatforms(sf::RenderWindow& window);
    void drawDecorations(sf::RenderWindow& window);
    void drawBackgrounds(sf::RenderWindow& window);
    void drawGrounds(sf::RenderWindow& window);
    ////////////////////////////////////////////////////

    //Getters
    sf::Vector2f getCurrentLevelSize() const;
    std::vector<std::shared_ptr<sf::RectangleShape>>& getPlatformRects();
    sf::RectangleShape& getGroundRect();
};