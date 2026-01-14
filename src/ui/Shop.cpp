#include<Shop.h>

void Shop::update()
{
    if(!isOpened) return;

    alignItemsOnGrid();
    onItemSelected();

    widget.update();
}

void Shop::handleEvent(const sf::Event &event)
{
    if(widget.getIsOpened())
    {
        widget.handleEvent(event);
        return;
    }


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
        else if(keyPressed->scancode == SHOP_KEY_TO_MOVE_DOWN)
        {
            moveSelectionDown();
        }
        else if(keyPressed->scancode == SHOP_KEY_TO_MOVE_UP)
        {
            moveSelectionUp();
        }
        else if(keyPressed->scancode == SHOP_KEY_TO_OPEN_ITEM_WIDGET)
        {
            openSelectedItemWidget();
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
        item->draw(window);
    }
    
    widget.draw(window);
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

    addItem(std::make_unique<Item>(
        *this->data,
        static_cast<sf::Vector2i>(BASE_SHOP_CELL_SIZE),
        static_cast<sf::Vector2i>(getCenterPosition()),
        "item_01.png",
        "Dungeon book",
        Item::COMMON,
        100,
        statsBlood
    ));

    addItem(std::make_unique<Item>(
        *this->data,
        static_cast<sf::Vector2i>(BASE_SHOP_CELL_SIZE),
        static_cast<sf::Vector2i>(getCenterPosition()),
        "item_02.png",
        "Dungeon book",
        Item::COMMON,
        100,
        statsBlood
    ));

    addItem(std::make_unique<Item>(
        *this->data,
        static_cast<sf::Vector2i>(BASE_SHOP_CELL_SIZE),
        static_cast<sf::Vector2i>(getCenterPosition()),
        "item_03.png",
        "Dungeon book",
        Item::COMMON,
        100,
        statsBlood
    ));

    addItem(std::make_unique<Item>(
        *this->data,
        static_cast<sf::Vector2i>(BASE_SHOP_CELL_SIZE),
        static_cast<sf::Vector2i>(getCenterPosition()),
        "item_04.png",
        "Dungeon book",
        Item::COMMON,
        100,
        statsBlood
    ));

    addItem(std::make_unique<Item>(
        *this->data,
        static_cast<sf::Vector2i>(BASE_SHOP_CELL_SIZE),
        static_cast<sf::Vector2i>(getCenterPosition()),
        "item_05.png",
        "Dungeon book",
        Item::COMMON,
        100,
        statsBlood
    ));

    addItem(std::make_unique<Item>(
        *this->data,
        static_cast<sf::Vector2i>(BASE_SHOP_CELL_SIZE),
        static_cast<sf::Vector2i>(getCenterPosition()),
        "item_06.png",
        "Dungeon book",
        Item::COMMON,
        100,
        statsBlood
    ));

    addItem(std::make_unique<Item>(
        *this->data,
        static_cast<sf::Vector2i>(BASE_SHOP_CELL_SIZE),
        static_cast<sf::Vector2i>(getCenterPosition()),
        "item_07.png",
        "Dungeon book",
        Item::COMMON,
        100,
        statsBlood
    ));

    addItem(std::make_unique<Item>(
        *this->data,
        static_cast<sf::Vector2i>(BASE_SHOP_CELL_SIZE),
        static_cast<sf::Vector2i>(getCenterPosition()),
        "item_08.png",
        "Dungeon book",
        Item::COMMON,
        100,
        statsBlood
    ));

    addItem(std::make_unique<Item>(
        *this->data,
        static_cast<sf::Vector2i>(BASE_SHOP_CELL_SIZE),
        static_cast<sf::Vector2i>(getCenterPosition()),
        "item_09.png",
        "Dungeon book",
        Item::COMMON,
        100,
        statsBlood
    ));

    addItem(std::make_unique<Item>(
        *this->data,
        static_cast<sf::Vector2i>(BASE_SHOP_CELL_SIZE),
        static_cast<sf::Vector2i>(getCenterPosition()),
        "item_10.png",
        "Dungeon book",
        Item::COMMON,
        100,
        statsBlood
    ));

    addItem(std::make_unique<Item>(
        *this->data,
        static_cast<sf::Vector2i>(BASE_SHOP_CELL_SIZE),
        static_cast<sf::Vector2i>(getCenterPosition()),
        "item_18.png",
        "Dungeon book",
        Item::COMMON,
        100,
        statsBlood
    ));

    addItem(std::make_unique<Item>(
        *this->data,
        static_cast<sf::Vector2i>(BASE_SHOP_CELL_SIZE),
        static_cast<sf::Vector2i>(getCenterPosition()),
        "item_19.png",
        "Dungeon book",
        Item::COMMON,
        100,
        statsBlood
    ));

    addItem(std::make_unique<Item>(
        *this->data,
        static_cast<sf::Vector2i>(BASE_SHOP_CELL_SIZE),
        static_cast<sf::Vector2i>(getCenterPosition()),
        "item_20.png",
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

        item->setPosition(convertedNextPos);

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
    itemsIt->get()->setScale({
        itemsIt->get()->getBaseScale().x*BASE_SHOP_ITEM_SCALEUP_MULTIPLY_ON_HOVER.x,
        itemsIt->get()->getBaseScale().y*BASE_SHOP_ITEM_SCALEUP_MULTIPLY_ON_HOVER.y
    });
}

void Shop::onSelectedChanged()
{
    itemsIt->get()->setScale(itemsIt->get()->getBaseScale());
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

void Shop::moveSelectionDown()
{
    onSelectedChanged();
    
    size_t currentIndex = itemsIt - items.begin();
    size_t newIndex = currentIndex + columns;
    
    // Если выходим за пределы вектора
    if (newIndex >= items.size())
    {
        // Циклически перемещаемся наверх: находим элемент в том же столбце в верхнем ряду
        // Вычисляем столбец текущего элемента
        size_t column = currentIndex % columns;
        
        // Перемещаемся в первый ряд, в тот же столбец
        newIndex = column;
        
        // Если в первом ряду в этом столбце нет элемента, ищем ближайший слева
        while (newIndex >= items.size())
        {
            newIndex--;
        }
    }
    
    itemsIt = items.begin() + newIndex;
}
void Shop::moveSelectionUp()
{
    onSelectedChanged();
    
    size_t currentIndex = itemsIt - items.begin();
    
    // Если мы уже в первом ряду или выше
    if (currentIndex < columns)
    {
        // Циклически перемещаемся вниз: находим элемент в том же столбце в последнем ряду
        size_t column = currentIndex % columns;
        
        // Начинаем с нижнего элемента в этом столбце
        size_t newIndex = ((items.size() - 1) / columns) * columns + column;
        
        // Если newIndex выходит за пределы вектора, уменьшаем его
        if (newIndex >= items.size())
        {
            newIndex = items.size() - 1;
            
            // Если последний элемент не в нужном столбце, ищем ближайший в том же столбце
            while (newIndex % columns > column)
            {
                newIndex--;
            }
        }
        
        itemsIt = items.begin() + newIndex;
    }
    else
    {
        // Просто перемещаемся на ряд вверх
        itemsIt -= columns;
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

void Shop::openSelectedItemWidget()
{
    isItemWidgetOpened = true;
    widget.attachItemStats(*itemsIt->get());
    widget.open();
}

void Shop::closeSelectedItemWidget()
{
    isItemWidgetOpened = false;
    widget.close();
}

Shop::Shop(GameData &d, Player &p, sf::Vector2f pos)
    : InteractiveObject(pos, d.bulletTextures[0]), isOpened(false), data(&d), player(&p), columns(BASE_SHOP_COLUMNS),
      rows(BASE_SHOP_ROWS), 
      cellSize(BASE_SHOP_CELL_SIZE), 
      isItemWidgetOpened(false), 
      widget({BASE_SHOP_BACKGROUND_SIZE.x, BASE_SHOP_BACKGROUND_SIZE.y},pos,{0,0},{0.f,BASE_SHOP_BACKGROUND_SIZE.y}, *d.gameFont, (BASE_SHOP_BACKGROUND_SIZE.x+BASE_SHOP_BACKGROUND_SIZE.y)/20, (BASE_SHOP_BACKGROUND_SIZE.x+BASE_SHOP_BACKGROUND_SIZE.y)/16)
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
    items.push_back(std::move(item));
}

Shop::ItemWidget::ItemWidget(sf::Vector2f widgetSize, sf::Vector2f widgetPos,sf::Vector2f displayNamePosLocal, sf::Vector2f statsTextPosLocal, sf::Font& font, uint8_t displayNameSize, uint8_t statsTextSize)
    : displayNameText(font,"",displayNameSize), statsText(font,"",statsTextSize), isOpened(false)
{
    // Background init
    rect.setFillColor(sf::Color::Blue);
    rect.setSize(widgetSize);
    setRectangleOriginToMiddle(rect);
    

    this->displayNameText.setCharacterSize(displayNameSize);
    this->statsText.setCharacterSize(statsTextSize);
    setDisplayNamePosition(displayNamePosLocal);
    setStatsTextPosition(statsTextPosLocal);

    setWidgetCenterPosition(widgetPos);
}

void Shop::ItemWidget::draw(sf::RenderWindow &window)
{
    if(!isOpened) return;
    
    window.draw(this->rect);
    window.draw(this->displayNameText);
    window.draw(this->statsText);
}

void Shop::ItemWidget::update()
{
    if(!isOpened) return;
}

void Shop::ItemWidget::handleEvent(const sf::Event &event)
{
    if(!isOpened) return;

    //При нажатии на кнопку
    if(const auto* keyPressed = event.getIf<sf::Event::KeyPressed>())
    {
        // Активировать выделенную кнопку
        if(keyPressed->scancode == sf::Keyboard::Scancode::Z)
        {
            buy(); 
        }
        else if(keyPressed->scancode == sf::Keyboard::Scancode::X)
        {
            close();
        }
    }
}

void Shop::ItemWidget::open()
{
    isOpened = true;
}

void Shop::ItemWidget::close()
{
    isOpened = false;
}

void Shop::ItemWidget::buy()
{

    close();
}

void Shop::ItemWidget::cancel()
{

    close();
}

bool Shop::ItemWidget::getIsOpened()
{
    return this->isOpened;
}

void Shop::ItemWidget::attachItemStats(Item& item)
{
    displayNameText.setString(item.displayName);
    statsText.setString(
        "BulletDistance: +"     + std::to_string(item.stats.bulletDistance)                 + "\n"+
        "BulletSpeed: +"        + std::to_string(item.stats.bulletSpeed)                    + "\n"+
        "BulletDamage: +"       + std::to_string(item.stats.damage)                         + "\n"+
        "Health: +"             + std::to_string(item.stats.health)                         + "\n"+
        "InitialSpeed: +"       + std::to_string(item.stats.initialSpeed)                   + "\n"+
        "MaxSpeed: +"           + std::to_string(item.stats.maxSpeed)                       + "\n"+
        "Shoot cooldown: -"     + std::to_string(item.stats.shootSpeedCooldownReduction)    + "\n"
    );
}

void Shop::ItemWidget::setWidgetSize(sf::Vector2f size)
{
    rect.setSize(size);
    setRectangleOriginToMiddle(rect);
}

void Shop::ItemWidget::setDisplayNameText(std::string str)
{
    this->displayNameText.setString(str);
}

void Shop::ItemWidget::setStatsText(std::string str)
{
    this->statsText.setString(str);
}

void Shop::ItemWidget::setDisplayNamePosition(sf::Vector2f position)
{
    sf::Vector2f rectLeftTopPos = {
        rect.getPosition().x-(rect.getSize().x/2),
        rect.getPosition().y-(rect.getSize().y/2)
    };
    
    this->displayNameText.setPosition({
        rectLeftTopPos.x+position.x,
        rectLeftTopPos.y+position.y
    });
}

void Shop::ItemWidget::setStatsTextPosition(sf::Vector2f position)
{
    sf::Vector2f rectLeftTopPos = {
        rect.getPosition().x-(rect.getSize().x/2),
        rect.getPosition().y-(rect.getSize().y/2)
    };
    
    this->statsText.setPosition({
        rectLeftTopPos.x+position.x,
        rectLeftTopPos.y+position.y
    });
}

void Shop::ItemWidget::setWidgetCenterPosition(sf::Vector2f position)
{
    // Getting difference
    sf::Vector2f positionDiff = {
        -(rect.getPosition().x-position.x),
        -(rect.getPosition().y-position.y)
    };

    // Rect positioning
    rect.setPosition(position);

    // DisplayName positioning
    sf::Vector2f displayNamePos = displayNameText.getPosition();
    displayNameText.setPosition({
        displayNamePos.x+positionDiff.x,
        displayNamePos.y+positionDiff.y
    });

    // StatsText positioning
    sf::Vector2f statsTextPos = statsText.getPosition();
    statsText.setPosition({
        statsTextPos.x+positionDiff.x,
        statsTextPos.y+positionDiff.y
    });
}
