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

class Background;

//////////////////////////////////////////////////
// Определяет экземпляр уровня с объектами, врагами, спавнерами
//////////////////////////////////////////////////
class GameLevel
{
private:
    sf::Vector2f size;                                          // Длинна и ширина уровня
    std::vector<std::shared_ptr< Platform   >> platforms;       // Платформы
    std::vector<std::shared_ptr< Decoration >> decorations;     // Декорации
    std::vector<std::shared_ptr< Background >> background;      // Фон
    std::vector<std::shared_ptr< Ground     >> ground;          // Пол 

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

public:
    GameLevel();                // IMPLEMENTME
    ~GameLevel();               // IMPLEMENTME

    //////////////////Variables///////////////////////
    std::string levelName;
    //////////////////////////////////////////////////

    void update();                      
    void draw(sf::RenderWindow& window);

    //////////////////////////////////////////////////
    // Должен подгружать данные из json файла.
    // Данные ето позиции и тип платформ, декораций, земли, фона. А также название уровня.
    //
    // Если doResetToBase==true, подгружает данные о уровне из основного файла, а если нет - из файла сохранения уровня(хранится в json)
    //////////////////////////////////////////////////
    void loadLevelData();       // IMPLEMENTME

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

};

//////////////////////////////////////////////////////
// Управляет игровыми уровнями
//////////////////////////////////////////////////////
class GameLevelManager
{
private:
    //////////////////////////////////////////////////
    // string - Название уровня, подгружается из GameLevel
    // GameLevel - Экземляр уровня
    //////////////////////////////////////////////////
    std::map<std::string, std::shared_ptr< GameLevel >> levels;

    std::map<std::string, std::shared_ptr< GameLevel >>::iterator levelIt;
    

public:
    GameLevelManager();
    ~GameLevelManager();

    //////////////////////////////////////////////////
    // Перемещает итератор на уровень с ключем <name> в std::map levels
    //////////////////////////////////////////////////
    void goToLevel(std::string name);

    //////////////////////////////////////////////////
    // Отрисовывает/обновляет уровень на который указывает итератор
    //////////////////////////////////////////////////
    void update();
    void draw(sf::RenderWindow& window);

    //Getters
    sf::Vector2f getCurrentLevelSize() const;
};