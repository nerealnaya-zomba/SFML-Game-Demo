#pragma once
#include<SFML/Graphics.hpp>
#include<Item.h>
#include<Player.h>
#include<GameData.h>
#include<vector>
#include<Interactive.h>

const unsigned int BASE_SHOP_COLUMNS          = 5;
const unsigned int BASE_SHOP_ROWS             = 4;
const sf::Vector2f BASE_SHOP_CELL_SIZE        = {40,40};
const sf::Vector2f BASE_SHOP_BACKGROUND_SIZE  = {200,200};
const sf::Vector2f BASE_SHOP_PADDING          = {20,20};
const sf::Vector2f BASE_SHOP_ITEM_SCALEUP_MULTIPLY_ON_HOVER = {1.2,1.2};

const sf::Keyboard::Scancode SHOP_KEY_TO_MOVE_RIGHT = sf::Keyboard::Scancode::Right;
const sf::Keyboard::Scancode SHOP_KEY_TO_MOVE_LEFT = sf::Keyboard::Scancode::Left;

class Shop : InteractiveObject
{
private:
    Player* player;
    GameData* data;

    sf::RectangleShape shopBackground;

    //Grid settings
    unsigned int columns;
    unsigned int rows;
    sf::Vector2f cellSize;

    // Items storage
    std::map<std::unique_ptr<sf::Text> ,std::unique_ptr<Item>> items;

    // Items selection iterator
    std::map<std::unique_ptr<sf::Text> ,std::unique_ptr<Item>>::iterator itemsIt;

    // Shop bools
    bool isOpened;
    
    // Items init
    void initializeItems();
    
    // Items aligning
    void alignItemsOnGrid();

    // Событие происходящее при выделении предмета
    void onItemSelected();
    // Событие происходящее перед перемещением выделения
    void onSelectedChanged();

    // Двинуть выделение
    void moveSelectionRight();
    void moveSelectionLeft();
public:
    Shop(GameData& d, Player& p, sf::Vector2f pos);
    ~Shop() = default;
    void open();
    void close();
    
    void addItem(std::unique_ptr<Item> item);

    void draw(sf::RenderWindow& window)         override;
    void update()                               override;
    void handleEvent(const sf::Event& event)    override;

    bool getIsOpened();
};