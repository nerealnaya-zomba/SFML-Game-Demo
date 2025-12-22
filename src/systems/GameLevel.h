#include<iostream>
#include<SFML/Graphics.hpp>
#include<Background.h>
#include<Platform.h>
#include<Decoration.h>
#include<Ground.h>
#include<vector>
#include<memory.h>
#include<map>

//////////////////////////////////////////////////
// Определяет экземпляр уровня с объектами, врагами, спавнерами
//////////////////////////////////////////////////
class GameLevel
{
private:
    sf::Vector2f size;
    std::vector<std::shared_ptr< Platform   >> platforms;
    std::vector<std::shared_ptr< Decoration >> decorations;
    std::vector<std::shared_ptr< Background >> background;
    std::vector<std::shared_ptr< Ground     >> ground;

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

    void update();                       // IMPLEMENTME
    void draw(sf::RenderWindow& window); // IMPLEMENTME

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
};