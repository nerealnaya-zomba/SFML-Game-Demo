#pragma once

#include <SFML/Graphics.hpp>
#include <GameData.h>
#include <Interactive.h>
#include <Item.h>
#include <Player.h>

#include <filesystem>
#include <memory>
#include <utility>
#include <vector>

// Базовый путь к traderData.json
const std::filesystem::path BASE_SHOP_DATA_PATH = "data/shopData.json";

const unsigned int BASE_SHOP_COLUMNS                        = 5;
const unsigned int BASE_SHOP_ROWS                           = 4;
const sf::Vector2f BASE_SHOP_CELL_SIZE                      = {40,40};
const sf::Vector2f BASE_SHOP_BACKGROUND_ADDITIONAL_SCALE    = {1.0f,1.0f};
const sf::Vector2f BASE_SHOP_PADDING                        = {52,60};
const sf::Vector2f BASE_SHOP_CELL_SPRITE_SCALE              = {2.0,2.0};
const sf::Vector2f BASE_SHOP_CELL_SPRITE_SELECTED_SCALE     = {2.3,2.3};
const sf::Vector2f BASE_SHOP_WIDGET_SPRITE_SCALE            = {8.5f,5.2f};
const float BASE_SHOP_FRAME_INSET_RATIO_X                   = 0.125f;
const float BASE_SHOP_FRAME_INSET_RATIO_Y                   = 0.125f;
const float BASE_SHOP_HEADER_SECTION_HEIGHT                 = 126.f;
const float BASE_SHOP_GRID_SECTION_GAP                      = 14.f;
const float BASE_SHOP_WIDGET_ICON_COLUMN_WIDTH              = 110.f;

const sf::Vector2i BASE_SHOP_ITEMS_MARGIN                   = {20,20};

const sf::Keyboard::Scancode SHOP_KEY_TO_MOVE_RIGHT         = sf::Keyboard::Scancode::Right;
const sf::Keyboard::Scancode SHOP_KEY_TO_MOVE_LEFT          = sf::Keyboard::Scancode::Left;
const sf::Keyboard::Scancode SHOP_KEY_TO_MOVE_DOWN          = sf::Keyboard::Scancode::Down;
const sf::Keyboard::Scancode SHOP_KEY_TO_MOVE_UP            = sf::Keyboard::Scancode::Up;
const sf::Keyboard::Scancode SHOP_KEY_TO_OPEN_ITEM_WIDGET   = sf::Keyboard::Scancode::Z;
const sf::Keyboard::Scancode SHOP_KEY_TO_CLOSE_ITEM_WIDGET  = sf::Keyboard::Scancode::X;
const sf::Keyboard::Scancode SHOP_KEY_TO_PREVIOUS_TAB       = sf::Keyboard::Scancode::Q;
const sf::Keyboard::Scancode SHOP_KEY_TO_NEXT_TAB           = sf::Keyboard::Scancode::W;

class Shop : public InteractiveObject
{
private:
    enum class ShopTab
    {
        Upgrades,
        Weapons
    };

    using ShopEntry = std::pair<sf::Sprite, std::unique_ptr<Item>>;

    class ItemWidget
    {
    private:
        Shop* owner = nullptr;
        Item* attachedItem = nullptr;

        sf::Sprite background;
        std::unique_ptr<sf::Sprite> itemIcon;
        bool hasItemIcon = false;

        sf::Text displayNameText;
        sf::Text categoryText;
        sf::Text qualityText;
        sf::Text priceText;
        sf::Text statsText;
        sf::Text descriptionText;
        sf::Text stateText;
        sf::Text hintText;

        bool isOpened = false;

        void refreshState();
        void updateLayout();

    public:
        ItemWidget(Shop& owner, GameData& data, sf::Vector2f widgetScale, sf::Vector2f widgetPos, sf::Font& font);
        ~ItemWidget() = default;

        void draw(sf::RenderWindow& window);
        void update();
        void handleEvent(const sf::Event& event);

        void open();
        void close();
        void buy();

        bool getIsOpened();

        void attachItemStats(Item& item);
        void setWidgetCenterPosition(sf::Vector2f position);
    };

private:
    Player* player = nullptr;
    GameData* data = nullptr;

    ItemWidget widget;

    //Grid settings
    int columns;
    int rows;
    sf::Vector2i itemsMargin;
    sf::Vector2f cellSize;

    sf::Text titleText;
    sf::Text goldText;
    sf::Text tabHintText;
    sf::Text merchantTitleText;
    sf::Text merchantAdviceText;
    sf::Text routeHintText;
    sf::Text upgradesTabText;
    sf::Text weaponsTabText;
    sf::RectangleShape upgradesTabPlate;
    sf::RectangleShape weaponsTabPlate;

    // Items storage
    std::vector<ShopEntry> upgradeItems;
    std::vector<ShopEntry> weaponItems;
    ShopTab activeTab_ = ShopTab::Upgrades;
    std::size_t selectedIndex_ = 0;

    // Shop bools
    bool isOpened;
    bool isItemWidgetOpened;
    
    // Items init
    void initializeItems();
    bool hasItems() const;
    void updateBackgroundLayout(const sf::Vector2f& pos);
    void updateHeaderTexts();
    void updateItemFrameStates();
    std::vector<ShopEntry>& getActiveItems();
    const std::vector<ShopEntry>& getActiveItems() const;
    ShopEntry* getSelectedEntry();
    const ShopEntry* getSelectedEntry() const;
    void setActiveTab(ShopTab tab);
    void switchTab(int direction);
    void clampSelection();
    
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

    bool buySelectedItem();

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
