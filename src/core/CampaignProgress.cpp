#include <CampaignProgress.h>

#include <algorithm>
#include <array>
#include <sstream>

namespace
{
constexpr std::size_t kStoryStageCount = 7;

const CampaignLevelInfo kFallbackLevelInfo{
    "unknown",
    "Unwritten Ground",
    "A route not yet named by the covenant.",
    "No chronicler has written what waits there.",
    "Best for: unknown returns.",
    "Its seal is not yet understood."
};

const std::array<CampaignLevelInfo, 6> kCampaignLevels{{
    {
        "level1.json",
        "Ashwake Causeway",
        "The broken outer terraces where the farm first starts paying for itself.",
        "Ruined ledges, simple prey and enough grave-gold to wake the Heart Lantern's first ember.",
        "Best for: early relic money and safe farming laps.",
        "Open from the first step."
    },
    {
        "level2.json",
        "Obsidian Rookery",
        "A cold vertical roost full of hungry wings and sharper rewards.",
        "Basalt scaffolds and bat-haunted spans where the Bone Collector tests whether your first profits became real tools.",
        "Best for: faster farming and proving your first relic build.",
        "Harvest 60 grave-gold and buy your first relic."
    },
    {
        "level3.json",
        "Crimson Nave",
        "A ceremonial hall where richer prey starts to fight back in earnest.",
        "The red nave still burns with old ritual heat, and its guardians carry enough coin to fund the next descent.",
        "Best for: mid-run gold bursts and denser enemy lines.",
        "Reach Obsidian Rookery and harvest 180 grave-gold total."
    },
    {
        "level4.json",
        "Bone Reliquary",
        "A pale vault of relic shelves built for serious upgrading.",
        "This crypt once stored covenant tools. Now it yields the parts and coin needed to turn a scavenger into a proper hunter.",
        "Best for: relic-focused farming and heavier investment runs.",
        "Own 3 relics from the trader."
    },
    {
        "level5.json",
        "Trial of Embers",
        "A compact proving ground where weak builds go to die quickly.",
        "The Heart Lantern only opens this route once you arrive armed for a harder harvest. Survive it, and the richest road returns.",
        "Best for: weapon checks and high-pressure gold bursts.",
        "Buy your first crafted weapon."
    },
    {
        "level6.json",
        "The Returning Veil",
        "The deepest scar in the farm, where the covenant can finally be restored.",
        "Beyond the last gate the ruined estate folds back on itself. Feed it enough power and the whole hunting loop comes alive again.",
        "Best for: endgame farming and sealing the story.",
        "Reach the Trial of Embers and harvest 420 grave-gold total."
    }
}};

float normalizedProgress(int current, int target)
{
    if (target <= 0)
    {
        return 1.f;
    }

    return std::clamp(static_cast<float>(current) / static_cast<float>(target), 0.f, 1.f);
}

std::string boolProgressLabel(bool value, const std::string& positiveLabel, const std::string& negativeLabel)
{
    return value ? positiveLabel : negativeLabel;
}
}

void CampaignProgress::onGoldCollected(int amount)
{
    if (amount <= 0)
    {
        return;
    }

    totalGoldCollected_ += amount;
    refreshProgression();
}

void CampaignProgress::onGoldSpent(int amount)
{
    if (amount <= 0)
    {
        return;
    }

    totalGoldSpent_ += amount;
    refreshProgression();
}

void CampaignProgress::onItemPurchased(Item::Category category)
{
    if (category == Item::Category::Weapon)
    {
        ++weaponsPurchased_;
    }
    else
    {
        ++relicsPurchased_;
    }

    refreshProgression();
}

void CampaignProgress::onLevelEntered(const std::string& levelName)
{
    if (levelName.empty())
    {
        return;
    }

    visitedLevels_.insert(levelName);
    refreshProgression();
}

bool CampaignProgress::isLevelUnlocked(const std::string& levelName) const
{
    if (levelName == "level1.json")
    {
        return true;
    }
    if (levelName == "level2.json")
    {
        return currentStageIndex_ >= 2;
    }
    if (levelName == "level3.json")
    {
        return currentStageIndex_ >= 3;
    }
    if (levelName == "level4.json")
    {
        return currentStageIndex_ >= 4;
    }
    if (levelName == "level5.json")
    {
        return currentStageIndex_ >= 5;
    }
    if (levelName == "level6.json")
    {
        return currentStageIndex_ >= 6;
    }

    return true;
}

std::vector<std::string> CampaignProgress::filterUnlockedLevels(const std::vector<std::string>& levels) const
{
    std::vector<std::string> unlockedLevels;
    unlockedLevels.reserve(levels.size());

    for (const auto& levelName : levels)
    {
        if (isLevelUnlocked(levelName))
        {
            unlockedLevels.push_back(levelName);
        }
    }

    return unlockedLevels;
}

CampaignObjectiveSnapshot CampaignProgress::buildSnapshot() const
{
    CampaignObjectiveSnapshot snapshot;
    snapshot.campaignTitle = "Restore the Heart Lantern";
    snapshot.storyComplete = currentStageIndex_ >= kStoryStageCount;

    if (snapshot.storyComplete)
    {
        snapshot.chapterTitle = "VIII. The Farm Breathes Again";
        snapshot.narrative = "The covenant lives. Every open route now exists to enrich your build and deepen the hunt.";
        snapshot.objective = "Keep farming, forging and pushing the restored grounds.";
        snapshot.progressText = std::to_string(weaponsPurchased_) + " weapons forged  |  " +
            std::to_string(totalGoldCollected_) + " grave-gold harvested";
        snapshot.rewardText = "Reward: free hunt across every attuned route.";
        snapshot.progressRatio = 1.f;
        return snapshot;
    }

    snapshot.chapterTitle = getStageChapterTitle(currentStageIndex_);
    snapshot.narrative = getStageNarrative(currentStageIndex_);
    snapshot.objective = getStageObjective(currentStageIndex_);
    snapshot.progressText = getStageProgressText(currentStageIndex_);
    snapshot.rewardText = getStageReward(currentStageIndex_);
    snapshot.progressRatio = getStageProgressRatio(currentStageIndex_);
    return snapshot;
}

std::string CampaignProgress::getLevelUnlockHint(const std::string& levelName) const
{
    std::ostringstream stream;

    if (levelName == "level2.json")
    {
        stream << "Seal: harvest 60 grave-gold and buy 1 relic.\n"
               << "Progress: " << std::min(totalGoldCollected_, 60) << "/60 gold, "
               << std::min(relicsPurchased_, 1) << "/1 relic.";
        return stream.str();
    }
    if (levelName == "level3.json")
    {
        stream << "Seal: enter Obsidian Rookery and reach 180 total grave-gold.\n"
               << "Progress: "
               << boolProgressLabel(hasVisitedLevel("level2.json"), "Rookery entered", "Rookery not entered")
               << ", " << std::min(totalGoldCollected_, 180) << "/180 gold.";
        return stream.str();
    }
    if (levelName == "level4.json")
    {
        stream << "Seal: own 3 relics from the trader.\n"
               << "Progress: " << std::min(relicsPurchased_, 3) << "/3 relics.";
        return stream.str();
    }
    if (levelName == "level5.json")
    {
        stream << "Seal: buy your first crafted weapon.\n"
               << "Progress: " << std::min(weaponsPurchased_, 1) << "/1 weapon.";
        return stream.str();
    }
    if (levelName == "level6.json")
    {
        stream << "Seal: enter the Trial of Embers and reach 420 total grave-gold.\n"
               << "Progress: "
               << boolProgressLabel(hasVisitedLevel("level5.json"), "Trial entered", "Trial not entered")
               << ", " << std::min(totalGoldCollected_, 420) << "/420 gold.";
        return stream.str();
    }

    return "No seal binds this route.";
}

std::string CampaignProgress::getMerchantGreeting() const
{
    if (currentStageIndex_ >= kStoryStageCount)
    {
        return "The Bone Collector grins through cracked teeth.";
    }

    switch (currentStageIndex_)
    {
    case 0:
        return "The Bone Collector taps ash from an old ledger.";
    case 1:
        return "The trader counts your first honest haul with approval.";
    case 2:
        return "The merchant traces a cold rune toward the next gate.";
    case 3:
        return "He opens another drawer of relics and nods toward the vault roads.";
    case 4:
        return "The old scavenger stops smiling. The next road wants a weapon.";
    case 5:
        return "He listens to the embers like they are answering him back.";
    case 6:
        return "The merchant says the last gate only opens for a hunter, not a survivor.";
    default:
        return "The trader waits for your next decision.";
    }
}

std::string CampaignProgress::getMerchantAdvice() const
{
    switch (currentStageIndex_)
    {
    case 0:
        return "Farm Ashwake until you drag home 60 grave-gold. The lantern wakes on profit, not courage.";
    case 1:
        return "Buy one relic before you chase depth. A farmer without investment is just bait with boots.";
    case 2:
        return "Obsidian Rookery is open now. Run it until your lifetime haul reaches 180 gold.";
    case 3:
        return "Stock 3 relics and the Bone Reliquary will answer. This is where the build starts compounding.";
    case 4:
        return "Stop polishing trinkets and forge a real weapon. Trial of Embers will break starter gear.";
    case 5:
        return "Take your new weapon into the Trial and push the total haul to 420 gold.";
    case 6:
        return "Enter the Returning Veil and bring 2 crafted weapons into the covenant. Finish the restoration.";
    default:
        return "Every route is open. Buy for greed, comfort or sheer spectacle now.";
    }
}

const CampaignLevelInfo& CampaignProgress::getRecommendedLevelInfo() const
{
    if (currentStageIndex_ <= 1)
    {
        return getLevelInfo("level1.json");
    }
    if (currentStageIndex_ == 2)
    {
        return getLevelInfo("level2.json");
    }
    if (currentStageIndex_ == 3)
    {
        return getLevelInfo("level3.json");
    }
    if (currentStageIndex_ == 4)
    {
        return getLevelInfo("level4.json");
    }
    if (currentStageIndex_ == 5)
    {
        return getLevelInfo("level5.json");
    }

    return getLevelInfo("level6.json");
}

CampaignBoonState CampaignProgress::getActiveBoonState() const
{
    CampaignBoonState boon;

    if (currentStageIndex_ >= 1)
    {
        boon.title = "Lantern Spark";
        boon.description = "The rekindled farm toughens your body.";
        boon.healthBonus += 60;
    }
    if (currentStageIndex_ >= 2)
    {
        boon.title = "Broker's Habit";
        boon.description = "Trade and harvest begin feeding your reserves.";
        boon.energyGainBonus += 1;
    }
    if (currentStageIndex_ >= 3)
    {
        boon.title = "Rookery Tithe";
        boon.description = "Cold hunts teach your bolts to hit harder.";
        boon.damageBonus += 8;
    }
    if (currentStageIndex_ >= 4)
    {
        boon.title = "Reliquary Reserve";
        boon.description = "The restored vault deepens your arcane reserve.";
        boon.maxEnergyBonus += 140;
    }
    if (currentStageIndex_ >= 5)
    {
        boon.title = "Ember Temper";
        boon.description = "Trial-forged rites carry farther through the ruins.";
        boon.bulletRangeBonus += 80;
    }
    if (currentStageIndex_ >= 6)
    {
        boon.title = "Veil Reflex";
        boon.description = "The returning paths teach swifter release.";
        boon.shootCooldownReduction += 12;
    }
    if (currentStageIndex_ >= kStoryStageCount)
    {
        boon.title = "Heart Rekindled";
        boon.description = "The covenant fully wakes and strengthens every hunt.";
        boon.healthBonus += 90;
        boon.damageBonus += 10;
    }

    return boon;
}

CampaignSaveState CampaignProgress::buildSaveState() const
{
    CampaignSaveState state;
    state.totalGoldCollected = totalGoldCollected_;
    state.totalGoldSpent = totalGoldSpent_;
    state.relicsPurchased = relicsPurchased_;
    state.weaponsPurchased = weaponsPurchased_;
    state.visitedLevels.assign(visitedLevels_.begin(), visitedLevels_.end());
    return state;
}

void CampaignProgress::loadSaveState(const CampaignSaveState& state)
{
    totalGoldCollected_ = std::max(0, state.totalGoldCollected);
    totalGoldSpent_ = std::max(0, state.totalGoldSpent);
    relicsPurchased_ = std::max(0, state.relicsPurchased);
    weaponsPurchased_ = std::max(0, state.weaponsPurchased);
    visitedLevels_.clear();
    visitedLevels_.insert(state.visitedLevels.begin(), state.visitedLevels.end());
    currentStageIndex_ = 0;
    refreshProgression();
}

int CampaignProgress::getTotalGoldCollected() const
{
    return totalGoldCollected_;
}

int CampaignProgress::getTotalGoldSpent() const
{
    return totalGoldSpent_;
}

int CampaignProgress::getRelicsPurchased() const
{
    return relicsPurchased_;
}

int CampaignProgress::getWeaponsPurchased() const
{
    return weaponsPurchased_;
}

bool CampaignProgress::hasVisitedLevel(const std::string& levelName) const
{
    return visitedLevels_.find(levelName) != visitedLevels_.end();
}

std::size_t CampaignProgress::getCurrentStageIndex() const
{
    return currentStageIndex_;
}

const CampaignLevelInfo& CampaignProgress::getLevelInfo(const std::string& levelName)
{
    const auto levelIt = std::find_if(
        kCampaignLevels.begin(),
        kCampaignLevels.end(),
        [&](const CampaignLevelInfo& levelInfo) {
            return levelInfo.levelName == levelName;
        }
    );

    return levelIt != kCampaignLevels.end() ? *levelIt : kFallbackLevelInfo;
}

void CampaignProgress::refreshProgression()
{
    while (currentStageIndex_ < kStoryStageCount && isStageComplete(currentStageIndex_))
    {
        ++currentStageIndex_;
    }
}

bool CampaignProgress::isStageComplete(std::size_t stageIndex) const
{
    switch (stageIndex)
    {
    case 0:
        return totalGoldCollected_ >= 60;
    case 1:
        return relicsPurchased_ >= 1;
    case 2:
        return hasVisitedLevel("level2.json") && totalGoldCollected_ >= 180;
    case 3:
        return relicsPurchased_ >= 3;
    case 4:
        return weaponsPurchased_ >= 1;
    case 5:
        return hasVisitedLevel("level5.json") && totalGoldCollected_ >= 420;
    case 6:
        return hasVisitedLevel("level6.json") && weaponsPurchased_ >= 2;
    default:
        return true;
    }
}

float CampaignProgress::getStageProgressRatio(std::size_t stageIndex) const
{
    switch (stageIndex)
    {
    case 0:
        return normalizedProgress(totalGoldCollected_, 60);
    case 1:
        return normalizedProgress(relicsPurchased_, 1);
    case 2:
        return (normalizedProgress(totalGoldCollected_, 180) + (hasVisitedLevel("level2.json") ? 1.f : 0.f)) * 0.5f;
    case 3:
        return normalizedProgress(relicsPurchased_, 3);
    case 4:
        return normalizedProgress(weaponsPurchased_, 1);
    case 5:
        return (normalizedProgress(totalGoldCollected_, 420) + (hasVisitedLevel("level5.json") ? 1.f : 0.f)) * 0.5f;
    case 6:
        return (normalizedProgress(weaponsPurchased_, 2) + (hasVisitedLevel("level6.json") ? 1.f : 0.f)) * 0.5f;
    default:
        return 1.f;
    }
}

std::string CampaignProgress::getStageChapterTitle(std::size_t stageIndex) const
{
    switch (stageIndex)
    {
    case 0:
        return "I. Wake the Lantern";
    case 1:
        return "II. First Bargain";
    case 2:
        return "III. Obsidian Harvest";
    case 3:
        return "IV. Stock the Farm";
    case 4:
        return "V. Forge a True Weapon";
    case 5:
        return "VI. Trial by Embers";
    case 6:
        return "VII. Return Through the Veil";
    default:
        return "VIII. Endless Harvest";
    }
}

std::string CampaignProgress::getStageNarrative(std::size_t stageIndex) const
{
    switch (stageIndex)
    {
    case 0:
        return "The Bone Collector says the ruined farm will breathe again once you feed its Heart Lantern.";
    case 1:
        return "Your first haul is enough to stop surviving and start investing in a proper farming loop.";
    case 2:
        return "The next gate answers only hunters who can bring wealth back from harsher ground.";
    case 3:
        return "Relics turn raw income into momentum. Stock the covenant before the deeper vaults open.";
    case 4:
        return "The Trial will not respect a peasant's tool. You need a crafted weapon now.";
    case 5:
        return "Trial of Embers is the last price before the richest route returns to the estate.";
    case 6:
        return "One more descent and one more forged weapon should be enough to complete the old covenant.";
    default:
        return "The harvest goes on.";
    }
}

std::string CampaignProgress::getStageObjective(std::size_t stageIndex) const
{
    switch (stageIndex)
    {
    case 0:
        return "Harvest 60 grave-gold from the outer ruins.";
    case 1:
        return "Buy your first relic from the trader.";
    case 2:
        return "Enter Obsidian Rookery and reach 180 total grave-gold.";
    case 3:
        return "Own 3 relics to stock the covenant.";
    case 4:
        return "Buy your first crafted weapon.";
    case 5:
        return "Enter the Trial of Embers and reach 420 total grave-gold.";
    case 6:
        return "Enter The Returning Veil and own 2 crafted weapons.";
    default:
        return "Keep the restored farm rich.";
    }
}

std::string CampaignProgress::getStageProgressText(std::size_t stageIndex) const
{
    switch (stageIndex)
    {
    case 0:
        return std::to_string(std::min(totalGoldCollected_, 60)) + " / 60 grave-gold";
    case 1:
        return std::to_string(std::min(relicsPurchased_, 1)) + " / 1 relic purchased";
    case 2:
        return boolProgressLabel(hasVisitedLevel("level2.json"), "Rookery entered", "Rookery pending") +
            std::string("  |  ") + std::to_string(std::min(totalGoldCollected_, 180)) + " / 180 gold";
    case 3:
        return std::to_string(std::min(relicsPurchased_, 3)) + " / 3 relics owned";
    case 4:
        return std::to_string(std::min(weaponsPurchased_, 1)) + " / 1 crafted weapon";
    case 5:
        return boolProgressLabel(hasVisitedLevel("level5.json"), "Trial entered", "Trial pending") +
            std::string("  |  ") + std::to_string(std::min(totalGoldCollected_, 420)) + " / 420 gold";
    case 6:
        return boolProgressLabel(hasVisitedLevel("level6.json"), "Veil entered", "Veil pending") +
            std::string("  |  ") + std::to_string(std::min(weaponsPurchased_, 2)) + " / 2 weapons";
    default:
        return "Every route is open.";
    }
}

std::string CampaignProgress::getStageReward(std::size_t stageIndex) const
{
    switch (stageIndex)
    {
    case 0:
        return "Reward: Lantern Spark (+60 Vitality) and enough leverage for your first bargain.";
    case 1:
        return "Reward: Level II opens and Broker's Habit grants +1 Energy gain.";
    case 2:
        return "Reward: Level III opens and Rookery Tithe grants +8 Damage.";
    case 3:
        return "Reward: Level IV opens and Reliquary Reserve grants +140 Energy.";
    case 4:
        return "Reward: Level V opens and Ember Temper grants +80 Range.";
    case 5:
        return "Reward: Level VI opens and Veil Reflex grants -12 ms Shot CD.";
    case 6:
        return "Reward: Heart Rekindled grants +90 Vitality and +10 Damage.";
    default:
        return "Reward: endless harvest.";
    }
}
