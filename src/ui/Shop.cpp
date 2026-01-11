#include<Shop.h>

void Shop::update()
{
    if(!isOpened) return;
    alignItemsOnGrid();
}

void Shop::handleEvent(const sf::Event &event)
{
    if(!isOpened) return;
}

void Shop::draw(sf::RenderWindow& window)
{
    if(!isOpened) return;

    // window.draw(*sprite);
    window.draw(shopBackground);

    for (auto &&item : items)
    {
        item.second->draw(window);
        window.draw(*(item.first));
    }
    
}

void Shop::initializeItems()
{
    Item::Stats statsBlood = {0,0,50,100,0,0,100};
    addItem(std::make_unique<Item>(
        *this->data,
        static_cast<sf::Vector2i>(BASE_SHOP_CELL_SIZE),
        static_cast<sf::Vector2i>(getCenterPosition()),
        "Item_11.png",
        "Book of blood",
        Item::COMMON,
        140,
        statsBlood
    ));

    
    addItem(std::make_unique<Item>(
        *this->data,
        static_cast<sf::Vector2i>(BASE_SHOP_CELL_SIZE),
        static_cast<sf::Vector2i>(getCenterPosition()),
        "Item_12.png",
        "Book of revenge",
        Item::COMMON,
        160,
        statsBlood
    ));

    
    addItem(std::make_unique<Item>(
        *this->data,
        static_cast<sf::Vector2i>(BASE_SHOP_CELL_SIZE),
        static_cast<sf::Vector2i>(getCenterPosition()),
        "Item_13.png",
        "Book of wind",
        Item::COMMON,
        200,
        statsBlood
    ));

    
    addItem(std::make_unique<Item>(
        *this->data,
        static_cast<sf::Vector2i>(BASE_SHOP_CELL_SIZE),
        static_cast<sf::Vector2i>(getCenterPosition()),
        "Item_14.png",
        "Book of myth",
        Item::COMMON,
        210,
        statsBlood
    ));

    
    addItem(std::make_unique<Item>(
        *this->data,
        static_cast<sf::Vector2i>(BASE_SHOP_CELL_SIZE),
        static_cast<sf::Vector2i>(getCenterPosition()),
        "Item_15.png",
        "Wooden book",
        Item::COMMON,
        50,
        statsBlood
    ));

    
    addItem(std::make_unique<Item>(
        *this->data,
        static_cast<sf::Vector2i>(BASE_SHOP_CELL_SIZE),
        static_cast<sf::Vector2i>(getCenterPosition()),
        "Item_16.png",
        "Dungeon book",
        Item::COMMON,
        100,
        statsBlood
    ));
}

void Shop::alignItemsOnGrid()
{
    sf::Vector2f backgroundStart = {shopBackground.getPosition().x-(shopBackground.getSize().x/2),shopBackground.getPosition().y-(shopBackground.getSize().y/2)};
    
    int iterationCount = 0;
    int countingForNextRow = 0;

    for (auto &&item : items)
    {
        sf::Vector2f nextPos = {
            backgroundStart.x+(BASE_SHOP_CELL_SIZE.x/2)+(BASE_SHOP_CELL_SIZE.x*iterationCount),
            backgroundStart.y+(BASE_SHOP_CELL_SIZE.y/2)+(BASE_SHOP_CELL_SIZE.y*countingForNextRow)
        };

        sf::Vector2i convertedNextPos = static_cast<sf::Vector2i>(nextPos);

        item.second->setPosition(convertedNextPos);

        iterationCount++;

        if(iterationCount==columns)
        {
            countingForNextRow++;
            iterationCount = 0;
        }
    }
}

Shop::Shop(GameData &d, Player &p, sf::Vector2f pos)
    : InteractiveObject(pos, d.bulletTextures[0]), isOpened(false), data(&d), player(&p), columns(BASE_SHOP_COLUMNS),
      rows(BASE_SHOP_ROWS), cellSize(BASE_SHOP_CELL_SIZE)
{
    shopBackground.setSize(BASE_SHOP_BACKGROUND_SIZE);
    setRectangleOriginToMiddle(shopBackground);
    shopBackground.setPosition(pos);
    shopBackground.setFillColor(sf::Color::Red);

    initializeItems();
}

void Shop::open()
{
    isOpened = true;
}

void Shop::close()
{
    isOpened = false;
}

void Shop::addItem(std::unique_ptr<Item> item)
{
    std::unique_ptr<sf::Text> text = std::make_unique<sf::Text>
    (
        *data->gameFont,
        item->displayName
    );

    items.emplace(
        std::move(text),
        std::move(item)
    );
}