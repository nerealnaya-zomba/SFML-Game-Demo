#pragma once
#include<SFML/Graphics.hpp>
#include<Item.h>
#include<Player.h>
#include<GameData.h>
#include<vector>
#include<Interactive.h>

const unsigned int BASE_SHOP_COLUMNS                        = 5;
const unsigned int BASE_SHOP_ROWS                           = 4;
const sf::Vector2f BASE_SHOP_CELL_SIZE                      = {40,40};
const sf::Vector2f BASE_SHOP_BACKGROUND_SIZE                = {200,200};
const sf::Vector2f BASE_SHOP_BACKGROUND_ADDITIONAL_SCALE    = {1.0f,1.0f};
const sf::Vector2f BASE_SHOP_PADDING                        = {52,60};
const sf::Vector2f BASE_SHOP_CELL_SPRITE_SCALE              = {2.0,2.0};
const sf::Vector2f BASE_SHOP_CELL_SPRITE_SELECTED_SCALE     = {2.3,2.3};
const sf::Vector2f BASE_SHOP_WIDGET_SPRITE_SCALE            = {5.f,5.f};
const sf::Vector2f BASE_SHOP_WIDGET_DISPLAY_NAME_PADDING    = {30.f,40};
const sf::Vector2f BASE_SHOP_WIDGET_STATS_TEXT_PADDING      = {30.f,-100};

const sf::Vector2i BASE_SHOP_ITEMS_MARGIN                   = {20,20};

const sf::Keyboard::Scancode SHOP_KEY_TO_MOVE_RIGHT         = sf::Keyboard::Scancode::Right;
const sf::Keyboard::Scancode SHOP_KEY_TO_MOVE_LEFT          = sf::Keyboard::Scancode::Left;
const sf::Keyboard::Scancode SHOP_KEY_TO_MOVE_DOWN          = sf::Keyboard::Scancode::Down;
const sf::Keyboard::Scancode SHOP_KEY_TO_MOVE_UP            = sf::Keyboard::Scancode::Up;
const sf::Keyboard::Scancode SHOP_KEY_TO_OPEN_ITEM_WIDGET   = sf::Keyboard::Scancode::Z;
const sf::Keyboard::Scancode SHOP_KEY_TO_CLOSE_ITEM_WIDGET  = sf::Keyboard::Scancode::X;

class Shop : InteractiveObject
{
private:
    class ItemWidget
    {
    private:
        sf::Text displayNameText;
        sf::Text statsText;
        sf::Sprite background;
        Item::Stats stat;

        bool isOpened;

    public:
        ItemWidget(GameData& data, sf::Vector2f widgetSize, sf::Vector2f widgetPos, sf::Vector2f displayNamePosLocal, sf::Vector2f statsTextPosLocal, sf::Font& font, uint8_t displayNameSize, uint8_t statsTextSize);
        ~ItemWidget() = default;

        void draw(sf::RenderWindow& window);
        void update();
        void handleEvent(const sf::Event& event);

        void open();
        void close();

        void buy();
        void cancel();

        // Getters
        bool getIsOpened();

        // Setters
        void attachItemStats(Item& item);
        void setWidgetScale(sf::Vector2f size);
        void setDisplayNameText(std::string str);
        void setStatsText(std::string str);
        void setDisplayNameColor(sf::Color c);
        void setStatsTextColor(sf::Color c);
            // Перемещается ТОЛЬКО внутри области виджета. Позиционирование от левого-верхнего угла
        void setDisplayNamePosition(sf::Vector2f position);
        void setStatsTextPosition(sf::Vector2f position);
            // Перемещает весь виджет, включая текст. Позиционирование по центру.
        void setWidgetCenterPosition(sf::Vector2f position);
    };
private:
    Player* player;
    GameData* data;

    ItemWidget widget;

    //Grid settings
    int columns;
    int rows;
    sf::Vector2i itemsMargin;
    sf::Vector2f cellSize;

    // Items storage
    std::vector<std::pair<sf::Sprite,std::unique_ptr<Item>>> items;

    // Items selection iterator
    std::vector<std::pair<sf::Sprite,std::unique_ptr<Item>>>::iterator itemsIt;

    // Shop bools
    bool isOpened;
    bool isItemWidgetOpened;
    
    // Items init
    void initializeItems();
    
    // Items aligning
    void alignItemsOnGrid();

    // Событие происходящее при выделении предмета
    void onItemSelected();
    // Событие происходящее перед перемещением выделения
    void onSelectedChanged();
    // Событие при закрытии магазина
    void onShopClosed();
    // Событие при открытии магазина
    void onShopOpened();

    // Двинуть выделение
    void moveSelectionRight();
    void moveSelectionLeft();
    void moveSelectionDown();
    void moveSelectionUp();

    // Заблокировать/разблокировать управление игроком
    void blockPlayerControl();
    void unblockPlayerControl();

    // Открыть/закрыть вкладку с информацией покупки
    void openSelectedItemWidget();
    void closeSelectedItemWidget();

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