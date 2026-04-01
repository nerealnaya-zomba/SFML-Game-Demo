#include<Shop.h>

#include<nlohmann/json.hpp>

#include<algorithm>
#include<fstream>
#include<iomanip>
#include<sstream>
#include<stdexcept>

namespace
{
sf::Color getQualityColor(Item::Quality quality)
{
    switch (quality)
    {
        case Item::COMMON:
            return sf::Color(185, 190, 205);
        case Item::RARE:
            return sf::Color(79, 180, 255);
        case Item::MYTH:
            return sf::Color(255, 119, 91);
        case Item::LEGENDARY:
            return sf::Color(255, 209, 84);
    }

    return sf::Color::White;
}

sf::Color getShopFrameColor(bool isPurchased, bool isSelected)
{
    if (isPurchased)
    {
        return sf::Color(120, 112, 118, 170);
    }

    if (isSelected)
    {
        return sf::Color(186, 54, 72, 255);
    }

    return sf::Color(145, 33, 50, 230);
}

std::string getQualityLabel(Item::Quality quality)
{
    switch (quality)
    {
        case Item::COMMON:
            return "Common relic";
        case Item::RARE:
            return "Rare relic";
        case Item::MYTH:
            return "Myth relic";
        case Item::LEGENDARY:
            return "Legendary relic";
    }

    return "Relic";
}

std::string buildStatsText(const Item::Stats& stats)
{
    std::ostringstream stream;
    stream << std::fixed << std::setprecision(2);

    if(stats.health != 0)
    {
        stream << "Health +" << stats.health << '\n';
    }
    if(stats.damage != 0)
    {
        stream << "Damage +" << stats.damage << '\n';
    }
    if(stats.bulletSpeed != 0)
    {
        stream << "Bullet speed +" << stats.bulletSpeed << '\n';
    }
    if(stats.bulletDistance != 0)
    {
        stream << "Bullet range +" << stats.bulletDistance << '\n';
    }
    if(stats.initialSpeed != 0)
    {
        stream << "Agility +" << static_cast<float>(stats.initialSpeed) / 100.f << '\n';
    }
    if(stats.maxSpeed != 0)
    {
        stream << "Top speed +" << static_cast<float>(stats.maxSpeed) / 10.f << '\n';
    }
    if(stats.shootSpeedCooldownReduction != 0)
    {
        stream << "Shoot cooldown -" << stats.shootSpeedCooldownReduction << " ms\n";
    }

    const std::string builtText = stream.str();
    if(builtText.empty())
    {
        return "A curious trinket with no direct stat bonus.";
    }

    return builtText;
}

void makeTextReadable(sf::Text& text, unsigned int characterSize, sf::Color fillColor, float outlineThickness)
{
    text.setCharacterSize(characterSize);
    text.setFillColor(fillColor);
    text.setOutlineThickness(outlineThickness);
    text.setOutlineColor(sf::Color(13, 18, 28, 230));
}

sf::FloatRect getInsetBounds(const sf::FloatRect& bounds, float insetRatioX, float insetRatioY)
{
    const float insetX = bounds.size.x * insetRatioX;
    const float insetY = bounds.size.y * insetRatioY;

    return {
        {bounds.position.x + insetX, bounds.position.y + insetY},
        {
            std::max(0.f, bounds.size.x - insetX * 2.f),
            std::max(0.f, bounds.size.y - insetY * 2.f)
        }
    };
}
}

void Shop::update()
{
    if(!isOpened) return;

    alignItemsOnGrid();
    updateHeaderTexts();
    updateItemFrameStates();
    if (hasItems())
    {
        onItemSelected();
    }

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

bool Shop::hasItems() const
{
    return !items.empty() && itemsIt != items.end();
}

void Shop::draw(sf::RenderWindow& window)
{
    if(!isOpened) return;

    window.draw(*sprite);
    window.draw(titleText);
    window.draw(goldText);

    for (auto &&item : items)
    {
        window.draw(item.first);
        item.second->draw(window);
    }
    
    widget.draw(window);
}

void Shop::initializeItems()
{
    std::fstream file(BASE_SHOP_DATA_PATH);

    if(!file.is_open())
    {
        throw std::runtime_error("Cannot load: " + (BASE_SHOP_DATA_PATH.string()));
    }

    nlohmann::json loadedData = nlohmann::json::parse(file);

    for (auto &&itemData : loadedData)
    {
        std::string iconName = itemData["IconName"];
        std::string itemName = itemData["ItemName"];
        Item::Quality quality = itemData["Quality"];
        int price = itemData["Price"];

        Item::Stats stats =
        {
            itemData["Stats"]["BulletSpeed"],
            itemData["Stats"]["BulletDistance"],
            itemData["Stats"]["ShootSpeedCooldownReduction"],
            itemData["Stats"]["InitialSpeed"],
            itemData["Stats"]["MaxSpeed"],
            itemData["Stats"]["Health"],
            itemData["Stats"]["Damage"]
        };

        addItem(std::make_unique<Item>(
            *this->data,
            static_cast<sf::Vector2i>(BASE_SHOP_CELL_SIZE),
            static_cast<sf::Vector2i>(getCenterPosition()),
            iconName,
            itemName,
            quality,
            price,
            stats
        ));
    }

    for (auto &&item : items)
    {
        setSpriteOriginToMiddle(item.first);
        item.first.setScale(BASE_SHOP_CELL_SPRITE_SCALE);
    }
}

void Shop::updateHeaderTexts()
{
    const sf::FloatRect contentBounds = getInsetBounds(
        sprite->getGlobalBounds(),
        BASE_SHOP_FRAME_INSET_RATIO_X,
        BASE_SHOP_FRAME_INSET_RATIO_Y
    );

    titleText.setString("Merchant's stock");
    titleText.setPosition({contentBounds.position.x, contentBounds.position.y});

    goldText.setString("Gold: " + std::to_string(player->getGold()));
    goldText.setPosition({
        contentBounds.position.x + contentBounds.size.x - goldText.getGlobalBounds().size.x,
        contentBounds.position.y
    });
}

void Shop::updateItemFrameStates()
{
    for (auto&& item : items)
    {
        item.first.setColor(getShopFrameColor(item.second->isPurchased(), false));
        item.first.setScale(BASE_SHOP_CELL_SPRITE_SCALE);

        item.second->setColor(item.second->isPurchased()
            ? sf::Color(150, 150, 150, 225)
            : sf::Color::White);
    }
}

void Shop::alignItemsOnGrid()
{
    if (items.empty())
    {
        return;
    }

    const sf::FloatRect contentBounds = getInsetBounds(
        sprite->getGlobalBounds(),
        BASE_SHOP_FRAME_INSET_RATIO_X,
        BASE_SHOP_FRAME_INSET_RATIO_Y
    );
    const float gridWidth = columns * cellSize.x + (columns - 1) * static_cast<float>(itemsMargin.x);
    const float gridStartX = contentBounds.position.x + std::max(0.f, (contentBounds.size.x - gridWidth) / 2.f);
    const float gridStartY = contentBounds.position.y + BASE_SHOP_HEADER_SECTION_HEIGHT + BASE_SHOP_GRID_SECTION_GAP;
    
    int iterationCount = 0;
    int countingForNextRow = 0;

    for (auto &&item : items)
    {
        sf::Vector2f nextPos = {
            gridStartX + (BASE_SHOP_CELL_SIZE.x / 2) +
            (BASE_SHOP_CELL_SIZE.x * iterationCount) +
            (itemsMargin.x * iterationCount),
            
            gridStartY + (BASE_SHOP_CELL_SIZE.y / 2) +
            (BASE_SHOP_CELL_SIZE.y * countingForNextRow) +
            (itemsMargin.y * countingForNextRow)
        };

        const sf::Vector2i convertedNextPos = static_cast<sf::Vector2i>(nextPos);
        item.second->setPosition(convertedNextPos);

        iterationCount++;

        if(iterationCount == columns)
        {
            countingForNextRow++;
            iterationCount = 0;
        }

        item.first.setPosition(item.second.get()->getCenterPosition());
    }
}

void Shop::onItemSelected()
{
    if (!hasItems())
    {
        return;
    }

    itemsIt->first.setScale(BASE_SHOP_CELL_SPRITE_SELECTED_SCALE);
    itemsIt->first.setColor(getShopFrameColor(itemsIt->second->isPurchased(), true));
}

void Shop::onSelectedChanged()
{
    if (!hasItems())
    {
        return;
    }

    itemsIt->first.setScale(BASE_SHOP_CELL_SPRITE_SCALE);
}

void Shop::onShopClosed()
{
    for (auto &&item : items)
    {
        item.second->setScale(item.second->getBaseScale());
    }

    isItemWidgetOpened = false;
    widget.close();
}

void Shop::onShopOpened()
{
    updateHeaderTexts();
}

bool Shop::buySelectedItem()
{
    if(!hasItems())
    {
        return false;
    }

    Item& selectedItem = *itemsIt->second;
    if(selectedItem.isPurchased())
    {
        return false;
    }

    if(!player->tryPurchaseItem(selectedItem))
    {
        return false;
    }

    selectedItem.markPurchased();
    selectedItem.setColor(sf::Color(150, 150, 150, 225));
    return true;
}

void Shop::moveSelectionRight()
{
    if (!hasItems())
    {
        return;
    }

    onSelectedChanged();

    itemsIt++;
    if(itemsIt == items.end())
    {
        itemsIt = items.begin();
    }
}

void Shop::moveSelectionLeft()
{
    if (!hasItems())
    {
        return;
    }

    onSelectedChanged();

    if (itemsIt == items.begin()) {
        itemsIt = std::prev(items.end());
    } else {
        itemsIt--;
    }
}

void Shop::moveSelectionDown()
{
    if (!hasItems())
    {
        return;
    }

    onSelectedChanged();
    
    const size_t currentIndex = itemsIt - items.begin();
    size_t newIndex = currentIndex + columns;
    
    if (newIndex >= items.size())
    {
        const size_t column = currentIndex % columns;
        newIndex = column;
        
        while (newIndex >= items.size())
        {
            newIndex--;
        }
    }
    
    itemsIt = items.begin() + newIndex;
}

void Shop::moveSelectionUp()
{
    if (!hasItems())
    {
        return;
    }

    onSelectedChanged();
    
    const size_t currentIndex = itemsIt - items.begin();
    
    if (currentIndex < columns)
    {
        const size_t column = currentIndex % columns;
        size_t newIndex = ((items.size() - 1) / columns) * columns + column;
        
        if (newIndex >= items.size())
        {
            newIndex = items.size() - 1;
            while (newIndex % columns > column)
            {
                newIndex--;
            }
        }
        
        itemsIt = items.begin() + newIndex;
    }
    else
    {
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
    if (!hasItems())
    {
        return;
    }

    isItemWidgetOpened = true;
    widget.attachItemStats(*itemsIt->second.get());
    widget.open();
}

void Shop::closeSelectedItemWidget()
{
    isItemWidgetOpened = false;
    widget.close();
}

Shop::Shop(GameData &d, Player &p, sf::Vector2f pos)
    : InteractiveObject(pos, d.guiTextures.at("GUI_10.png"))
    , player(&p)
    , data(&d)
    , widget(*this, d, BASE_SHOP_WIDGET_SPRITE_SCALE, pos, *d.gameFont)
    , columns(BASE_SHOP_COLUMNS)
    , rows(BASE_SHOP_ROWS)
    , itemsMargin(BASE_SHOP_ITEMS_MARGIN)
    , cellSize(BASE_SHOP_CELL_SIZE)
    , titleText(*d.gameFont)
    , goldText(*d.gameFont)
    , isOpened(false)
    , isItemWidgetOpened(false)
{   
    titleText.setCharacterSize(22);
    titleText.setFillColor(sf::Color(242, 237, 226));

    goldText.setCharacterSize(20);
    goldText.setFillColor(sf::Color(255, 215, 102));

    initializeItems();
    itemsIt = items.begin();
    updateBackgroundLayout(pos);
}

void Shop::open()
{
    isOpened = true;
    itemsIt = items.begin();
    onShopOpened();
    blockPlayerControl();
}

void Shop::close()
{
    isOpened = false;
    itemsIt = items.begin();
    onShopClosed();
    unblockPlayerControl();
}

void Shop::addItem(std::unique_ptr<Item> item)
{
    items.push_back(std::pair(sf::Sprite(data->guiTextures.at("GUI_04.png")),std::move(item)));
}

void Shop::updateBackgroundLayout(const sf::Vector2f& pos)
{
    const sf::Vector2u backTextureSize = sprite->getTexture().getSize();

    if (items.empty())
    {
        setScale({
            sprite->getScale().x + BASE_SHOP_BACKGROUND_ADDITIONAL_SCALE.x,
            sprite->getScale().y + BASE_SHOP_BACKGROUND_ADDITIONAL_SCALE.y
        });
        sprite->setPosition(pos);
        updateHeaderTexts();
        return;
    }

    const unsigned int requiredRows = static_cast<unsigned int>((items.size() + columns - 1) / columns);
    const float requiredGridWidth = cellSize.x * columns + itemsMargin.x * (columns - 1);
    const float requiredGridHeight = cellSize.y * requiredRows + itemsMargin.y * (requiredRows - 1);
    const float requiredInnerWidth = requiredGridWidth;
    const float requiredInnerHeight = BASE_SHOP_HEADER_SECTION_HEIGHT + BASE_SHOP_GRID_SECTION_GAP + requiredGridHeight;
    const float usableWidthRatio = 1.f - BASE_SHOP_FRAME_INSET_RATIO_X * 2.f;
    const float usableHeightRatio = 1.f - BASE_SHOP_FRAME_INSET_RATIO_Y * 2.f;

    sf::Vector2f finalScale = {
        requiredInnerWidth / (static_cast<float>(backTextureSize.x) * usableWidthRatio),
        requiredInnerHeight / (static_cast<float>(backTextureSize.y) * usableHeightRatio)
    };

    finalScale += BASE_SHOP_BACKGROUND_ADDITIONAL_SCALE;
    setScale(finalScale);
    setSpriteOriginToMiddle(*sprite);
    sprite->setPosition(pos);

    updateHeaderTexts();
    widget.setWidgetCenterPosition({pos.x, pos.y - 20.f});
}

Shop::ItemWidget::ItemWidget(Shop& owner, GameData& data, sf::Vector2f widgetScale, sf::Vector2f widgetPos, sf::Font& font)
    : owner(&owner)
    , background(data.guiTextures.at("GUI_17.png"))
    , displayNameText(font)
    , qualityText(font)
    , priceText(font)
    , statsText(font)
    , stateText(font)
    , hintText(font)
{
    background.setScale(widgetScale);
    setSpriteOriginToMiddle(background);
    background.setColor(sf::Color(130, 138, 156, 248));

    makeTextReadable(displayNameText, 23, sf::Color(245, 239, 227), 2.f);
    makeTextReadable(qualityText, 17, sf::Color(255, 215, 102), 2.f);
    makeTextReadable(priceText, 18, sf::Color(255, 215, 102), 2.f);
    makeTextReadable(statsText, 16, sf::Color(232, 234, 240), 1.5f);
    makeTextReadable(stateText, 18, sf::Color::White, 2.f);
    makeTextReadable(hintText, 15, sf::Color(198, 206, 222), 1.5f);

    setWidgetCenterPosition(widgetPos);
}

void Shop::ItemWidget::refreshState()
{
    if(!attachedItem || !owner || !owner->player)
    {
        return;
    }

    priceText.setString("Price: " + std::to_string(attachedItem->price) + " gold");

    if(attachedItem->isPurchased())
    {
        stateText.setString("Already purchased");
        stateText.setFillColor(sf::Color(132, 231, 170));
        hintText.setString("X - Close");
    }
    else if(owner->player->canAfford(attachedItem->price))
    {
        stateText.setString("You have enough gold");
        stateText.setFillColor(sf::Color(255, 215, 102));
        hintText.setString("Z - Buy    X - Close");
    }
    else
    {
        stateText.setString("Not enough gold");
        stateText.setFillColor(sf::Color(255, 135, 135));
        hintText.setString("X - Close");
    }

    updateLayout();
}

void Shop::ItemWidget::updateLayout()
{
    const sf::FloatRect contentBounds = getInsetBounds(
        background.getGlobalBounds(),
        BASE_SHOP_FRAME_INSET_RATIO_X,
        BASE_SHOP_FRAME_INSET_RATIO_Y
    );
    const float left = contentBounds.position.x;
    const float right = contentBounds.position.x + contentBounds.size.x;
    const float top = contentBounds.position.y;
    const float bottom = contentBounds.position.y + contentBounds.size.y;
    const float iconColumnWidth = hasItemIcon ? std::min(BASE_SHOP_WIDGET_ICON_COLUMN_WIDTH, contentBounds.size.x * 0.35f) : 0.f;
    const float iconCenterX = right - iconColumnWidth / 2.f;

    displayNameText.setPosition({left, top});
    qualityText.setPosition({left, top + 36.f});
    priceText.setPosition({left, top + 66.f});
    statsText.setPosition({left, top + 118.f});
    stateText.setPosition({left, bottom - 78.f});
    hintText.setPosition({left, bottom - 44.f});

    if(hasItemIcon)
    {
        itemIcon->setPosition({
            iconCenterX,
            top + 78.f
        });
    }
}

void Shop::ItemWidget::draw(sf::RenderWindow &window)
{
    if(!isOpened) return;
    
    window.draw(this->background);
    if(hasItemIcon)
    {
        window.draw(*itemIcon);
    }
    window.draw(this->displayNameText);
    window.draw(this->qualityText);
    window.draw(this->priceText);
    window.draw(this->statsText);
    window.draw(this->stateText);
    window.draw(this->hintText);
}

void Shop::ItemWidget::update()
{
    if(!isOpened) return;

    refreshState();
}

void Shop::ItemWidget::handleEvent(const sf::Event &event)
{
    if(!isOpened) return;

    if(const auto* keyPressed = event.getIf<sf::Event::KeyPressed>())
    {
        if(keyPressed->scancode == SHOP_KEY_TO_OPEN_ITEM_WIDGET)
        {
            buy(); 
        }
        else if(keyPressed->scancode == SHOP_KEY_TO_CLOSE_ITEM_WIDGET)
        {
            owner->closeSelectedItemWidget();
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
    if(!owner)
    {
        close();
        return;
    }

    if(owner->buySelectedItem())
    {
        owner->closeSelectedItemWidget();
        return;
    }

    refreshState();
}

bool Shop::ItemWidget::getIsOpened()
{
    return this->isOpened;
}

void Shop::ItemWidget::attachItemStats(Item& item)
{
    attachedItem = &item;
    displayNameText.setString(item.displayName);
    displayNameText.setFillColor(getQualityColor(item.quality));
    qualityText.setString(getQualityLabel(item.quality));
    qualityText.setFillColor(getQualityColor(item.quality));
    statsText.setString(buildStatsText(item.stats));

    itemIcon = std::make_unique<sf::Sprite>(item.getTexture());
    setSpriteOriginToMiddle(*itemIcon);
    itemIcon->setColor(item.isPurchased() ? sf::Color(190, 190, 190, 220) : sf::Color::White);

    const sf::Vector2u textureSize = item.getTexture().getSize();
    const sf::Vector2f maxIconSize = {76.f, 76.f};
    const float scaleX = maxIconSize.x / static_cast<float>(textureSize.x);
    const float scaleY = maxIconSize.y / static_cast<float>(textureSize.y);
    const float iconScale = std::min(scaleX, scaleY);
    itemIcon->setScale({iconScale, iconScale});
    hasItemIcon = true;

    refreshState();
}

void Shop::ItemWidget::setWidgetCenterPosition(sf::Vector2f position)
{
    background.setPosition(position);
    updateLayout();
}
