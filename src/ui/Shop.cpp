#include<Shop.h>
#include<Localization.h>

#include<nlohmann/json.hpp>

#include<algorithm>
#include<fstream>
#include<iomanip>
#include<sstream>
#include<stdexcept>
#include<unordered_map>

namespace
{
const std::unordered_map<std::string, std::pair<std::string, std::string>>& russianItemText()
{
    static const std::unordered_map<std::string, std::pair<std::string, std::string>> text = {
        {"Item_01.png", {"Карта троп", "Схема погребальных путей, прошитая серебряной нитью. Ваши заряды летят дальше, прежде чем тьма их проглотит."}},
        {"Item_03.png", {"Быстрая шпора", "Наездничья шпора, благословленная для коротких рывков по старым могильным дорогам."}},
        {"Item_04.png", {"Железный оберег", "Тяжелый и старый, но защищенный. Он помогает телу пережить еще несколько дурных пророчеств."}},
        {"Item_06.png", {"Угольные чернила", "Черно-красный пигмент, превращающий простые заряды в горящие сигилы при ударе."}},
        {"Item_08.png", {"Соколиное перо", "Все еще легкое настолько, что дрожит в мертвом воздухе. Движение становится резче и увереннее."}},
        {"Item_09.png", {"Охотничья линза", "Шлифованный кристалл, через который даже охотник на ведьм измерил бы дистанцию одним взглядом."}},
        {"Item_10.png", {"Руководство шквала", "Тонкий кодекс о том, как выпустить выстрел до того, как страх успеет лечь в руки."}},
        {"Item_12.png", {"Шипастый гримуар", "Поля исписаны жестокими заметками. Каждый снаряд попадает с более злой силой."}},
        {"Item_13.png", {"Печать стража", "Холодная печать забытого смотрителя крипты. Она укрепляет плоть и делает шаг ровнее."}},
        {"Item_14.png", {"Теневые шпоры", "Шпоры всадников, научившихся прорываться между надгробиями быстрее, чем луна их заметит."}},
        {"Item_15.png", {"Лунное сухожилие", "Упругое серебряное сухожилие, позволяющее прыжку чуть выше вгрызаться в ночь."}},
        {"Item_16.png", {"Плащ мотылька", "Сшит из крыльев пепельных мотыльков. Падение перестает быть капитуляцией и становится выбором."}},
        {"Item_17.png", {"Браслет двух клыков", "Ритуальный браслет, отвечающий панике еще одним прыжком, когда первый должен был стать последним."}},
        {"Item_18.png", {"Бездонная шпора", "Почерневшая шпора всадника, которого похоронили все еще пытающимся обогнать пророчество."}},
        {"Item_19.png", {"Соборные пружины", "Реликвии с заведенным напряжением из механизма колокольни. Дают еще один прыжок и более чистый взлет."}},
        {"Item_31.png", {"Пика Могилобоя", "Костяно-белый ритуал, который просверливает одного врага и летит дальше. Быстро, холодно и беспощадно."}},
        {"Item_32.png", {"Скипетр Пиросферы", "Выплевывает медленную угольную сферу, которая взрывается при касании и обжигает кости вокруг."}},
        {"Item_33.png", {"Штормовой игольник", "Три осколочных заряда за один вдох. Он ощущается зло быстрым и рисует экран электрическими полосами."}},
        {"Item_34.png", {"Призма ужаса", "Расколотый ритуальный фокус, веером выпускающий заряды в призрачный клин. Его владения - тесные коридоры."}},
        {"Item_35.png", {"Луч сумерек", "Финальный ритуал только для богатых охотников. Он прорезает комнату лазерным шрамом и прошивает целые ряды врагов."}}
    };
    return text;
}

std::string readLocalizedString(const nlohmann::json& data, const char* englishKey, const char* russianKey, const std::string& iconName)
{
    if (Localization::isRussian())
    {
        const std::string russianValue = data.value(russianKey, std::string{});
        if (!russianValue.empty())
        {
            return russianValue;
        }
        const auto it = russianItemText().find(iconName);
        if (it != russianItemText().end())
        {
            return std::string{englishKey} == "ItemName" ? it->second.first : it->second.second;
        }
    }

    return data.value(englishKey, std::string{});
}

std::string wrapTextByWords(const std::string& text, std::size_t maxLineLength)
{
    if (text.empty() || maxLineLength == 0)
    {
        return text;
    }

    std::istringstream words(text);
    std::ostringstream wrapped;
    std::string word;
    std::size_t currentLineLength = 0;

    while (words >> word)
    {
        const std::size_t requiredLength = currentLineLength == 0
            ? word.size()
            : currentLineLength + 1 + word.size();

        if (currentLineLength > 0 && requiredLength > maxLineLength)
        {
            wrapped << '\n' << word;
            currentLineLength = word.size();
            continue;
        }

        if (currentLineLength > 0)
        {
            wrapped << ' ';
            ++currentLineLength;
        }

        wrapped << word;
        currentLineLength += word.size();
    }

    return wrapped.str();
}

std::string wrapTextToWidth(const std::string& text, const sf::Text& styleSource, const float maxWidth)
{
    if (text.empty() || maxWidth <= 0.f)
    {
        return text;
    }

    sf::Text probe(styleSource);
    std::istringstream words(text);
    std::ostringstream wrapped;
    std::string word;
    std::string line;
    bool firstLine = true;

    auto textWidth = [&](const std::string& value) {
        Localization::setText(probe, value);
        return probe.getLocalBounds().size.x;
    };

    while (words >> word)
    {
        const std::string candidate = line.empty() ? word : line + " " + word;
        if (!line.empty() && textWidth(candidate) > maxWidth)
        {
            if (!firstLine)
            {
                wrapped << '\n';
            }
            wrapped << line;
            firstLine = false;
            line = word;
        }
        else
        {
            line = candidate;
        }
    }

    if (!line.empty())
    {
        if (!firstLine)
        {
            wrapped << '\n';
        }
        wrapped << line;
    }

    return wrapped.str();
}

float getTextBottom(const sf::Text& text)
{
    const sf::FloatRect bounds = text.getGlobalBounds();
    return bounds.position.y + bounds.size.y;
}

float getShopGridStartYOffset()
{
    return Localization::isRussian()
        ? 300.f
        : BASE_SHOP_HEADER_SECTION_HEIGHT + BASE_SHOP_GRID_SECTION_GAP + 34.f;
}

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
            return Localization::isRussian() ? Localization::tr("shop.common_relic") : "Common relic";
        case Item::RARE:
            return Localization::isRussian() ? Localization::tr("shop.rare_relic") : "Rare relic";
        case Item::MYTH:
            return Localization::isRussian() ? Localization::tr("shop.myth_relic") : "Myth relic";
        case Item::LEGENDARY:
            return Localization::isRussian() ? Localization::tr("shop.legendary_relic") : "Legendary relic";
    }

    return Localization::isRussian() ? Localization::tr("shop.relic") : "Relic";
}

std::string getCategoryLabel(Item::Category category)
{
    switch (category)
    {
        case Item::Category::Upgrade:
            return Localization::isRussian() ? Localization::tr("shop.relic_upgrade") : "Relic upgrade";
        case Item::Category::Weapon:
            return Localization::isRussian() ? Localization::tr("shop.weapon_rite") : "Weapon rite";
    }

    return Localization::isRussian() ? Localization::tr("shop.relic") : "Relic";
}

std::string getWeaponKindLabel(Item::WeaponKind kind)
{
    switch (kind)
    {
        case Item::WeaponKind::AshenBolt:
            return Localization::weaponName("Ashen Bolt");
        case Item::WeaponKind::Gravepiercer:
            return Localization::weaponName("Gravepiercer Pike");
        case Item::WeaponKind::PyreOrb:
            return Localization::weaponName("Pyre Orb Scepter");
        case Item::WeaponKind::StormNeedler:
            return Localization::weaponName("Storm Needler");
        case Item::WeaponKind::DreadPrism:
            return Localization::weaponName("Dread Prism");
        case Item::WeaponKind::NightfallBeam:
            return Localization::weaponName("Nightfall Beam");
    }

    return Localization::isRussian() ? Localization::tr("shop.relic") : "Relic";
}

std::string buildStatsText(const Item& item)
{
    std::ostringstream stream;
    stream << std::fixed << std::setprecision(2);

    if(item.category == Item::Category::Weapon)
    {
        stream << (Localization::isRussian() ? Localization::tr("shop.form") : "Form") << ": " << getWeaponKindLabel(item.weaponStats.kind) << '\n';
        stream << (Localization::isRussian() ? Localization::tr("shop.damage") : "Damage") << " +" << item.weaponStats.damageBonus << '\n';
        stream << (Localization::isRussian() ? Localization::tr("shop.cooldown") : "Cooldown") << " " << item.weaponStats.cooldownMs << " ms\n";
        stream << (Localization::isRussian() ? Localization::tr("shop.energy_cost") : "Energy cost") << " " << item.weaponStats.energyCost << '\n';
        if(item.weaponStats.projectileSpeed != 0)
        {
            stream << (Localization::isRussian() ? Localization::tr("shop.shot_speed") : "Shot speed") << " +" << item.weaponStats.projectileSpeed << '\n';
        }
        if(item.weaponStats.projectileRange != 0)
        {
            stream << (Localization::isRussian() ? Localization::tr("hud.range") : "Range") << " +" << item.weaponStats.projectileRange << '\n';
        }
        if(item.weaponStats.projectileCount > 1)
        {
            stream << (Localization::isRussian() ? Localization::tr("shop.projectiles") : "Projectiles") << " x" << item.weaponStats.projectileCount << '\n';
        }
        if(item.weaponStats.pierceCount > 0)
        {
            stream << (Localization::isRussian() ? Localization::tr("shop.pierce") : "Pierce") << " +" << item.weaponStats.pierceCount << '\n';
        }
        if(item.weaponStats.splashRadius > 0)
        {
            stream << (Localization::isRussian() ? Localization::tr("shop.splash") : "Splash") << " " << item.weaponStats.splashRadius << '\n';
        }
        if(item.weaponStats.spread > 0)
        {
            stream << (Localization::isRussian() ? Localization::tr("shop.fan_spread") : "Fan spread") << " " << item.weaponStats.spread << '\n';
        }

        return stream.str();
    }

    const Item::Stats& stats = item.stats;

    if(stats.health != 0)
    {
        stream << (Localization::isRussian() ? Localization::tr("shop.health") : "Health") << " +" << stats.health << '\n';
    }
    if(stats.damage != 0)
    {
        stream << (Localization::isRussian() ? Localization::tr("shop.damage") : "Damage") << " +" << stats.damage << '\n';
    }
    if(stats.bulletSpeed != 0)
    {
        stream << (Localization::isRussian() ? Localization::tr("shop.bullet_speed") : "Bullet speed") << " +" << stats.bulletSpeed << '\n';
    }
    if(stats.bulletDistance != 0)
    {
        stream << (Localization::isRussian() ? Localization::tr("shop.bullet_range") : "Bullet range") << " +" << stats.bulletDistance << '\n';
    }
    if(stats.initialSpeed != 0)
    {
        stream << (Localization::isRussian() ? Localization::tr("shop.agility") : "Agility") << " +" << static_cast<float>(stats.initialSpeed) / 100.f << '\n';
    }
    if(stats.maxSpeed != 0)
    {
        stream << (Localization::isRussian() ? Localization::tr("shop.top_speed") : "Top speed") << " +" << static_cast<float>(stats.maxSpeed) / 10.f << '\n';
    }
    if(stats.shootSpeedCooldownReduction != 0)
    {
        stream << (Localization::isRussian() ? Localization::tr("shop.shoot_cooldown") : "Shoot cooldown") << " -" << stats.shootSpeedCooldownReduction << " ms\n";
    }
    if(stats.dashForce != 0)
    {
        stream << (Localization::isRussian() ? Localization::tr("shop.dash_force") : "Dash force") << " +" << static_cast<float>(stats.dashForce) / 10.f << '\n';
    }
    if(stats.dashCooldownReduction != 0)
    {
        stream << (Localization::isRussian() ? Localization::tr("shop.dash_cooldown") : "Dash cooldown") << " -" << stats.dashCooldownReduction << " ms\n";
    }
    if(stats.extraJumpCount != 0)
    {
        stream << (Localization::isRussian() ? Localization::tr("shop.extra_jump") : "Extra jump") << " +" << stats.extraJumpCount << '\n';
    }
    if(stats.jumpPower != 0)
    {
        stream << (Localization::isRussian() ? Localization::tr("shop.jump_height") : "Jump height") << " +" << static_cast<float>(stats.jumpPower) / 10.f << '\n';
    }
    if(stats.slowFallPercent != 0)
    {
        stream << (Localization::isRussian() ? Localization::tr("shop.fall_speed") : "Fall speed") << " -" << stats.slowFallPercent << "%\n";
    }

    const std::string builtText = stream.str();
    if(builtText.empty())
    {
        return Localization::isRussian() ? Localization::tr("shop.no_bonus") : "A curious trinket with no direct stat bonus.";
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

std::vector<Shop::ShopEntry>& Shop::getActiveItems()
{
    return activeTab_ == ShopTab::Weapons ? weaponItems : upgradeItems;
}

const std::vector<Shop::ShopEntry>& Shop::getActiveItems() const
{
    return activeTab_ == ShopTab::Weapons ? weaponItems : upgradeItems;
}

Shop::ShopEntry* Shop::getSelectedEntry()
{
    auto& activeItems = getActiveItems();
    if (activeItems.empty())
    {
        return nullptr;
    }

    clampSelection();
    return &activeItems[selectedIndex_];
}

const Shop::ShopEntry* Shop::getSelectedEntry() const
{
    const auto& activeItems = getActiveItems();
    if (activeItems.empty())
    {
        return nullptr;
    }

    const std::size_t safeIndex = std::min(selectedIndex_, activeItems.size() - 1);
    return &activeItems[safeIndex];
}

void Shop::clampSelection()
{
    auto& activeItems = getActiveItems();
    if (activeItems.empty())
    {
        selectedIndex_ = 0;
        return;
    }

    selectedIndex_ = std::min(selectedIndex_, activeItems.size() - 1);
}

void Shop::setActiveTab(ShopTab tab)
{
    if (activeTab_ == tab)
    {
        clampSelection();
        return;
    }

    onSelectedChanged();
    activeTab_ = tab;
    selectedIndex_ = 0;
    closeSelectedItemWidget();
    clampSelection();
    updateBackgroundLayout(sprite->getPosition());
    alignItemsOnGrid();
}

void Shop::switchTab(int direction)
{
    setActiveTab(direction > 0 ? ShopTab::Weapons : ShopTab::Upgrades);
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

bool Shop::handleEvent(const sf::Event &event)
{
    if (!isOpened)
    {
        return false;
    }

    if(widget.getIsOpened())
    {
        widget.handleEvent(event);
        return true;
    }

    if(const auto* keyPressed = event.getIf<sf::Event::KeyPressed>())
    {
        if(keyPressed->scancode == SHOP_KEY_TO_PREVIOUS_TAB)
        {
            switchTab(-1);
            return true;
        }
        else if(keyPressed->scancode == SHOP_KEY_TO_NEXT_TAB)
        {
            switchTab(1);
            return true;
        }
        else if(keyPressed->scancode == SHOP_KEY_TO_MOVE_RIGHT)
        {
            moveSelectionRight();
            return true;
        }
        else if(keyPressed->scancode == SHOP_KEY_TO_MOVE_LEFT)
        {
            moveSelectionLeft();
            return true;
        }
        else if(keyPressed->scancode == SHOP_KEY_TO_MOVE_DOWN)
        {
            moveSelectionDown();
            return true;
        }
        else if(keyPressed->scancode == SHOP_KEY_TO_MOVE_UP)
        {
            moveSelectionUp();
            return true;
        }
        else if(keyPressed->scancode == SHOP_KEY_TO_OPEN_ITEM_WIDGET)
        {
            openSelectedItemWidget();
            return true;
        }
    }

    return false;
}

bool Shop::getIsOpened()
{
    return this->isOpened;
}

bool Shop::blocksPlayerInput() const
{
    return isOpened;
}

bool Shop::hasItems() const
{
    return !getActiveItems().empty();
}

void Shop::draw(sf::RenderWindow& window)
{
    if(!isOpened) return;

    window.draw(*sprite);
    window.draw(upgradesTabPlate);
    window.draw(weaponsTabPlate);
    window.draw(titleText);
    window.draw(goldText);
    window.draw(merchantTitleText);
    window.draw(merchantAdviceText);
    window.draw(routeHintText);
    window.draw(tabHintText);
    window.draw(upgradesTabText);
    window.draw(weaponsTabText);

    for (auto &&item : getActiveItems())
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
        std::string itemName = readLocalizedString(itemData, "ItemName", "ItemNameRu", iconName);
        Item::Quality quality = itemData["Quality"];
        const std::string categoryName = itemData.value("Category", "Upgrade");
        const Item::Category category = categoryName == "Weapon"
            ? Item::Category::Weapon
            : Item::Category::Upgrade;
        int price = itemData["Price"];
        const std::string description = readLocalizedString(itemData, "Description", "DescriptionRu", iconName);

        Item::Stats stats =
        {
            itemData["Stats"]["BulletSpeed"],
            itemData["Stats"]["BulletDistance"],
            itemData["Stats"]["ShootSpeedCooldownReduction"],
            itemData["Stats"]["InitialSpeed"],
            itemData["Stats"]["MaxSpeed"],
            itemData["Stats"]["Health"],
            itemData["Stats"]["Damage"],
            itemData["Stats"].value("DashForce", 0),
            itemData["Stats"].value("DashCooldownReduction", 0),
            itemData["Stats"].value("ExtraJumpCount", 0),
            itemData["Stats"].value("JumpPower", 0),
            itemData["Stats"].value("SlowFallPercent", 0)
        };

        Item::WeaponStats weaponStats{};
        if (category == Item::Category::Weapon && itemData.contains("Weapon"))
        {
            const auto& weaponData = itemData["Weapon"];
            const std::string kindName = weaponData.value("Kind", "AshenBolt");

            if (kindName == "Gravepiercer")
            {
                weaponStats.kind = Item::WeaponKind::Gravepiercer;
            }
            else if (kindName == "PyreOrb")
            {
                weaponStats.kind = Item::WeaponKind::PyreOrb;
            }
            else if (kindName == "StormNeedler")
            {
                weaponStats.kind = Item::WeaponKind::StormNeedler;
            }
            else if (kindName == "DreadPrism")
            {
                weaponStats.kind = Item::WeaponKind::DreadPrism;
            }
            else if (kindName == "NightfallBeam")
            {
                weaponStats.kind = Item::WeaponKind::NightfallBeam;
            }
            else
            {
                weaponStats.kind = Item::WeaponKind::AshenBolt;
            }

            weaponStats.damageBonus = weaponData.value("DamageBonus", 0);
            weaponStats.cooldownMs = weaponData.value("CooldownMs", 0);
            weaponStats.energyCost = weaponData.value("EnergyCost", 0);
            weaponStats.projectileSpeed = weaponData.value("ProjectileSpeed", 0);
            weaponStats.projectileRange = weaponData.value("ProjectileRange", 0);
            weaponStats.projectileCount = weaponData.value("ProjectileCount", 1);
            weaponStats.pierceCount = weaponData.value("PierceCount", 0);
            weaponStats.splashRadius = weaponData.value("SplashRadius", 0);
            weaponStats.spread = weaponData.value("Spread", 0);
        }

        addItem(std::make_unique<Item>(
            *this->data,
            static_cast<sf::Vector2i>(BASE_SHOP_CELL_SIZE),
            static_cast<sf::Vector2i>(getCenterPosition()),
            iconName,
            itemName,
            quality,
            category,
            price,
            stats,
            weaponStats,
            description
        ));
    }

    for (auto &&item : upgradeItems)
    {
        setSpriteOriginToMiddle(item.first);
        item.first.setScale(BASE_SHOP_CELL_SPRITE_SCALE);
    }

    for (auto &&item : weaponItems)
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
    const CampaignProgress& campaign = player->getCampaignProgress();
    const CampaignLevelInfo& recommendedLevel = campaign.getRecommendedLevelInfo();

    Localization::setText(titleText, activeTab_ == ShopTab::Weapons
        ? (Localization::isRussian() ? Localization::tr("shop.arsenal") : "Arsenal of rites")
        : (Localization::isRussian() ? Localization::tr("shop.stock") : "Merchant's stock"));
    titleText.setPosition({contentBounds.position.x, contentBounds.position.y});

    Localization::setText(goldText, (Localization::isRussian() ? Localization::tr("shop.gold") : "Gold") + std::string(": ") + std::to_string(player->getGold()));
    goldText.setPosition({
        contentBounds.position.x + contentBounds.size.x - goldText.getGlobalBounds().size.x,
        contentBounds.position.y
    });

    float cursorY = contentBounds.position.y + 34.f;
    const float textMaxWidth = std::max(160.f, contentBounds.size.x - 8.f);

    Localization::setText(merchantTitleText, wrapTextToWidth(campaign.getMerchantGreeting(), merchantTitleText, textMaxWidth));
    merchantTitleText.setPosition({contentBounds.position.x, cursorY});
    cursorY = getTextBottom(merchantTitleText) + 7.f;

    Localization::setText(merchantAdviceText, wrapTextToWidth(campaign.getMerchantAdvice(), merchantAdviceText, textMaxWidth));
    merchantAdviceText.setPosition({contentBounds.position.x, cursorY});
    cursorY = getTextBottom(merchantAdviceText) + 9.f;

    Localization::setText(routeHintText, wrapTextToWidth(
        (Localization::isRussian() ? Localization::tr("shop.next_route") : "Next marked route") + std::string(": ") + recommendedLevel.title,
        routeHintText,
        textMaxWidth));
    routeHintText.setPosition({contentBounds.position.x, cursorY});
    cursorY = getTextBottom(routeHintText) + 12.f;

    Localization::setText(upgradesTabText, Localization::isRussian() ? Localization::tr("shop.relics") : "Relics");
    Localization::setText(weaponsTabText, Localization::isRussian() ? Localization::tr("shop.weapons") : "Weapons");
    Localization::setText(tabHintText, Localization::isRussian() ? Localization::tr("shop.tabs") : "Q/W - Tabs");

    const float maxTabsY = contentBounds.position.y + getShopGridStartYOffset() - 110.f;
    const sf::Vector2f tabsOrigin = {contentBounds.position.x, std::min(cursorY, maxTabsY)};
    const sf::Vector2f tabSize = {124.f, 28.f};
    const bool upgradesActive = activeTab_ == ShopTab::Upgrades;

    upgradesTabPlate.setPosition(tabsOrigin);
    upgradesTabPlate.setSize(tabSize);
    upgradesTabPlate.setFillColor(upgradesActive ? sf::Color(178, 61, 77, 230) : sf::Color(44, 28, 36, 215));
    upgradesTabPlate.setOutlineThickness(1.f);
    upgradesTabPlate.setOutlineColor(sf::Color(146, 94, 76, 210));

    weaponsTabPlate.setPosition({tabsOrigin.x + tabSize.x + 10.f, tabsOrigin.y});
    weaponsTabPlate.setSize(tabSize);
    weaponsTabPlate.setFillColor(!upgradesActive ? sf::Color(178, 61, 77, 230) : sf::Color(44, 28, 36, 215));
    weaponsTabPlate.setOutlineThickness(1.f);
    weaponsTabPlate.setOutlineColor(sf::Color(146, 94, 76, 210));

    upgradesTabText.setPosition({tabsOrigin.x + 18.f, tabsOrigin.y + 4.f});
    weaponsTabText.setPosition({tabsOrigin.x + tabSize.x + 28.f, tabsOrigin.y + 4.f});
    const float tabsRight = weaponsTabPlate.getPosition().x + weaponsTabPlate.getSize().x;
    const float hintWidth = tabHintText.getGlobalBounds().size.x;
    const float hintX = contentBounds.position.x + contentBounds.size.x - hintWidth;
    if (hintX > tabsRight + 14.f)
    {
        tabHintText.setPosition({hintX, tabsOrigin.y + 5.f});
    }
    else
    {
        tabHintText.setPosition({
            contentBounds.position.x + contentBounds.size.x - hintWidth,
            tabsOrigin.y + tabSize.y + 6.f
        });
    }
}

void Shop::updateItemFrameStates()
{
    for (auto&& item : upgradeItems)
    {
        item.first.setColor(getShopFrameColor(item.second->isPurchased(), false));
        item.first.setScale(BASE_SHOP_CELL_SPRITE_SCALE);

        item.second->setColor(item.second->isPurchased()
            ? sf::Color(150, 150, 150, 225)
            : sf::Color::White);
    }

    for (auto&& item : weaponItems)
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
    auto& items = getActiveItems();
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
    const float gridStartY = contentBounds.position.y + getShopGridStartYOffset();
    
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
    ShopEntry* selectedEntry = getSelectedEntry();
    if (!selectedEntry)
    {
        return;
    }

    selectedEntry->first.setScale(BASE_SHOP_CELL_SPRITE_SELECTED_SCALE);
    selectedEntry->first.setColor(getShopFrameColor(selectedEntry->second->isPurchased(), true));
}

void Shop::onSelectedChanged()
{
    ShopEntry* selectedEntry = getSelectedEntry();
    if (!selectedEntry)
    {
        return;
    }

    selectedEntry->first.setScale(BASE_SHOP_CELL_SPRITE_SCALE);
}

void Shop::onShopClosed()
{
    for (auto &&item : upgradeItems)
    {
        item.second->setScale(item.second->getBaseScale());
    }

    for (auto &&item : weaponItems)
    {
        item.second->setScale(item.second->getBaseScale());
    }

    isItemWidgetOpened = false;
    widget.close();
}

void Shop::onShopOpened()
{
    clampSelection();
    updateBackgroundLayout(sprite->getPosition());
    alignItemsOnGrid();
}

bool Shop::buySelectedItem()
{
    ShopEntry* selectedEntry = getSelectedEntry();
    if(!selectedEntry)
    {
        return false;
    }

    Item& selectedItem = *selectedEntry->second;
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
    auto& items = getActiveItems();
    if (items.empty())
    {
        return;
    }

    onSelectedChanged();

    selectedIndex_ = (selectedIndex_ + 1) % items.size();
}

void Shop::moveSelectionLeft()
{
    auto& items = getActiveItems();
    if (items.empty())
    {
        return;
    }

    onSelectedChanged();

    selectedIndex_ = selectedIndex_ == 0 ? items.size() - 1 : selectedIndex_ - 1;
}

void Shop::moveSelectionDown()
{
    auto& items = getActiveItems();
    if (items.empty())
    {
        return;
    }

    onSelectedChanged();
    
    const size_t currentIndex = selectedIndex_;
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
    
    selectedIndex_ = newIndex;
}

void Shop::moveSelectionUp()
{
    auto& items = getActiveItems();
    if (items.empty())
    {
        return;
    }

    onSelectedChanged();
    
    const size_t currentIndex = selectedIndex_;
    
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
        
        selectedIndex_ = newIndex;
    }
    else
    {
        selectedIndex_ -= columns;
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
    ShopEntry* selectedEntry = getSelectedEntry();
    if (!selectedEntry)
    {
        return;
    }

    isItemWidgetOpened = true;
    widget.attachItemStats(*selectedEntry->second.get());
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
    , tabHintText(*d.gameFont)
    , merchantTitleText(*d.gameFont)
    , merchantAdviceText(*d.gameFont)
    , routeHintText(*d.gameFont)
    , upgradesTabText(*d.gameFont)
    , weaponsTabText(*d.gameFont)
    , isOpened(false)
    , isItemWidgetOpened(false)
{   
    titleText.setCharacterSize(22);
    titleText.setFillColor(sf::Color(242, 237, 226));

    goldText.setCharacterSize(20);
    goldText.setFillColor(sf::Color(255, 215, 102));

    tabHintText.setCharacterSize(15);
    tabHintText.setFillColor(sf::Color(188, 195, 212));

    merchantTitleText.setCharacterSize(15);
    merchantTitleText.setFillColor(sf::Color(255, 205, 130));

    merchantAdviceText.setCharacterSize(14);
    merchantAdviceText.setFillColor(sf::Color(215, 220, 232));
    merchantAdviceText.setLineSpacing(1.05f);

    routeHintText.setCharacterSize(13);
    routeHintText.setFillColor(sf::Color(151, 190, 229));

    upgradesTabText.setCharacterSize(17);
    upgradesTabText.setFillColor(sf::Color(242, 237, 226));

    weaponsTabText.setCharacterSize(17);
    weaponsTabText.setFillColor(sf::Color(242, 237, 226));

    initializeItems();
    clampSelection();
    updateBackgroundLayout(pos);
}

void Shop::open()
{
    isOpened = true;
    selectedIndex_ = 0;
    onShopOpened();
    blockPlayerControl();
}

void Shop::close()
{
    isOpened = false;
    selectedIndex_ = 0;
    onShopClosed();
    unblockPlayerControl();
}

void Shop::addItem(std::unique_ptr<Item> item)
{
    if (!item)
    {
        return;
    }

    ShopEntry entry{sf::Sprite(data->guiTextures.at("GUI_04.png")), std::move(item)};
    if (entry.second->category == Item::Category::Weapon)
    {
        weaponItems.push_back(std::move(entry));
    }
    else
    {
        upgradeItems.push_back(std::move(entry));
    }
}

void Shop::updateBackgroundLayout(const sf::Vector2f& pos)
{
    const sf::Vector2u backTextureSize = sprite->getTexture().getSize();

    const std::size_t maxItems = std::max(upgradeItems.size(), weaponItems.size());

    if (maxItems == 0)
    {
        setScale({
            sprite->getScale().x + BASE_SHOP_BACKGROUND_ADDITIONAL_SCALE.x,
            sprite->getScale().y + BASE_SHOP_BACKGROUND_ADDITIONAL_SCALE.y
        });
        sprite->setPosition(pos);
        updateHeaderTexts();
        return;
    }

    const unsigned int requiredRows = static_cast<unsigned int>((maxItems + columns - 1) / columns);
    const float requiredGridWidth = cellSize.x * columns + itemsMargin.x * (columns - 1);
    const float requiredGridHeight = cellSize.y * requiredRows + itemsMargin.y * (requiredRows - 1);
    const float requiredInnerWidth = requiredGridWidth;
    const float requiredInnerHeight = getShopGridStartYOffset() + requiredGridHeight;
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
    , categoryText(font)
    , qualityText(font)
    , priceText(font)
    , statsText(font)
    , descriptionText(font)
    , stateText(font)
    , hintText(font)
{
    background.setScale(widgetScale);
    setSpriteOriginToMiddle(background);
    background.setColor(sf::Color(130, 138, 156, 248));

    makeTextReadable(displayNameText, 23, sf::Color(245, 239, 227), 2.f);
    makeTextReadable(categoryText, 15, sf::Color(196, 204, 220), 1.5f);
    makeTextReadable(qualityText, 17, sf::Color(255, 215, 102), 2.f);
    makeTextReadable(priceText, 18, sf::Color(255, 215, 102), 2.f);
    makeTextReadable(statsText, 16, sf::Color(232, 234, 240), 1.5f);
    makeTextReadable(descriptionText, 15, sf::Color(216, 221, 231), 1.2f);
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

    Localization::setText(priceText, (Localization::isRussian() ? Localization::tr("shop.price") : "Price") +
        std::string(": ") + std::to_string(attachedItem->price) +
        (Localization::isRussian() ? " золота" : " gold"));

    if(attachedItem->isPurchased())
    {
        Localization::setText(stateText, Localization::isRussian() ? Localization::tr("shop.already_purchased") : "Already purchased");
        stateText.setFillColor(sf::Color(132, 231, 170));
        Localization::setText(hintText, Localization::isRussian() ? Localization::tr("shop.close_hint") : "X - Close");
    }
    else if(owner->player->canAfford(attachedItem->price))
    {
        Localization::setText(stateText, Localization::isRussian() ? Localization::tr("shop.enough_gold") : "You have enough gold");
        stateText.setFillColor(sf::Color(255, 215, 102));
        Localization::setText(hintText, attachedItem->category == Item::Category::Weapon
            ? (Localization::isRussian() ? Localization::tr("shop.buy_equip_hint") : "Z - Buy and equip    X - Close")
            : (Localization::isRussian() ? Localization::tr("shop.buy_hint") : "Z - Buy    X - Close"));
    }
    else
    {
        Localization::setText(stateText, Localization::isRussian() ? Localization::tr("shop.not_enough_gold") : "Not enough gold");
        stateText.setFillColor(sf::Color(255, 135, 135));
        Localization::setText(hintText, Localization::isRussian() ? Localization::tr("shop.close_hint") : "X - Close");
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
    const bool isWeapon = attachedItem != nullptr && attachedItem->category == Item::Category::Weapon;
    const float maxIconColumnWidth = isWeapon ? 78.f : BASE_SHOP_WIDGET_ICON_COLUMN_WIDTH;
    const float iconColumnRatio = isWeapon ? 0.26f : 0.35f;
    const float iconColumnWidth = hasItemIcon ? std::min(maxIconColumnWidth, contentBounds.size.x * iconColumnRatio) : 0.f;
    const float textRight = hasItemIcon ? right - iconColumnWidth - (isWeapon ? 10.f : 18.f) : right;
    const float textWidth = std::max(isWeapon ? 190.f : 120.f, textRight - left);
    const float iconCenterX = right - iconColumnWidth / 2.f;

    if (attachedItem != nullptr)
    {
        Localization::setText(displayNameText, wrapTextToWidth(attachedItem->displayName, displayNameText, textWidth));
        Localization::setText(descriptionText, wrapTextToWidth(attachedItem->description, descriptionText, textWidth));
    }

    const float smallGap = isWeapon ? 5.f : 7.f;
    float cursorY = top;

    displayNameText.setPosition({left, cursorY});
    cursorY = getTextBottom(displayNameText) + smallGap;

    categoryText.setPosition({left, cursorY});
    cursorY = getTextBottom(categoryText) + smallGap;

    qualityText.setPosition({left, cursorY});
    cursorY = getTextBottom(qualityText) + smallGap;

    priceText.setPosition({left, cursorY});
    cursorY = getTextBottom(priceText) + (isWeapon ? 9.f : 12.f);

    descriptionText.setPosition({left, cursorY});
    cursorY = getTextBottom(descriptionText) + (isWeapon ? 10.f : 16.f);

    statsText.setPosition({left, cursorY});
    float statsBottom = getTextBottom(statsText);
    const float stateReserve = isWeapon ? 66.f : 78.f;
    if (isWeapon && statsBottom > bottom - stateReserve - 8.f)
    {
        statsText.setCharacterSize(11);
        statsText.setLineSpacing(0.88f);
        statsText.setPosition({left, cursorY});
        statsBottom = getTextBottom(statsText);
    }

    const float stateTop = std::min(bottom - stateReserve, std::max(statsBottom + 12.f, bottom - stateReserve));
    stateText.setPosition({left, stateTop});
    hintText.setPosition({left, stateTop + (isWeapon ? 29.f : 34.f)});

    if(hasItemIcon)
    {
        itemIcon->setPosition({
            iconCenterX,
            top + (isWeapon ? 70.f : 78.f)
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
    window.draw(this->categoryText);
    window.draw(this->qualityText);
    window.draw(this->priceText);
    window.draw(this->descriptionText);
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
    const bool isWeapon = item.category == Item::Category::Weapon;

    background.setScale(isWeapon ? sf::Vector2f{9.4f, 7.45f} : BASE_SHOP_WIDGET_SPRITE_SCALE);
    setSpriteOriginToMiddle(background);

    displayNameText.setCharacterSize(isWeapon ? 19 : 23);
    categoryText.setCharacterSize(isWeapon ? 13 : 15);
    qualityText.setCharacterSize(isWeapon ? 14 : 17);
    priceText.setCharacterSize(isWeapon ? 15 : 18);
    statsText.setCharacterSize(isWeapon ? 12 : 16);
    descriptionText.setCharacterSize(isWeapon ? 13 : 15);
    stateText.setCharacterSize(isWeapon ? 15 : 18);
    hintText.setCharacterSize(isWeapon ? 13 : 15);
    statsText.setLineSpacing(isWeapon ? 0.9f : 1.f);

    Localization::setText(displayNameText, item.displayName);
    displayNameText.setFillColor(sf::Color(245, 239, 227));
    Localization::setText(categoryText, getCategoryLabel(item.category));
    Localization::setText(qualityText, getQualityLabel(item.quality));
    qualityText.setFillColor(getQualityColor(item.quality));
    priceText.setFillColor(sf::Color(255, 215, 102));
    Localization::setText(statsText, buildStatsText(item));
    Localization::setText(descriptionText, item.description);
    descriptionText.setLineSpacing(isWeapon ? 1.05f : 1.15f);

    itemIcon = std::make_unique<sf::Sprite>(item.getTexture());
    setSpriteOriginToMiddle(*itemIcon);
    itemIcon->setColor(item.isPurchased() ? sf::Color(190, 190, 190, 220) : sf::Color::White);

    const sf::Vector2u textureSize = item.getTexture().getSize();
    const sf::Vector2f maxIconSize = isWeapon ? sf::Vector2f{58.f, 58.f} : sf::Vector2f{76.f, 76.f};
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
