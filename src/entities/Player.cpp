#include<Player.h>
#include<ScreenTransition.h>
#include<GameCamera.h>
#include<VisualEffects.h>

#include <algorithm>
#include <cmath>
#include <exception>

using namespace gameUtils;

namespace
{
constexpr float kPi = 3.14159265f;
const char* kPlayerProgressPath = "data/playerProgress.json";

std::string categoryToString(Item::Category category)
{
    return category == Item::Category::Weapon ? "Weapon" : "Upgrade";
}

Item::Category categoryFromString(const std::string& value)
{
    return value == "Weapon" ? Item::Category::Weapon : Item::Category::Upgrade;
}

std::string weaponKindToString(Item::WeaponKind kind)
{
    switch (kind)
    {
    case Item::WeaponKind::AshenBolt:
        return "AshenBolt";
    case Item::WeaponKind::Gravepiercer:
        return "Gravepiercer";
    case Item::WeaponKind::PyreOrb:
        return "PyreOrb";
    case Item::WeaponKind::StormNeedler:
        return "StormNeedler";
    case Item::WeaponKind::DreadPrism:
        return "DreadPrism";
    case Item::WeaponKind::NightfallBeam:
        return "NightfallBeam";
    }

    return "AshenBolt";
}

Item::WeaponKind weaponKindFromString(const std::string& value)
{
    if (value == "Gravepiercer")
    {
        return Item::WeaponKind::Gravepiercer;
    }
    if (value == "PyreOrb")
    {
        return Item::WeaponKind::PyreOrb;
    }
    if (value == "StormNeedler")
    {
        return Item::WeaponKind::StormNeedler;
    }
    if (value == "DreadPrism")
    {
        return Item::WeaponKind::DreadPrism;
    }
    if (value == "NightfallBeam")
    {
        return Item::WeaponKind::NightfallBeam;
    }

    return Item::WeaponKind::AshenBolt;
}

nlohmann::json statsToJson(const Item::Stats& stats)
{
    return nlohmann::json{
        {"bulletSpeed", stats.bulletSpeed},
        {"bulletDistance", stats.bulletDistance},
        {"shootSpeedCooldownReduction", stats.shootSpeedCooldownReduction},
        {"initialSpeed", stats.initialSpeed},
        {"maxSpeed", stats.maxSpeed},
        {"health", stats.health},
        {"damage", stats.damage},
        {"dashForce", stats.dashForce},
        {"dashCooldownReduction", stats.dashCooldownReduction},
        {"extraJumpCount", stats.extraJumpCount},
        {"jumpPower", stats.jumpPower},
        {"slowFallPercent", stats.slowFallPercent}
    };
}

Item::Stats statsFromJson(const nlohmann::json& data)
{
    Item::Stats stats;
    stats.bulletSpeed = data.value("bulletSpeed", 0);
    stats.bulletDistance = data.value("bulletDistance", 0);
    stats.shootSpeedCooldownReduction = data.value("shootSpeedCooldownReduction", 0);
    stats.initialSpeed = data.value("initialSpeed", 0);
    stats.maxSpeed = data.value("maxSpeed", 0);
    stats.health = data.value("health", 0);
    stats.damage = data.value("damage", 0);
    stats.dashForce = data.value("dashForce", 0);
    stats.dashCooldownReduction = data.value("dashCooldownReduction", 0);
    stats.extraJumpCount = data.value("extraJumpCount", 0);
    stats.jumpPower = data.value("jumpPower", 0);
    stats.slowFallPercent = data.value("slowFallPercent", 0);
    return stats;
}

nlohmann::json weaponStatsToJson(const Item::WeaponStats& stats)
{
    return nlohmann::json{
        {"kind", weaponKindToString(stats.kind)},
        {"damageBonus", stats.damageBonus},
        {"cooldownMs", stats.cooldownMs},
        {"energyCost", stats.energyCost},
        {"projectileSpeed", stats.projectileSpeed},
        {"projectileRange", stats.projectileRange},
        {"projectileCount", stats.projectileCount},
        {"pierceCount", stats.pierceCount},
        {"splashRadius", stats.splashRadius},
        {"spread", stats.spread}
    };
}

Item::WeaponStats weaponStatsFromJson(const nlohmann::json& data)
{
    Item::WeaponStats stats;
    stats.kind = weaponKindFromString(data.value("kind", "AshenBolt"));
    stats.damageBonus = data.value("damageBonus", 0);
    stats.cooldownMs = data.value("cooldownMs", 0);
    stats.energyCost = data.value("energyCost", 0);
    stats.projectileSpeed = data.value("projectileSpeed", 0);
    stats.projectileRange = data.value("projectileRange", 0);
    stats.projectileCount = data.value("projectileCount", 1);
    stats.pierceCount = data.value("pierceCount", 0);
    stats.splashRadius = data.value("splashRadius", 0);
    stats.spread = data.value("spread", 0);
    return stats;
}

nlohmann::json ownedItemToJson(const Player::OwnedItem& item)
{
    return nlohmann::json{
        {"iconName", item.iconName},
        {"displayName", item.displayName},
        {"quality", item.quality},
        {"category", categoryToString(item.category)},
        {"price", item.price},
        {"description", item.description},
        {"stats", statsToJson(item.stats)},
        {"weaponStats", weaponStatsToJson(item.weaponStats)}
    };
}

Player::OwnedItem ownedItemFromJson(const nlohmann::json& data)
{
    Player::OwnedItem item;
    item.iconName = data.value("iconName", "");
    item.displayName = data.value("displayName", item.iconName);
    item.quality = static_cast<Item::Quality>(data.value("quality", static_cast<int>(Item::COMMON)));
    item.category = categoryFromString(data.value("category", "Upgrade"));
    item.price = data.value("price", 0);
    item.description = data.value("description", "");
    item.stats = statsFromJson(data.value("stats", nlohmann::json::object()));
    item.weaponStats = weaponStatsFromJson(data.value("weaponStats", nlohmann::json::object()));
    return item;
}
}

Player::Player(GameData& gameTextures, GameLevelManager& m, GameCamera& c, sf::RenderWindow& w)
    : 
    CDMenu(
        gameTextures,
        c,
        m,
        *this,
        BASE_CHOOSEDESTINATIONMENU_MOVELEFT_KEY,
        BASE_CHOOSEDESTINATIONMENU_MOVERIGHT_KEY,
        BASE_CHOOSEDESTINATIONMENU_SELECT_KEY,
        BASE_CHOOSEDESTINATIONMENU_OPEN_CLOSE_KEY
    )
{
    this->gameTextures = &gameTextures;
    this->levelManager = &m;
    this->camera = &c;
    // Loading default config, then restoring live progress if it exists
    loadData();
    initializeDefaultWeapon();
    loadProgressData();
    recalculateStatsFromInventory();

    // Attaching levels to ChooseDestinationMenu
    for (auto &&i : m.getLevelsMap())
    {
        CDMenu.addLevelInVector(*i.second,i.second->getLevelBackgroundSprite().getTexture());
    }

    //Textures initialization
    attachTexture(gameTextures.idleTextures,this->idleTextures);
    attachTexture(gameTextures.runningTextures,this->runningTextures);
    attachTexture(gameTextures.fallingTextures,this->fallingTextures);
    attachTexture(gameTextures.satiro_dieTextures,this->satiro_dieTextures,gameTextures.satiro_die_helper,this->satiro_die_helper);
    attachTexture(gameTextures.satiro_hurtTextures,this->satiro_hurtTextures,gameTextures.satiro_hurt_helper,this->satiro_hurt_helper);
    attachTexture(gameTextures.satiro_dashTextures,this->satiro_dashTextures,gameTextures.satiro_dash_helper,this->satiro_dash_helper);
    attachTexture(gameTextures.satiro_slideTextures,this->satiro_slideTextures,gameTextures.satiro_slide_helper,this->satiro_slide_helper);
    attachTexture(gameTextures.satiro_jumpTextures,this->satiro_jumpTextures,gameTextures.satiro_jump_helper,this->satiro_jump_helper);
    attachTexture(gameTextures.satiro_landingTextures,this->satiro_landingTextures,gameTextures.satiro_landing_helper,this->satiro_landing_helper);

    //Rectangles initialization
    playerRectangle_ = new sf::RectangleShape();
    playerRectangle_->setSize({37.f,53.f});
    playerRectangle_->setFillColor(sf::Color::Red);
    playerRectangle_->setPosition({playerPosX_m,playerPosY_m});
    //Sprite initialization
    playerSprite = new sf::Sprite(idleTextures->at(0));
    setSpriteOriginToMiddle(*playerSprite);
    //Trail initialization
    trail = new Trail(*playerSprite);
    //Transition initialization
    this->transition = std::make_shared<ScreenTransition>(w,c,0.5f);
    //Portal initizalization
    portal = new LevelPortal({0.0,0.0},BASE_PORTAL_SPEED_OF_OPENING,BASE_PORTAL_SPEED_OF_CLOSING,portalExistTime,*playerSprite,*playerRectangle_,gameTextures,m,*transition);
    portalCallCloseCooldownClock.reset();
    portalCallOpenCooldownClock.reset();

    runEffectClock_.restart();
    teleportEffectClock_.restart();
    criticalEffectClock_.restart();
    landingEffectClock_.restart();
}

void Player::switchToNextFallingSprite()
{
    static int fps = 1;
    if(fps!=WINDOW_FPS/12)
    {   
        fps++;
        return;
    }
    static size_t i = 0;
    playerSprite->setTexture(fallingTextures->at(i),true);
    i++;
    if(i == fallingTextures->size()-1)
    {
        i=0;
    }
    
    fps=1;   
}

sf::Vector2f Player::getSpriteScale()
{
    return playerSprite->getScale();
}

sf::Vector2f Player::getCenterPosition()
{
    return playerRectangle_->getGlobalBounds().getCenter();
}

sf::Clock& Player::getShootClock()
{
    return shootTimer;
}

int &Player::getShootCooldown()
{
    return ButtonRepeat_shootCooldown;
}

sf::Clock &Player::getDashClock()
{
    return dash_Clock;
}

int &Player::getDashCooldown()
{
    return dashCooldown;
}

ScreenTransition &Player::getScreenTransition()
{
    return *transition;
}

bool Player::isCDMenuOpened()
{
    return this->CDMenu.getIsOpened();
}

int Player::getHP()
{
    return this->HP_;
}

int Player::getMaxHP()
{
    return this->maxHP;
}

int Player::getEnergy()
{
    return this->energy;
}

int Player::getMaxEnergy()
{
    return this->maxEnergy;
}

int Player::getGold() const
{
    return gold_;
}

int Player::getDamageValue() const
{
    return DMG_;
}

int Player::getShootCostValue() const
{
    return shootCost;
}

int Player::getShootCooldownValue() const
{
    return ButtonRepeat_shootCooldown;
}

int Player::getEnergyGainValue() const
{
    return energyGain;
}

float Player::getBulletSpeedValue() const
{
    return bulletSpeed;
}

float Player::getBulletRangeValue() const
{
    return bulletMaxDistance_;
}

float Player::getAccelerationValue() const
{
    return speed;
}

float Player::getMaxWalkSpeedValue() const
{
    return maxWalkSpeed;
}

float Player::getDashForceValue() const
{
    return dashForce;
}

int Player::getDashCooldownValue() const
{
    return dashCooldown;
}

float Player::getJumpPowerValue() const
{
    return jumpImpulse_;
}

int Player::getExtraJumpCountValue() const
{
    return maxAirJumps_;
}

int Player::getSlowFallPercentValue() const
{
    if (baseGravity_ <= 0.0001f)
    {
        return 0;
    }

    const float normalizedGravity = std::clamp(gravity_ / baseGravity_, 0.f, 1.f);
    return static_cast<int>(std::round((1.f - normalizedGravity) * 100.f));
}

bool Player::canAfford(int amount) const
{
    return amount <= gold_;
}

const std::vector<Player::OwnedItem> &Player::getInventory() const
{
    return inventory_;
}

bool Player::ownsItem(const std::string &iconName) const
{
    return std::any_of(
        inventory_.begin(),
        inventory_.end(),
        [&](const OwnedItem& ownedItem) {
            return ownedItem.iconName == iconName;
        }
    );
}

sf::Vector2f Player::getFeetPosition() const
{
    const sf::FloatRect bounds = playerRectangle_->getGlobalBounds();
    return {
        bounds.position.x + bounds.size.x / 2.f,
        bounds.position.y + bounds.size.y
    };
}

float Player::getFacingDirection() const
{
    return playerSprite->getScale().x >= 0.f ? 1.f : -1.f;
}

void Player::triggerCameraImpact(const sf::Vector2f& direction, float impulseStrength, float trauma, float zoomPunch)
{
    if (camera)
    {
        camera->addImpact(direction, impulseStrength, trauma, zoomPunch);
    }
}

void Player::pushRing(
    const sf::Vector2f& position,
    const sf::Color& color,
    float radius,
    float maxRadius,
    float growth,
    float thickness,
    float alpha
)
{
    effectRings_.push_back(VisualRing{
        .position = position,
        .color = color,
        .radius = radius,
        .maxRadius = maxRadius,
        .growth = growth,
        .thickness = thickness,
        .alpha = alpha
    });
}

void Player::spawnParticleBurst(
    const sf::Vector2f& origin,
    const sf::Color& color,
    int count,
    float minSpeed,
    float maxSpeed,
    float radius,
    float gravity,
    float lifetime
)
{
    for (int index = 0; index < count; ++index)
    {
        const float angle = random(0.f, 360.f) * kPi / 180.f;
        const float speedValue = random(minSpeed, maxSpeed);
        const sf::Vector2f velocity = {
            std::cos(angle) * speedValue,
            std::sin(angle) * speedValue
        };

        particles.emplace_back(
            origin,
            velocity,
            sf::Vector2f{random(-55.f, 55.f), random(-45.f, 45.f)},
            color,
            radius,
            gravity,
            0.88f,
            lifetime
        );
    }
}

void Player::spawnRunEffect()
{
    const sf::Vector2f feet = getFeetPosition();
    const sf::Color ashColor = std::abs(initialWalkSpeed) > maxWalkSpeed * 0.8f
        ? sf::Color(168, 110, 102, 95)
        : sf::Color(110, 122, 134, 78);

    spawnParticleBurst(
        {feet.x + random(-10.f, 10.f), feet.y - 4.f},
        ashColor,
        2,
        12.f,
        44.f,
        1.9f,
        -12.f,
        0.55f
    );
}

void Player::spawnJumpEffect()
{
    const sf::Vector2f feet = getFeetPosition();
    pushRing(feet, sf::Color(156, 88, 76, 148), 8.f, 52.f, 3.5f, 2.4f, 148.f);
    spawnParticleBurst(
        {feet.x, feet.y - 6.f},
        sf::Color(204, 114, 92, 190),
        10,
        50.f,
        150.f,
        2.3f,
        18.f,
        0.58f
    );
}

void Player::spawnLandingEffect(float impactStrength)
{
    if (landingEffectClock_.getElapsedTime().asMilliseconds() < LANDING_EFFECT_COOLDOWN_MS)
    {
        return;
    }

    landingEffectClock_.restart();

    const sf::Vector2f feet = getFeetPosition();
    const float clampedImpact = std::clamp(impactStrength, 1.5f, 8.f);
    pushRing(
        feet,
        sf::Color(188, 94, 82, 156),
        12.f,
        62.f + clampedImpact * 4.f,
        4.1f,
        2.5f,
        156.f
    );
    spawnParticleBurst(
        {feet.x, feet.y - 4.f},
        sf::Color(112, 92, 86, 170),
        static_cast<int>(6 + clampedImpact),
        28.f,
        120.f + clampedImpact * 16.f,
        2.4f,
        54.f,
        0.8f
    );
    triggerCameraImpact({0.f, -0.45f}, 26.f + clampedImpact * 3.f, 0.12f + clampedImpact * 0.015f, 0.018f);
}

void Player::spawnShootEffect(bool direction)
{
    const float facingDirection = direction ? 1.f : -1.f;
    const sf::Vector2f center = getCenterPosition();
    const sf::Vector2f muzzleOrigin = {
        center.x + facingDirection * 24.f,
        center.y - 4.f
    };
    const Item::WeaponKind weaponKind = getCurrentWeapon().weaponStats.kind;
    sf::Color primaryColor = sf::Color(255, 210, 148, 220);
    sf::Color ringColor = sf::Color(242, 184, 112, 178);

    switch (weaponKind)
    {
        case Item::WeaponKind::Gravepiercer:
            primaryColor = sf::Color(214, 228, 242, 220);
            ringColor = sf::Color(176, 204, 232, 178);
            break;
        case Item::WeaponKind::PyreOrb:
            primaryColor = sf::Color(255, 136, 92, 225);
            ringColor = sf::Color(255, 178, 122, 185);
            break;
        case Item::WeaponKind::StormNeedler:
            primaryColor = sf::Color(120, 246, 255, 225);
            ringColor = sf::Color(92, 198, 245, 185);
            break;
        case Item::WeaponKind::DreadPrism:
            primaryColor = sf::Color(220, 132, 255, 228);
            ringColor = sf::Color(170, 98, 240, 185);
            break;
        case Item::WeaponKind::NightfallBeam:
            primaryColor = sf::Color(246, 96, 170, 235);
            ringColor = sf::Color(196, 84, 210, 196);
            break;
        case Item::WeaponKind::AshenBolt:
            break;
    }

    pushRing(
        muzzleOrigin,
        ringColor,
        7.f,
        weaponKind == Item::WeaponKind::NightfallBeam ? 48.f : 36.f,
        weaponKind == Item::WeaponKind::NightfallBeam ? 4.4f : 3.6f,
        2.f,
        178.f
    );
    spawnParticleBurst(
        muzzleOrigin,
        primaryColor,
        weaponKind == Item::WeaponKind::NightfallBeam ? 11 : 7,
        70.f,
        weaponKind == Item::WeaponKind::NightfallBeam ? 240.f : 188.f,
        2.1f,
        14.f,
        0.42f
    );
    triggerCameraImpact(
        {direction ? -1.f : 1.f, 0.f},
        weaponKind == Item::WeaponKind::NightfallBeam ? 42.f : 18.f,
        weaponKind == Item::WeaponKind::NightfallBeam ? 0.16f : 0.08f,
        weaponKind == Item::WeaponKind::NightfallBeam ? 0.026f : 0.014f
    );
}

void Player::spawnDashBurst()
{
    const float direction = getFacingDirection();
    const sf::Vector2f center = getCenterPosition();
    const sf::Vector2f origin = {center.x - direction * 18.f, center.y + 10.f};

    pushRing(origin, sf::Color(180, 66, 72, 170), 10.f, 68.f, 4.3f, 3.f, 170.f);
    spawnParticleBurst(
        origin,
        sf::Color(210, 82, 88, 205),
        14,
        80.f,
        220.f,
        2.7f,
        12.f,
        0.62f
    );
    triggerCameraImpact({direction, -0.08f}, 32.f, 0.16f, 0.025f);
}

void Player::spawnTeleportEffect(bool enteringPortal)
{
    const sf::Vector2f center = getCenterPosition();
    const sf::Color portalColor = enteringPortal
        ? sf::Color(112, 210, 240, 215)
        : sf::Color(92, 170, 214, 170);

    pushRing(center, portalColor, enteringPortal ? 10.f : 6.f, enteringPortal ? 64.f : 42.f, 3.8f, 2.4f, enteringPortal ? 185.f : 140.f);
    spawnParticleBurst(
        {center.x + random(-12.f, 12.f), center.y + random(-18.f, 18.f)},
        portalColor,
        enteringPortal ? 10 : 4,
        34.f,
        enteringPortal ? 148.f : 92.f,
        2.2f,
        -36.f,
        enteringPortal ? 0.72f : 0.52f
    );
}

void Player::spawnDeathEffect()
{
    const sf::Vector2f center = getCenterPosition();
    pushRing(center, sf::Color(224, 84, 76, 215), 12.f, 86.f, 5.f, 3.4f, 215.f);
    pushRing(center, sf::Color(255, 226, 180, 130), 8.f, 44.f, 3.f, 2.f, 130.f);
    spawnParticleBurst(center, sf::Color(226, 70, 76, 220), 20, 90.f, 240.f, 3.3f, 48.f, 0.95f);
    spawnParticleBurst(center, sf::Color(32, 10, 14, 220), 12, 34.f, 112.f, 4.4f, -16.f, 1.15f);
}

void Player::spawnCriticalHealthEffect()
{
    const sf::Vector2f center = getCenterPosition();
    const sf::Vector2f emberOrigin = {
        center.x + random(-10.f, 10.f),
        center.y + random(-22.f, 10.f)
    };

    spawnParticleBurst(
        emberOrigin,
        sf::Color(144, 30, 42, 150),
        3,
        12.f,
        54.f,
        1.8f,
        -10.f,
        0.7f
    );
    pushRing(center, sf::Color(86, 22, 28, 75), 10.f, 32.f, 1.9f, 1.4f, 75.f);
}

void Player::spawnWeaponSwitchEffect()
{
    const sf::Vector2f center = getCenterPosition();
    const sf::Color weaponColor = getCurrentWeaponQuality() == Item::LEGENDARY
        ? sf::Color(255, 214, 112, 195)
        : getCurrentWeaponQuality() == Item::MYTH
            ? sf::Color(255, 126, 102, 190)
            : getCurrentWeaponQuality() == Item::RARE
                ? sf::Color(118, 188, 255, 180)
                : sf::Color(180, 188, 210, 165);

    pushRing(center, weaponColor, 10.f, 54.f, 3.4f, 2.2f, 160.f);
    spawnParticleBurst(
        {center.x, center.y - 10.f},
        weaponColor,
        8,
        40.f,
        120.f,
        2.2f,
        -14.f,
        0.52f
    );
}

void Player::updateRingEffects()
{
    for (auto& ring : effectRings_)
    {
        ring.radius += ring.growth;
        ring.alpha = std::max(0.f, ring.alpha - ring.growth * 3.4f);
    }

    effectRings_.erase(
        std::remove_if(effectRings_.begin(), effectRings_.end(), [](const VisualRing& ring) {
            return ring.radius >= ring.maxRadius || ring.alpha <= 2.f;
        }),
        effectRings_.end()
    );
}

int Player::takeAllGold()
{
    const int lostGold = gold_;
    gold_ = 0;
    return lostGold;
}

void Player::initializeDefaultWeapon()
{
    arsenal_.clear();
    arsenal_.push_back({
        "Item_02.png",
        "Ashen Bolt",
        Item::COMMON,
        Item::Category::Weapon,
        0,
        {},
        {
            Item::WeaponKind::AshenBolt,
            0,
            baseShootCooldown_,
            baseShootCost_,
            0,
            0,
            1,
            0,
            0,
            0
        },
        "The default rite: a single ember bolt, steady and dependable."
    });
    currentWeaponIndex_ = 0;
}

const Player::OwnedItem& Player::getCurrentWeapon() const
{
    if (arsenal_.empty())
    {
        static const OwnedItem fallbackWeapon{
            "Item_02.png",
            "Ashen Bolt",
            Item::COMMON,
            Item::Category::Weapon,
            0,
            {},
            {},
            "Fallback rite"
        };
        return fallbackWeapon;
    }

    return arsenal_[std::min(currentWeaponIndex_, arsenal_.size() - 1)];
}

std::string Player::getCurrentWeaponName() const
{
    return getCurrentWeapon().displayName;
}

std::string Player::getCurrentWeaponIconName() const
{
    return getCurrentWeapon().iconName;
}

Item::Quality Player::getCurrentWeaponQuality() const
{
    return getCurrentWeapon().quality;
}

const std::vector<Player::OwnedItem>& Player::getWeapons() const
{
    return arsenal_;
}

CampaignObjectiveSnapshot Player::getCampaignSnapshot() const
{
    return campaignProgress_.buildSnapshot();
}

const CampaignProgress& Player::getCampaignProgress() const
{
    return campaignProgress_;
}

std::string Player::getCampaignBoonTitle() const
{
    return campaignProgress_.getActiveBoonState().title;
}

bool Player::isLevelUnlocked(const std::string& levelName) const
{
    return campaignProgress_.isLevelUnlocked(levelName);
}

std::vector<std::string> Player::getUnlockedLevelNames(const std::vector<std::string>& levelNames) const
{
    return campaignProgress_.filterUnlockedLevels(levelNames);
}

std::string Player::getLevelUnlockHint(const std::string& levelName) const
{
    return campaignProgress_.getLevelUnlockHint(levelName);
}

void Player::switchWeapon(int direction)
{
    if (arsenal_.size() <= 1)
    {
        return;
    }

    if (direction > 0)
    {
        currentWeaponIndex_ = (currentWeaponIndex_ + 1) % arsenal_.size();
    }
    else
    {
        currentWeaponIndex_ = currentWeaponIndex_ == 0 ? arsenal_.size() - 1 : currentWeaponIndex_ - 1;
    }

    applyCurrentWeaponStats();
    spawnWeaponSwitchEffect();
}

void Player::applyCurrentWeaponStats()
{
    const OwnedItem& weapon = getCurrentWeapon();
    const CampaignBoonState campaignBoons = campaignProgress_.getActiveBoonState();

    shootCost = std::max(5, weapon.weaponStats.energyCost);
    ButtonRepeat_shootCooldown = std::max(
        20,
        weapon.weaponStats.cooldownMs - inventoryStatsBonus_.shootSpeedCooldownReduction - campaignBoons.shootCooldownReduction
    );
    DMG_ = baseDMG_ + inventoryStatsBonus_.damage + campaignBoons.damageBonus + weapon.weaponStats.damageBonus;
    bulletSpeed = baseBulletSpeed_ + static_cast<float>(inventoryStatsBonus_.bulletSpeed + weapon.weaponStats.projectileSpeed);
    bulletMaxDistance_ = baseBulletMaxDistance_ + static_cast<float>(
        inventoryStatsBonus_.bulletDistance + campaignBoons.bulletRangeBonus + weapon.weaponStats.projectileRange
    );
}

bool Player::canPerformJump() const
{
    return !isFalling || airJumpsRemaining_ > 0;
}

void Player::restoreAirJumps()
{
    airJumpsRemaining_ = maxAirJumps_;
}

std::shared_ptr<Bullet> Player::createProjectile(const Bullet::Config& config, const sf::Vector2f& startPosition, const sf::Vector2f& speedValue)
{
    auto projectile = std::make_shared<Bullet>(startPosition, bulletMaxDistance_, *gameTextures, config);
    projectile->setSpeed(speedValue);
    projectile->setPosition(startPosition);
    return projectile;
}

sf::Clock &Player::getPortalClock()
{
    return portalCooldownClock;
}

int &Player::getPortalCooldown()
{
    return portalCallCooldown;
}

void Player::attachGameLevelManager(GameLevelManager& m)
{
    this->levelManager = &m;
}

void Player::setPosition(sf::Vector2f pos)
{
    this->playerRectangle_->setPosition(pos);
    this->playerSprite->setPosition(playerRectangle_->getGlobalBounds().getCenter());
}

void Player::addGold(int amount)
{
    if(amount <= 0)
    {
        return;
    }

    const std::size_t previousStageIndex = campaignProgress_.getCurrentStageIndex();
    gold_ += amount;
    campaignProgress_.onGoldCollected(amount);
    if (campaignProgress_.getCurrentStageIndex() != previousStageIndex)
    {
        recalculateStatsFromInventory();
    }
}

bool Player::spendGold(int amount)
{
    if(amount < 0 || gold_ < amount)
    {
        return false;
    }

    gold_ -= amount;
    campaignProgress_.onGoldSpent(amount);
    return true;
}

void Player::restoreVitalResources()
{
    const bool wasInjured = HP_ < maxHP || energy < maxEnergy;

    HP_ = maxHP;
    energy = maxEnergy;
    restoreAirJumps();

    if (!wasInjured)
    {
        return;
    }

    const sf::Vector2f center = getCenterPosition();
    pushRing(center, sf::Color(124, 224, 202, 180), 10.f, 72.f, 4.4f, 2.8f, 180.f);
    pushRing(center, sf::Color(255, 214, 144, 132), 7.f, 46.f, 3.2f, 2.1f, 132.f);
    spawnParticleBurst(
        center,
        sf::Color(138, 232, 214, 215),
        12,
        82.f,
        176.f,
        2.4f,
        -54.f,
        0.62f
    );
    triggerCameraImpact({0.f, -0.18f}, 18.f, 0.08f, 0.01f);
}

void Player::teleportToSupportPoint(const sf::Vector2f& supportPoint)
{
    initialWalkSpeed = 0.f;
    fallingSpeed = 0.f;
    isFalling = false;
    isFliesUp = false;
    isJumped = false;

    restoreAirJumps();

    setPosition({
        supportPoint.x - playerRectangle_->getSize().x / 2.f,
        supportPoint.y - playerRectangle_->getSize().y
    });

    const sf::Vector2f center = getCenterPosition();
    pushRing(center, sf::Color(104, 212, 182, 166), 12.f, 74.f, 4.6f, 2.4f, 166.f);
    pushRing(center, sf::Color(226, 196, 118, 124), 8.f, 44.f, 3.4f, 2.0f, 124.f);
    spawnParticleBurst(
        center,
        sf::Color(168, 236, 214, 208),
        10,
        72.f,
        142.f,
        2.0f,
        -36.f,
        0.54f
    );

    if (camera)
    {
        camera->setCenterPosition(center);
        camera->addImpact({0.f, -0.16f}, 10.f, 0.08f, 0.012f);
    }
}

void Player::respawnAt(sf::Vector2f pos)
{
    HP_ = maxHP;
    energy = maxEnergy;
    fallingSpeed = 0.f;
    initialWalkSpeed = 0.f;

    isAlive = true;
    isIdle = true;
    isFalling = true;
    isFliesUp = false;
    isJumped = false;
    isControlsBlocked = false;
    isPlayingDieAnimation = false;
    isPlayingHurtAnimation = false;
    isPlayingDashAnimation = false;
    isDashOnCooldown = false;
    takeDMG_isOnCooldown = false;
    isPortalOnCooldown = false;
    canShoot = true;
    canJump = true;
    canDash = true;
    canSwitchWeapon = true;

    bullets.clear();
    particles.clear();
    effectRings_.clear();

    shootTimer.reset();
    shootTimer.stop();
    jumpTimer.reset();
    jumpTimer.stop();
    dashTimer.reset();
    dashTimer.stop();
    dash_Clock.reset();
    dash_Clock.stop();
    takeDMG_timer.reset();
    takeDMG_timer.stop();
    portalCooldownClock.reset();
    portalCooldownClock.stop();
    portalCallOpenCooldownClock.reset();
    portalCallCloseCooldownClock.reset();
    weaponSwitchTimer.reset();
    weaponSwitchTimer.stop();
    runEffectClock_.restart();
    teleportEffectClock_.restart();
    criticalEffectClock_.restart();
    landingEffectClock_.restart();
    deathEffectPlayed_ = false;
    wasInTeleportArea_ = false;
    restoreAirJumps();
    applyCurrentWeaponStats();

    if (portal)
    {
        portal->resetState();
    }

    if (camera)
    {
        camera->clearEffects();
    }

    CDMenu.close();
    setPosition(pos);
    playerSprite->setTexture(idleTextures->at(0), true);
    playerSprite->setScale({std::abs(playerSprite->getScale().x), playerSprite->getScale().y});
    trail->clearTrailArray();
}

bool Player::tryPurchaseItem(const Item &item)
{
    if(item.isPurchased() || ownsItem(item.iconName) || !spendGold(item.price))
    {
        return false;
    }

    inventory_.push_back({
        item.iconName,
        item.displayName,
        item.quality,
        item.category,
        item.price,
        item.stats,
        item.weaponStats,
        item.description
    });

    if (item.category == Item::Category::Weapon)
    {
        arsenal_.push_back(inventory_.back());
        currentWeaponIndex_ = arsenal_.size() - 1;
    }

    campaignProgress_.onItemPurchased(item.category);
    recalculateStatsFromInventory();
    if (item.category == Item::Category::Weapon)
    {
        spawnWeaponSwitchEffect();
    }
    return true;
}

void Player::notifyLevelEntered(const std::string& levelName)
{
    const std::size_t previousStageIndex = campaignProgress_.getCurrentStageIndex();
    campaignProgress_.onLevelEntered(levelName);
    if (campaignProgress_.getCurrentStageIndex() != previousStageIndex)
    {
        recalculateStatsFromInventory();
    }
}

void Player::updateTextures()
{
    portal->update();   //NOTE Should be updating no matter what
    if(portal->getIsInAreaOfTeleportation()) fallingSpeed = 0.f;
    updateTransition();

    if(!isAlive)
    {
        sf::Texture &lastDieTexture = satiro_dieTextures->at(satiro_dieTextures->size()-1);
        this->playerSprite->setTexture(lastDieTexture);
        return; //Nothing happens if player not alive
    }

    if(isPlayingDieAnimation)
    {
        if (!deathEffectPlayed_)
        {
            spawnDeathEffect();
            deathEffectPlayed_ = true;
        }

        pulseSprite(*playerSprite, sf::Color(230, 78, 82, 255), 2.2f, sf::seconds(0.35f));
        if(!switchToNextSprite(this->playerSprite,*this->satiro_dieTextures,satiro_die_helper,switchSprite_SwitchOption::Single))
        {
            isAlive = false;
            isPlayingDieAnimation = false;
        }
        return;
    }

    if(isPlayingHurtAnimation)
    {
        pulseSprite(*playerSprite, sf::Color(255, 96, 92, 255), 1.8f, sf::seconds(0.24f));
        if(!switchToNextSprite(this->playerSprite,*this->satiro_hurtTextures,satiro_hurt_helper,switchSprite_SwitchOption::Single))
        {
            isPlayingHurtAnimation = false;
        }
        return;
    }

    if(isFliesUp)
    {
        switchToNextSprite(this->playerSprite,*this->satiro_jumpTextures,satiro_jump_helper,switchSprite_SwitchOption::Loop);
    } else {
        if(isJumped && !isFalling)
        {
            if(!switchToNextSprite(this->playerSprite,*this->satiro_landingTextures,satiro_landing_helper,switchSprite_SwitchOption::Single))
            {
                isJumped = false;
            }
        }
    }

    if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
    {
        if(!isFalling && !isPlayingDashAnimation && !isJumped && !isControlsBlocked)
        {
            switchToNextRunningSprite();
        }
        if(playerSprite->getScale().x>0 && !isControlsBlocked)
        {
            playerSprite->setScale({-(playerSprite->getScale().x),1.f});
        }
    }
    else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
    {
        if(!isFalling && !isPlayingDashAnimation && !isJumped && !isControlsBlocked)
        {
            switchToNextRunningSprite();
        }
        if(playerSprite->getScale().x<0 && !isControlsBlocked)
        {
            playerSprite->setScale({-(playerSprite->getScale().x),1.f});
        }
    }

    if(isPlayingDashAnimation)
    {
        pulseSprite(*playerSprite, sf::Color(198, 90, 100, 255), 2.4f, sf::seconds(0.2f));
        if(!switchToNextSprite(this->playerSprite,*this->satiro_dashTextures,satiro_dash_helper,switchSprite_SwitchOption::Single))
        {
            isPlayingDashAnimation= false;
        }
        dashParticles();
        return;
    }

    if(isFalling && !isFliesUp)
    {  
        switchToNextFallingSprite();
    }
    if(isIdle && !isFalling && !isJumped && !isFliesUp)
    {
        switchToNextIdleSprite();
    }
    for (auto &&i : bullets)
    {
        i->updateTextures();
    }
    
}

void Player::drawBullets(sf::RenderWindow& window)
{
    for (auto &&i : bullets)
    {
        i->draw(window);
    }
    
}

Player::~Player()
{
    saveData();
}

void Player::portalUpdate()
{
    //Call cooldown
    if(isPortalOnCooldown && portal->getIsClosed() && !portal->getIsCalledForClose() && !portal->getIsCalledForOpen() && !portalCallOpenCooldownClock.isRunning())
    {
        portalCallOpenCooldownClock.restart();
        portalCooldownClock.restart();
    }
    if(isPortalOnCooldown && checkInterval(portalCallOpenCooldownClock,portalCallCooldown))
    {
        isPortalOnCooldown = false;
        portalCallOpenCooldownClock.reset();
        portalCooldownClock.stop();
    }

    //Closing portal
    if(portal->getIsOpened() && !portal->getIsCalledForClose() && !portalCallCloseCooldownClock.isRunning())
    {
        portalCallCloseCooldownClock.restart();
    }
    if(checkInterval(portalCallCloseCooldownClock,portalExistTime))
    {
        portal->closePortal();
        if(!transition->isFadeInComplete())
        {
            transition->fadeIn();
        }
        portalCallCloseCooldownClock.reset();
    }

    //Is player in area of teleportation
    if(portal->getIsOpened())
    {
        portal->checkIsTargetInAreaOfTeleportation();
    }
}

void Player::tryOpenPortal()
{
    //Open portal only when it closed and is not called for open
    if( portal->getIsClosed() && !portal->getIsCalledForOpen() && !isPortalOnCooldown)
    {
        if(sf::Keyboard::isKeyPressed(portalCallKey))
        {
            portalCooldownClock.reset();
            //Setting portal position based on what side player watches now
                //Right
            if(playerSprite->getScale().x > 0.f)
            {
                portal->setSide(PortalCalledSide::RIGHT);
                portal->setPosition({playerSprite->getGlobalBounds().getCenter().x+BASE_OFFSET_TO_CREATE_PORTAL,playerSprite->getGlobalBounds().getCenter().y});
            }
                //Left
            else if(playerSprite->getScale().x < 0.f)
            {
                portal->setSide(PortalCalledSide::LEFT);
                portal->setPosition({playerSprite->getGlobalBounds().getCenter().x-BASE_OFFSET_TO_CREATE_PORTAL,playerSprite->getGlobalBounds().getCenter().y});
            }

            portal->openPortal();
            isPortalOnCooldown = true;
        }
    }
}

void Player::setPortalDestination(std::optional<std::string> levelName)
{
    portal->setPortalDestination(levelName);
}

void Player::chooseDestinationMenuDraw(sf::RenderWindow &w)
{
    CDMenu.draw(w);
}

void Player::chooseDestinationMenuUpdate()
{
    CDMenu.update();
}

void Player::chooseDestinationMenuHandleEvents(const sf::Event &ev)
{
    if(const auto* keyPressed = ev.getIf<sf::Event::KeyPressed>())
    {
        if(keyPressed->scancode == BASE_CHOOSEDESTINATIONMENU_OPEN_CLOSE_KEY)
        {
            if(!CDMenu.getIsOpened())
            {
                CDMenu.open();
            }
            else if(CDMenu.getIsOpened())
            {
                CDMenu.close();
            }

            return;
        }
    }

    CDMenu.handleEvents(ev);
}

void Player::drawTransition()
{
    this->transition->draw();
}

void Player::playFadeInAnimation()
{
    transition->fadeIn();
}

void Player::updateTransition()
{
    transition->update();
}

void Player::applyFriction(float &walkSpeed, float friction)
{
    if (walkSpeed > 0.f)
    {
        walkSpeed = std::max(0.f, walkSpeed - friction);
    }
    else if (walkSpeed < 0.f)
    {
        walkSpeed = std::min(0.f, walkSpeed + friction);
    }
    // Если walkSpeed очень мал, можно сразу установить в 0
    if (std::abs(walkSpeed) < friction * 0.5f)
    {
        walkSpeed = 0.f;
    }
}

void Player::saveData()
{
    try
    {
        nlohmann::json inventoryData = nlohmann::json::array();
        for (const auto& item : inventory_)
        {
            inventoryData.push_back(ownedItemToJson(item));
        }

        const CampaignSaveState campaignSave = campaignProgress_.buildSaveState();
        nlohmann::json saveData = {
            {"version", 1},
            {"player", {
                {"gold", gold_},
                {"currentWeaponIconName", getCurrentWeaponIconName()},
                {"inventory", inventoryData}
            }},
            {"campaign", {
                {"totalGoldCollected", campaignSave.totalGoldCollected},
                {"totalGoldSpent", campaignSave.totalGoldSpent},
                {"relicsPurchased", campaignSave.relicsPurchased},
                {"weaponsPurchased", campaignSave.weaponsPurchased},
                {"visitedLevels", campaignSave.visitedLevels}
            }}
        };

        std::ofstream output(kPlayerProgressPath);
        output << saveData.dump(4);
    }
    catch (const std::exception&)
    {
    }
}

void Player::loadData()
{
    std::fstream f("data/PlayerConfig.json");
    nlohmann::json data = nlohmann::json::parse(f);
    //Player
    baseHP_ = data["Player"]["HP"];
    this->HP_ = baseHP_;
    maxHP = baseHP_;
    this->takeDMG_cooldown = data["Player"]["takeDMG_cooldown"];
    gold_ = data["Player"].value("Gold", 0);
    this->energy = data["Player"]["Energy"];
    maxEnergy = energy;
    baseMaxEnergy_ = maxEnergy;
    this->energyGain = data["Player"]["EnergyGain"];
    baseEnergyGain_ = energyGain;
    this->shootCost = data["Player"]["ShootCost"];
    baseShootCost_ = shootCost;

    //Jump
    this->ButtonRepeat_jumpCooldown = data["Jump"]["repeatCooldown"];

    //Bullet
    this->DMG_ = data["Bullet"]["DMG"];
    baseDMG_ = DMG_;
    this->bulletMaxDistance_ = data["Bullet"]["bulletMaxDistance"];
    baseBulletMaxDistance_ = bulletMaxDistance_;
    this->bulletSpeed = data["Bullet"]["bulletSpeed"];
    baseBulletSpeed_ = bulletSpeed;
    this->bulletSpeedReduction = data["Bullet"]["bulletSpeedReduction"];
    this->ButtonRepeat_shootCooldown = data["Bullet"]["repeatCooldown"];
    baseShootCooldown_ = ButtonRepeat_shootCooldown;
    

    //Dash
    this->dashForce = data["Dash"]["force"];
    this->dashCooldown = data["Dash"]["Cooldown"];
    this->ButtonRepeat_dashCooldown = data["Dash"]["repeatCooldown"];
    baseDashForce_ = dashForce;
    baseDashCooldown_ = dashCooldown;

    baseAcceleration_ = speed;
    baseMaxWalkSpeed_ = maxWalkSpeed;
    inventory_.clear();
    arsenal_.clear();
    inventoryStatsBonus_ = {};
}

void Player::loadProgressData()
{
    std::ifstream progressFile(kPlayerProgressPath);
    if (!progressFile.is_open())
    {
        return;
    }

    try
    {
        const nlohmann::json progressData = nlohmann::json::parse(progressFile);
        const nlohmann::json playerData = progressData.value("player", nlohmann::json::object());
        const nlohmann::json inventoryData = playerData.value("inventory", nlohmann::json::array());

        gold_ = std::max(0, playerData.value("gold", gold_));
        inventory_.clear();
        arsenal_.clear();
        initializeDefaultWeapon();

        for (const auto& itemData : inventoryData)
        {
            restoreOwnedItem(ownedItemFromJson(itemData));
        }

        const std::string currentWeaponIcon = playerData.value("currentWeaponIconName", getCurrentWeaponIconName());
        const auto equippedWeaponIt = std::find_if(
            arsenal_.begin(),
            arsenal_.end(),
            [&](const OwnedItem& weapon) {
                return weapon.iconName == currentWeaponIcon;
            }
        );
        currentWeaponIndex_ = equippedWeaponIt != arsenal_.end()
            ? static_cast<std::size_t>(std::distance(arsenal_.begin(), equippedWeaponIt))
            : 0u;

        const nlohmann::json campaignData = progressData.value("campaign", nlohmann::json::object());
        CampaignSaveState campaignSave;
        campaignSave.totalGoldCollected = campaignData.value("totalGoldCollected", 0);
        campaignSave.totalGoldSpent = campaignData.value("totalGoldSpent", 0);
        campaignSave.relicsPurchased = campaignData.value("relicsPurchased", 0);
        campaignSave.weaponsPurchased = campaignData.value("weaponsPurchased", 0);
        campaignSave.visitedLevels = campaignData.value("visitedLevels", std::vector<std::string>{});
        campaignProgress_.loadSaveState(campaignSave);
    }
    catch (const std::exception&)
    {
        inventory_.clear();
        arsenal_.clear();
        initializeDefaultWeapon();
    }
}

void Player::restoreOwnedItem(const OwnedItem& itemData)
{
    if (itemData.iconName.empty() || ownsItem(itemData.iconName))
    {
        return;
    }

    inventory_.push_back(itemData);
    if (itemData.category == Item::Category::Weapon)
    {
        arsenal_.push_back(itemData);
    }
}

void Player::recalculateStatsFromInventory()
{
    const CampaignBoonState campaignBoons = campaignProgress_.getActiveBoonState();
    inventoryStatsBonus_ = {};
    for (const auto& item : inventory_)
    {
        inventoryStatsBonus_.bulletSpeed += item.stats.bulletSpeed;
        inventoryStatsBonus_.bulletDistance += item.stats.bulletDistance;
        inventoryStatsBonus_.shootSpeedCooldownReduction += item.stats.shootSpeedCooldownReduction;
        inventoryStatsBonus_.initialSpeed += item.stats.initialSpeed;
        inventoryStatsBonus_.maxSpeed += item.stats.maxSpeed;
        inventoryStatsBonus_.health += item.stats.health;
        inventoryStatsBonus_.damage += item.stats.damage;
        inventoryStatsBonus_.dashForce += item.stats.dashForce;
        inventoryStatsBonus_.dashCooldownReduction += item.stats.dashCooldownReduction;
        inventoryStatsBonus_.extraJumpCount += item.stats.extraJumpCount;
        inventoryStatsBonus_.jumpPower += item.stats.jumpPower;
        inventoryStatsBonus_.slowFallPercent += item.stats.slowFallPercent;
    }

    const int previousMaxHP = maxHP;

    maxHP = baseHP_ + inventoryStatsBonus_.health + campaignBoons.healthBonus;
    HP_ = std::min(HP_ + std::max(0, maxHP - previousMaxHP), maxHP);
    maxEnergy = baseMaxEnergy_ + campaignBoons.maxEnergyBonus;
    energyGain = baseEnergyGain_ + campaignBoons.energyGainBonus;
    shootCost = baseShootCost_;

    speed = baseAcceleration_ + static_cast<float>(inventoryStatsBonus_.initialSpeed) / 100.f;
    maxWalkSpeed = baseMaxWalkSpeed_ + static_cast<float>(inventoryStatsBonus_.maxSpeed) / 10.f;
    dashForce = baseDashForce_ + static_cast<float>(inventoryStatsBonus_.dashForce) / 10.f;
    dashCooldown = std::max(140, baseDashCooldown_ - inventoryStatsBonus_.dashCooldownReduction);
    jumpImpulse_ = baseJumpImpulse_ + static_cast<float>(inventoryStatsBonus_.jumpPower) / 10.f;
    gravity_ = baseGravity_ * std::clamp(1.f - static_cast<float>(inventoryStatsBonus_.slowFallPercent) / 100.f, 0.35f, 1.0f);
    maxAirJumps_ = std::max(0, inventoryStatsBonus_.extraJumpCount);
    restoreAirJumps();
    applyCurrentWeaponStats();
}

void Player::checkPlatformRectCollision(std::vector<std::shared_ptr<sf::RectangleShape>>& rects)
{
    for (auto& rectPtr : rects)  // меняем тип итератора
    {
        // Получаем глобальные границы (для удобства)
        sf::FloatRect playerBounds = playerRectangle_->getGlobalBounds();
        sf::FloatRect platformBounds = rectPtr->getGlobalBounds();  // используем -> для shared_ptr

        // Проверяем пересечение
        if (playerBounds.findIntersection(platformBounds)) 
        {
            // Определяем направление коллизии
            float overlapLeft   = playerBounds.position.x + playerBounds.size.x - platformBounds.position.x;
            float overlapRight  = platformBounds.position.x + platformBounds.size.x - playerBounds.position.x;
            float overlapTop    = playerBounds.position.y + playerBounds.size.y - platformBounds.position.y;
            float overlapBottom = platformBounds.position.y + platformBounds.size.y - playerBounds.position.y;

            // Ищем минимальное перекрытие
            bool fromLeft   = (overlapLeft < overlapRight);
            bool fromTop    = (overlapTop < overlapBottom);
            float minXOverlap = fromLeft ? overlapLeft : overlapRight;
            float minYOverlap = fromTop ? overlapTop : overlapBottom;

            // Коллизия с БОКОВЫМИ сторонами
            if (minXOverlap < minYOverlap) 
            {
                if (fromLeft) {
                    // Слева
                    playerRectangle_->setPosition({platformBounds.position.x - playerBounds.size.x, playerBounds.position.y});
                } else {
                    // Справа
                    playerRectangle_->setPosition({platformBounds.position.x + platformBounds.size.x, playerBounds.position.y});
                }
            } 
            // Коллизия с ВЕРХНЕЙ/НИЖНЕЙ сторонами
            else 
            {
                if (fromTop) {
                    // Сверху
                    const float landingSpeed = fallingSpeed;
                    const bool hardLanding = isFalling && landingSpeed > 1.6f;
                    isFalling = false;
                    if(fallingSpeed>0.f)
                    {
                        fallingSpeed = 0.f;
                    }
                    if(fallingSpeed >= -0.1f && fallingSpeed<=0.1f)
                    {
                        if(playerBounds.position.y+playerBounds.size.y >= platformBounds.position.y+3.f)
                        {
                            playerRectangle_->setPosition({playerBounds.position.x,playerBounds.position.y-2.f});
                        }
                    }

                    if (hardLanding)
                    {
                        spawnLandingEffect(landingSpeed);
                    }

                    restoreAirJumps();
                } else {
                    // Снизу
                    playerRectangle_->setPosition({playerBounds.position.x, platformBounds.position.y + platformBounds.size.y});
                    fallingSpeed = -(fallingSpeed);
                }
            }
        }
    }
}

void Player::checkGroundCollision(sf::RectangleShape& groundRect)
{
    float playerX = playerRectangle_->getPosition().x;
    float playerY = playerRectangle_->getPosition().y+playerRectangle_->getSize().y;
    float groundY = groundRect.getPosition().y;

    if(playerY>=groundY)
    {
        const float landingSpeed = fallingSpeed;
        const bool hardLanding = isFalling && landingSpeed > 1.6f;
        isFalling = false;
        fallingSpeed = 0.f;
        playerRectangle_->setPosition({playerX,groundY-playerRectangle_->getSize().y});

        if (hardLanding)
        {
            spawnLandingEffect(landingSpeed);
        }

        restoreAirJumps();
    }
}

void Player::moveBullets()
{
    // Удаляем пули которые можно удалить
    bullets.remove_if([](std::shared_ptr<Bullet>& bullet) {
        return bullet->canBeDeleted;
    });
    
    for (auto &&i : bullets)
    {
        i->update();
    }
}

void Player::updateParticles()
{
    for (auto& particle : particles) { 
        particle.update();
    }
    particles.erase(
        std::remove_if(particles.begin(), particles.end(),
            [](const Particle& p) { return !p.getIsAlive(); }),
        particles.end()
    );

    updateRingEffects();
}

void Player::updateEnergy()
{
    // Energy gain
    if(energy>=maxEnergy) return;
    energy+=energyGain;
    if(energy>maxEnergy) energy=maxEnergy;
}

void Player::walkLeft()
{
    if(!this->isAlive || isPlayingDieAnimation || isPlayingDashAnimation) return;       //Locking movement on Die, Dash

    if(initialWalkSpeed<=(-maxWalkSpeed))
    {
        return; 
    }
    initialWalkSpeed-=speed;
}

void Player::walkRight()
{
    if(!this->isAlive || isPlayingDieAnimation || isPlayingDashAnimation) return;       //Locking movement on Die, Dash

    if(initialWalkSpeed>=maxWalkSpeed)
    {
        return;
    }
    initialWalkSpeed+=speed;
}

void Player::jump()
{
    if(!this->isAlive || isPlayingDieAnimation || isPlayingDashAnimation) return;       //Locking movement on Die, Dash
    if (isFalling && airJumpsRemaining_ <= 0)
    {
        return;
    }

    if (isFalling)
    {
        airJumpsRemaining_--;
    }

    playerRectangle_->setPosition({playerRectangle_->getPosition().x,playerRectangle_->getPosition().y-1.f});
    fallingSpeed = -jumpImpulse_;
    spawnJumpEffect();
    triggerCameraImpact({0.f, -1.f}, isFalling ? 24.f : 16.f, isFalling ? 0.12f : 0.08f, 0.015f);
}

void Player::fallDown()
{
}

void Player::dash()
{
    if(!this->isAlive || isPlayingDieAnimation) return;

        if(isDashOnCooldown)
    {
        if(dash_Clock.getElapsedTime().asMilliseconds() >= dashCooldown)
        {
            isDashOnCooldown = false;
            dash_Clock.stop();
        }
    }
    if(!isDashOnCooldown)
    {
        //Cooldown
        isPlayingDashAnimation = true;
        isDashOnCooldown = true;
        dash_Clock.restart();
        if(playerSprite->getScale().x>0){
            initialWalkSpeed = dashForce;
        } else{
            initialWalkSpeed = -dashForce;
        }

        spawnDashBurst();
    }
}

bool Player::shoot(bool direction)
{
    if(!this->isAlive || isPlayingDieAnimation) return false;
    if(energy<shootCost) return false;

    const OwnedItem& weapon = getCurrentWeapon();
    const float directionSign = direction ? 1.f : -1.f;
    const sf::Vector2f playerCenter = playerRectangle_->getGlobalBounds().getCenter();
    const sf::Vector2f baseSpawnPosition = {
        playerCenter.x + directionSign * 16.f,
        playerCenter.y - 4.f
    };
    const float inheritedSpeed = std::max(0.f, initialWalkSpeed * directionSign * 1.35f);
    const float projectileSpeed = std::max(2.f, bulletSpeed + inheritedSpeed);

    auto makeHorizontalVelocity = [&](float speedMultiplier = 1.f, float verticalSpeed = 0.f) {
        return sf::Vector2f(directionSign * projectileSpeed * speedMultiplier, verticalSpeed);
    };

    auto pushProjectile = [&](const Bullet::Config& config, const sf::Vector2f& spawnPosition, const sf::Vector2f& speedValue, bool faceLeft = false) {
        std::shared_ptr<Bullet> projectile = createProjectile(config, spawnPosition, speedValue);
        if (faceLeft)
        {
            projectile->setSpriteScale({-config.spriteScale.x, config.spriteScale.y});
        }
        bullets.push_back(std::move(projectile));
    };

    switch (weapon.weaponStats.kind)
    {
        case Item::WeaponKind::AshenBolt:
        {
            Bullet::Config config;
            config.type = Bullet::Type::Ember;
            config.tint = sf::Color(180, 244, 255, 255);
            config.trailColor = sf::Color(124, 235, 255, 215);
            config.impactColor = sf::Color(186, 248, 255, 235);
            config.damage = DMG_;
            config.maxHits = 1;
            config.spriteScale = {direction ? 1.f : -1.f, 1.f};
            pushProjectile(config, baseSpawnPosition, makeHorizontalVelocity());
            break;
        }
        case Item::WeaponKind::Gravepiercer:
        {
            Bullet::Config config;
            config.type = Bullet::Type::Piercing;
            config.hitboxSize = {40.f, 18.f};
            config.spriteScale = {1.45f, 0.75f};
            config.tint = sf::Color(222, 230, 245, 255);
            config.trailColor = sf::Color(198, 214, 235, 215);
            config.impactColor = sf::Color(236, 242, 255, 235);
            config.damage = DMG_;
            config.maxHits = std::max(2, weapon.weaponStats.pierceCount + 1);
            config.keepAliveOnHit = true;
            config.trailParticleCount = 4;
            pushProjectile(config, baseSpawnPosition, makeHorizontalVelocity(1.12f));
            break;
        }
        case Item::WeaponKind::PyreOrb:
        {
            Bullet::Config config;
            config.type = Bullet::Type::Splash;
            config.hitboxSize = {34.f, 34.f};
            config.spriteScale = {1.25f, 1.25f};
            config.tint = sf::Color(255, 162, 116, 255);
            config.trailColor = sf::Color(255, 126, 88, 215);
            config.impactColor = sf::Color(255, 196, 144, 235);
            config.damage = DMG_;
            config.splashRadius = static_cast<float>(weapon.weaponStats.splashRadius);
            config.deathParticleCount = 56;
            config.trailParticleCount = 6;
            pushProjectile(config, {baseSpawnPosition.x, baseSpawnPosition.y - 4.f}, makeHorizontalVelocity(0.82f, -0.18f));
            break;
        }
        case Item::WeaponKind::StormNeedler:
        {
            Bullet::Config config;
            config.type = Bullet::Type::Storm;
            config.hitboxSize = {24.f, 12.f};
            config.spriteScale = {1.1f, 0.6f};
            config.tint = sf::Color(122, 244, 255, 255);
            config.trailColor = sf::Color(82, 196, 255, 225);
            config.impactColor = sf::Color(206, 248, 255, 240);
            config.damage = DMG_;
            config.maxHits = 1;
            config.trailParticleCount = 3;

            const std::vector<float> offsets = {-8.f, 0.f, 8.f};
            for (const float offsetY : offsets)
            {
                pushProjectile(
                    config,
                    {baseSpawnPosition.x, baseSpawnPosition.y + offsetY},
                    makeHorizontalVelocity(1.28f, offsetY * 0.035f)
                );
            }
            break;
        }
        case Item::WeaponKind::DreadPrism:
        {
            Bullet::Config config;
            config.type = Bullet::Type::Prism;
            config.hitboxSize = {28.f, 16.f};
            config.spriteScale = {1.15f, 0.78f};
            config.tint = sf::Color(220, 136, 255, 255);
            config.trailColor = sf::Color(170, 92, 240, 225);
            config.impactColor = sf::Color(238, 188, 255, 240);
            config.damage = DMG_;
            config.maxHits = 2;
            config.keepAliveOnHit = true;
            config.trailParticleCount = 4;

            const float spread = std::max(1.f, static_cast<float>(weapon.weaponStats.spread) * 0.18f);
            pushProjectile(config, baseSpawnPosition, makeHorizontalVelocity(1.02f, -spread));
            pushProjectile(config, baseSpawnPosition, makeHorizontalVelocity(1.08f, 0.f));
            pushProjectile(config, baseSpawnPosition, makeHorizontalVelocity(1.02f, spread));
            break;
        }
        case Item::WeaponKind::NightfallBeam:
        {
            const float beamLength = std::max(220.f, bulletMaxDistance_ * 0.55f);
            const float beamHeight = 22.f;

            Bullet::Config config;
            config.type = Bullet::Type::Beam;
            config.hitboxSize = {beamLength, beamHeight};
            config.spriteScale = {beamLength / 30.f, 0.8f};
            config.tint = sf::Color(255, 128, 196, 230);
            config.trailColor = sf::Color(212, 102, 255, 205);
            config.impactColor = sf::Color(255, 154, 214, 235);
            config.damage = DMG_;
            config.maxHits = 8;
            config.keepAliveOnHit = true;
            config.beamLike = true;
            config.beamLifetime = 0.14f;
            config.particleCooldownMs = 18.0;
            config.trailParticleCount = 9;
            config.deathParticleCount = 62;

            const sf::Vector2f beamTopLeft = direction
                ? sf::Vector2f{playerCenter.x + 26.f, playerCenter.y - beamHeight / 2.f}
                : sf::Vector2f{playerCenter.x - beamLength - 26.f, playerCenter.y - beamHeight / 2.f};
            pushProjectile(config, beamTopLeft, {0.f, 0.f}, !direction);
            break;
        }
    }

    spawnShootEffect(direction);

    energy-=shootCost;
    if(energy<0) energy = 0;

    return true;
}
void Player::dashParticles()
{
    const float direction = getFacingDirection();
    const sf::Vector2f center = getCenterPosition();
    const sf::Vector2f origin = {
        center.x - direction * random(4.f, 18.f),
        playerRectangle_->getPosition().y + playerRectangle_->getSize().y - random(2.f, 10.f)
    };

    spawnParticleBurst(
        origin,
        sf::Color(126, 76, 86, 135),
        6,
        30.f,
        120.f,
        1.8f,
        -12.f,
        0.32f
    );

    if (random(0.f, 1.f) > 0.68f)
    {
        pushRing(origin, sf::Color(154, 78, 88, 78), 4.f, 18.f, 1.8f, 1.4f, 78.f);
    }
}
/*
    Returns true on successful hit \ Returns false on unsucessful hit

    Has cooldown that declared in Player.h
*/
bool Player::takeDMG(int count, sf::Vector2f knockback, bool side) 
{
    if(!isAlive || isPlayingDieAnimation)
    {
        return false;
    }

    if(takeDMG_isOnCooldown)
    {
        if(takeDMG_timer.getElapsedTime().asMilliseconds() >= takeDMG_cooldown)
        {
            takeDMG_isOnCooldown = false;
            takeDMG_timer.stop();
        }
        return false;
    }
    if(!takeDMG_isOnCooldown)
    {
        //Knockback
        this->fallingSpeed-=knockback.y;
        side ? this->initialWalkSpeed-=knockback.x : this->initialWalkSpeed+=knockback.x;

        //HP reduction
        this->HP_ = std::max(0, this->HP_ - count);

        //Hurt animation enabling
        isPlayingHurtAnimation = true;

        //Blood
        bloodExplode();
        pushRing(getCenterPosition(), sf::Color(210, 64, 70, 165), 10.f, 48.f, 4.f, 2.2f, 165.f);
        spawnParticleBurst(
            getCenterPosition(),
            sf::Color(212, 72, 78, 215),
            8,
            70.f,
            180.f,
            2.4f,
            28.f,
            0.48f
        );
        triggerCameraImpact({side ? -1.f : 1.f, -0.16f}, 72.f, 0.5f, 0.065f);

        //Cooldown
        takeDMG_isOnCooldown = true;
        takeDMG_timer.restart();

        if (HP_ <= 0)
        {
            CDMenu.close();
            if (!deathEffectPlayed_)
            {
                spawnDeathEffect();
                deathEffectPlayed_ = true;
            }
            triggerCameraImpact({0.f, -0.65f}, 110.f, 0.75f, 0.11f);
        }
        return true;
    }
    return false;
}

void Player::bloodExplode()
{
    sf::Vector2f playerPos = this->playerRectangle_->getGlobalBounds().getCenter();

    for (int i = 0; i < 40; i++) {
        particles.emplace_back(
            sf::Vector2f(playerPos.x,playerPos.y),
            sf::Vector2f(random(-150,150), random(-450,-150)*1.5),
            sf::Vector2f(random(-60,60), random(-60,60)),
            sf::Color(138, 3, 3),
            2.0f,
            150.0f*3,
            0.8f,
            3.0f
        );
    }
}

void Player::updateControls()
{
    if(!this->isAlive || isPlayingDieAnimation || isControlsBlocked) return;
    
    // Обновляем готовность действий по таймерам
    if (!canShoot && shootTimer.getElapsedTime().asMilliseconds() >= ButtonRepeat_shootCooldown) {
        canShoot = true;
        shootTimer.stop();
    }
    
    if (!canJump && jumpTimer.getElapsedTime().asMilliseconds() >= ButtonRepeat_jumpCooldown) {
        canJump = true;
        jumpTimer.stop();
    }
    
    if (!canDash && dashTimer.getElapsedTime().asMilliseconds() >= ButtonRepeat_dashCooldown) {
        canDash = true;
        dashTimer.stop();
    }

    if (!canSwitchWeapon && weaponSwitchTimer.getElapsedTime().asMilliseconds() >= ButtonRepeat_weaponSwitchCooldown) {
        canSwitchWeapon = true;
        weaponSwitchTimer.stop();
    }
    
    // Shooting (X key)
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::X) && canShoot)
    {
        if(shoot(getSpriteScale().x > 0))
        {
            canShoot = false;
            shootTimer.restart();
        } 
    }
    
    // Jumping (Z key)
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Z) && canJump)
    {
        if(canPerformJump())
        {
            jump();
            canJump = false;
            jumpTimer.restart();
        }
    }
    
    // Dash (C key)
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::C) && canDash)
    {
        dash();
        canDash = false;
        dashTimer.restart();
    }

    if (canSwitchWeapon)
    {
        if (sf::Keyboard::isKeyPressed(BASE_WEAPON_SWITCH_PREVIOUS_KEY))
        {
            switchWeapon(-1);
            canSwitchWeapon = false;
            weaponSwitchTimer.restart();
        }
        else if (sf::Keyboard::isKeyPressed(BASE_WEAPON_SWITCH_NEXT_KEY))
        {
            switchWeapon(1);
            canSwitchWeapon = false;
            weaponSwitchTimer.restart();
        }
    }
    
    // Moving right-left
    isIdle = true;
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
    {
        isIdle = false;
        walkLeft();
    }
    else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
    {
        isIdle = false;
        walkRight();
    }

    //Open portal  
    tryOpenPortal();
}

void Player::updatePhysics()
{
    portalUpdate();

    updateParticles();

    if(this->HP_<=0)
    {
        isPlayingDieAnimation = true;
        if (!deathEffectPlayed_)
        {
            spawnDeathEffect();
            deathEffectPlayed_ = true;
            triggerCameraImpact({0.f, -0.65f}, 110.f, 0.75f, 0.11f);
        }
    }

    if (portal->getIsInAreaOfTeleportation())
    {
        if (!wasInTeleportArea_)
        {
            wasInTeleportArea_ = true;
            teleportEffectClock_.restart();
            spawnTeleportEffect(true);
            triggerCameraImpact({0.f, -0.2f}, 18.f, 0.1f, 0.02f);
        }
        else if (teleportEffectClock_.getElapsedTime().asMilliseconds() >= TELEPORT_EFFECT_INTERVAL_MS)
        {
            teleportEffectClock_.restart();
            spawnTeleportEffect(false);
        }
    }
    else
    {
        wasInTeleportArea_ = false;
    }

    if(isPlayingDieAnimation || !isAlive)
    {
        initialWalkSpeed = 0.f;
        fallingSpeed = 0.f;
        return;
    }

    applyFriction(initialWalkSpeed,this->frictionForce);

    updateEnergy();

    if (HP_ > 0 &&
        HP_ <= static_cast<int>(std::ceil(maxHP * CRITICAL_HP_RATIO)) &&
        criticalEffectClock_.getElapsedTime().asMilliseconds() >= CRITICAL_EFFECT_INTERVAL_MS)
    {
        criticalEffectClock_.restart();
        spawnCriticalHealthEffect();
    }

    if (!isFalling &&
        !isPlayingDashAnimation &&
        std::abs(initialWalkSpeed) > maxWalkSpeed * 0.45f &&
        runEffectClock_.getElapsedTime().asMilliseconds() >= RUN_EFFECT_INTERVAL_MS)
    {
        runEffectClock_.restart();
        spawnRunEffect();
    }

    if(fallingSpeed<0)
    {
            isFliesUp = true;
            isJumped  = true;
    } else  isFliesUp = false;

    if(!isPlayingDashAnimation) playerRectangle_->move({0.f,this->fallingSpeed});
    
    playerRectangle_->move({initialWalkSpeed,0.f});
    
    if(playerRectangle_->getPosition().y+playerRectangle_->getSize().y>=WINDOW_HEIGHT)
    {
        isFalling = false;
        playerRectangle_->setPosition({playerRectangle_->getPosition().x, WINDOW_HEIGHT-playerRectangle_->getSize().y});
        restoreAirJumps();
    }
    else
    {
        isFalling = true;
    }
    if(isFalling)
    {
        if(isPlayingDashAnimation)
        {
            fallingSpeed= 0.f;
        } else{
            fallingSpeed += gravity_;
        }

    }
    else
    {
        fallingSpeed = 0.f;
    }
    //std::cout << playerRectangle->getPosition().x << std::endl;

    //Level border collision (left and right)
    float levelWidth = static_cast<float>(levelManager->getCurrentLevelSize().x);
    if(playerRectangle_->getPosition().x+playerRectangle_->getSize().x>=levelWidth)
    {
        playerRectangle_->setPosition({levelWidth-playerRectangle_->getSize().x,playerRectangle_->getPosition().y});
    }
    else if(playerRectangle_->getPosition().x<=0)
    {
        playerRectangle_->setPosition({0.f,playerRectangle_->getPosition().y});
    }
}

void Player::switchToNextIdleSprite()
{
    static int fps{1};
    if(fps!=WINDOW_FPS/12)
    {   
        fps++;
        return;
    }
    static size_t i = 0;
    static bool goForward = true;
    playerSprite->setTexture(idleTextures->at(i),true);
    if(i == idleTextures->size()-1 && goForward == true)
    {
        goForward = false;
    }
    else if(i == 0 && goForward == false)
    {
        goForward = true;
    }
    if(goForward)
    {
        i++;
    }
    else
    {
        i--;
    }
    fps=1;
}

void Player::switchToNextRunningSprite()
{
    static int fps = 1;
    if(fps!=WINDOW_FPS/12)
    {   
        fps++;
        return;
    }
    static size_t i = 0;
    playerSprite->setTexture(runningTextures->at(i),true);
    i++;
    if(i == runningTextures->size()-1)
    {
        i=0;
    }
    
    fps=1;
}



void Player::draw(sf::RenderWindow& window)
{
    portal->draw(window);

    drawPlayerTrail(window);

    drawParticles(window);
    playerSprite->setPosition({(playerRectangle_->getPosition().x+playerRectangle_->getSize().x/2),(playerRectangle_->getPosition().y+playerRectangle_->getSize().y/2)-6.f});
    // window.draw(*playerRectangle_);
    window.draw(*playerSprite);
}

void Player::drawParticles(sf::RenderWindow &window)
{
    for (const auto& ring : effectRings_)
    {
        sf::CircleShape circle(ring.radius);
        circle.setOrigin({circle.getRadius(), circle.getRadius()});
        circle.setPosition(ring.position);
        circle.setFillColor(sf::Color::Transparent);
        circle.setOutlineThickness(ring.thickness);
        circle.setOutlineColor(sf::Color(ring.color.r, ring.color.g, ring.color.b, static_cast<std::uint8_t>(ring.alpha)));
        window.draw(circle);
    }

    for (auto& particle : particles) {
        particle.draw(window);
    }
}

void Player::drawPlayerTrail(sf::RenderWindow& window)
{
    trail->speedOfTrailDisappearing = 14;
    trail->trailColor = sf::Color(14, 12, 16, 82);

    if (portal->getIsInAreaOfTeleportation())
    {
        trail->speedOfTrailDisappearing = 10;
        trail->trailColor = sf::Color(54, 132, 166, 96);
        trail->generateTrail(window);
    }
    else if (isPlayingDashAnimation)
    {
        trail->speedOfTrailDisappearing = 9;
        trail->trailColor = sf::Color(110, 24, 30, 155);
        trail->generateTrail(window);
    }
    else if (!isFalling && std::abs(initialWalkSpeed) > maxWalkSpeed * 0.6f)
    {
        trail->trailColor = sf::Color(22, 18, 22, 92);
        trail->generateTrail(window);
    }

    trail->makeTrailDisappear();
    trail->drawTrail(window);
}

void Player::blockControls()
{
    this->isControlsBlocked = true;
}

void Player::unblockControls()
{
    this->isControlsBlocked = false;
}
