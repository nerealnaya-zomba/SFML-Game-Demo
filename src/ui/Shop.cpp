#include<Shop.h>

void Shop::update()
{
    if(!isOpened) return;
    alignItemsOnGrid();
    onItemSelected();
}

void Shop::handleEvent(const sf::Event &event)
{
    if(!isOpened) return;

    if(const auto* keyPressed = event.getIf<sf::Event::KeyPressed>())
    {
        if(keyPressed->scancode == SHOP_KEY_TO_MOVE_RIGHT)
        {
            moveSelectionRight();
        }
        else if(keyPressed->scancode == SHOP_KEY_TO_MOVE_LEFT)
        {
            moveSelectionLeft();
        }
    }
}

bool Shop::getIsOpened()
{
    return this->isOpened;
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
        "item_11.png",
        "Book of blood",
        Item::COMMON,
        140,
        statsBlood
    ));

    
    addItem(std::make_unique<Item>(
        *this->data,
        static_cast<sf::Vector2i>(BASE_SHOP_CELL_SIZE),
        static_cast<sf::Vector2i>(getCenterPosition()),
        "item_12.png",
        "Book of revenge",
        Item::COMMON,
        160,
        statsBlood
    ));

    
    addItem(std::make_unique<Item>(
        *this->data,
        static_cast<sf::Vector2i>(BASE_SHOP_CELL_SIZE),
        static_cast<sf::Vector2i>(getCenterPosition()),
        "item_13.png",
        "Book of wind",
        Item::COMMON,
        200,
        statsBlood
    ));

    
    addItem(std::make_unique<Item>(
        *this->data,
        static_cast<sf::Vector2i>(BASE_SHOP_CELL_SIZE),
        static_cast<sf::Vector2i>(getCenterPosition()),
        "item_14.png",
        "Book of myth",
        Item::COMMON,
        210,
        statsBlood
    ));

    
    addItem(std::make_unique<Item>(
        *this->data,
        static_cast<sf::Vector2i>(BASE_SHOP_CELL_SIZE),
        static_cast<sf::Vector2i>(getCenterPosition()),
        "item_15.png",
        "Wooden book",
        Item::COMMON,
        50,
        statsBlood
    ));

    
    addItem(std::make_unique<Item>(
        *this->data,
        static_cast<sf::Vector2i>(BASE_SHOP_CELL_SIZE),
        static_cast<sf::Vector2i>(getCenterPosition()),
        "item_16.png",
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
        nextPos.x+=BASE_SHOP_PADDING.x;
        nextPos.y+=BASE_SHOP_PADDING.y;

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

void Shop::onItemSelected()
{
    itemsIt->second->setScale({
        itemsIt->second->getBaseScale().x*BASE_SHOP_ITEM_SCALEUP_MULTIPLY_ON_HOVER.x,
        itemsIt->second->getBaseScale().y*BASE_SHOP_ITEM_SCALEUP_MULTIPLY_ON_HOVER.y
    });
}

void Shop::onSelectedChanged()
{
    itemsIt->second->setScale(itemsIt->second->getBaseScale());
}

void Shop::moveSelectionRight()
{
    onSelectedChanged();

    itemsIt++;
    if(itemsIt == items.end())
    {
        itemsIt = items.begin();
    }
}

void Shop::moveSelectionLeft()
{
    onSelectedChanged();

    if (itemsIt == items.begin()) {
        itemsIt = std::prev(items.end());
    } else {
        itemsIt--;
    }
}

void Shop::blockPlayerControl()
{
    this->player->blockControls();
}

void Shop::unblockPlayerControl()
{
    this->player->unblockControls();
}

Shop::Shop(GameData &d, Player &p, sf::Vector2f pos)
    : InteractiveObject(pos, d.bulletTextures[0]), isOpened(false), data(&d), player(&p), columns(BASE_SHOP_COLUMNS),
      rows(BASE_SHOP_ROWS), cellSize(BASE_SHOP_CELL_SIZE)
{
    shopBackground.setSize(BASE_SHOP_BACKGROUND_SIZE);
    setRectangleOriginToMiddle(shopBackground);
    shopBackground.setPosition(pos);
    shopBackground.setFillColor(sf::Color::Red);

    // Инициализация предметов
    initializeItems();
    itemsIt = items.begin();
}

void Shop::open()
{
    isOpened = true;
    itemsIt = items.begin();
    blockPlayerControl();
}

void Shop::close()
{
    isOpened = false;
    itemsIt = items.begin();
    unblockPlayerControl();
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