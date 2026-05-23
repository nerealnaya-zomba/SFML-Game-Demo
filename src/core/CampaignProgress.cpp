#include <CampaignProgress.h>

#include <algorithm>
#include <array>
#include <sstream>

namespace
{
constexpr std::size_t kStoryStageCount = 6;

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
        "Dark Gate Sanctuary",
        "A quiet hub beneath the sealed gate, watched over by the trader.",
        "No enemies cross this threshold. Return here to buy gear, hear the next task and choose the next opened route.",
        "Best for: trading, planning and quest handoff.",
        "Open from the first step."
    },
    {
        "level2.json",
        "Obsidian Rookery",
        "A cold vertical roost where the trader sends you for the first gate fragment.",
        "Basalt scaffolds and bat-haunted spans hold the first proof that the Dark Gate can still be opened.",
        "Best for: the first combat quest and early grave-gold.",
        "Open from the first step."
    },
    {
        "level3.json",
        "Crimson Nave",
        "A ceremonial hall holding the second gate fragment.",
        "The red nave still burns with old ritual heat. The trader will not point you here until the Rookery debt is paid.",
        "Best for: the second quest step and denser enemy lines.",
        "Complete the Obsidian Rookery task."
    },
    {
        "level4.json",
        "Bone Reliquary",
        "A pale vault where the third fragment waits among old relic shelves.",
        "This crypt once stored covenant tools. Now it tests whether your kit is strong enough for deeper orders.",
        "Best for: the third quest step and relic-focused farming.",
        "Complete the Crimson Nave task."
    },
    {
        "level5.json",
        "Trial of Embers",
        "A compact proving ground where the fourth fragment is sealed in flame.",
        "The trader sends only armed hunters here. Weak builds burn out before the gate hears them.",
        "Best for: the fourth quest step and weapon checks.",
        "Complete the Bone Reliquary task."
    },
    {
        "level6.json",
        "The Returning Veil",
        "The final route before the Dark Gate answers.",
        "Beyond the last seal the ruined estate folds back on itself. Bring the trader enough proof and the gate will open.",
        "Best for: the final quest step and endgame farming.",
        "Complete the Trial of Embers task."
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
        return true;
    }
    if (levelName == "level3.json")
    {
        return currentStageIndex_ >= 1;
    }
    if (levelName == "level4.json")
    {
        return currentStageIndex_ >= 2;
    }
    if (levelName == "level5.json")
    {
        return currentStageIndex_ >= 3;
    }
    if (levelName == "level6.json")
    {
        return currentStageIndex_ >= 4;
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
    snapshot.campaignTitle = "Open the Dark Gate";
    snapshot.storyComplete = currentStageIndex_ >= kStoryStageCount;

    if (snapshot.storyComplete)
    {
        snapshot.chapterTitle = "VII. The Gate Stands Open";
        snapshot.narrative = "The trader's map is complete. Every route now feeds the open Dark Gate.";
        snapshot.objective = "Keep farming, forging and pushing the opened routes.";
        snapshot.progressText = std::to_string(weaponsPurchased_) + " weapons forged  |  " +
            std::to_string(totalGoldCollected_) + " grave-gold harvested";
        snapshot.rewardText = "Reward: free hunt across every opened route.";
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
        stream << "Seal: open from the hub. This is the trader's first task.\n"
               << "Progress: route available.";
        return stream.str();
    }
    if (levelName == "level3.json")
    {
        stream << "Seal: finish the Obsidian Rookery task.\n"
               << "Progress: "
               << boolProgressLabel(hasVisitedLevel("level2.json"), "Rookery entered", "Rookery not entered")
               << ", " << std::min(totalGoldCollected_, 60) << "/60 gold.";
        return stream.str();
    }
    if (levelName == "level4.json")
    {
        stream << "Seal: finish the Crimson Nave task.\n"
               << "Progress: "
               << boolProgressLabel(hasVisitedLevel("level3.json"), "Nave entered", "Nave not entered")
               << ", " << std::min(totalGoldCollected_, 180) << "/180 gold.";
        return stream.str();
    }
    if (levelName == "level5.json")
    {
        stream << "Seal: finish the Bone Reliquary task.\n"
               << "Progress: "
               << boolProgressLabel(hasVisitedLevel("level4.json"), "Reliquary entered", "Reliquary not entered")
               << ", " << std::min(relicsPurchased_, 3) << "/3 relics.";
        return stream.str();
    }
    if (levelName == "level6.json")
    {
        stream << "Seal: finish the Trial of Embers task.\n"
               << "Progress: "
               << boolProgressLabel(hasVisitedLevel("level5.json"), "Trial entered", "Trial not entered")
               << ", " << std::min(weaponsPurchased_, 1) << "/1 weapon.";
        return stream.str();
    }

    return "No seal binds this route.";
}

std::string CampaignProgress::getMerchantGreeting() const
{
    if (currentStageIndex_ >= kStoryStageCount)
    {
        return "The trader grins beneath the opened Dark Gate.";
    }

    switch (currentStageIndex_)
    {
    case 0:
        return "The trader waits under the Dark Gate with a marked map.";
    case 1:
        return "The trader seals the Rookery mark and opens a red road.";
    case 2:
        return "The merchant traces a crimson rune toward the Reliquary.";
    case 3:
        return "He checks your relics, then points at the ember seal.";
    case 4:
        return "The old scavenger stops smiling. The last road wants proof.";
    case 5:
        return "He listens to the Veil like it is answering him back.";
    default:
        return "The trader waits for your next decision.";
    }
}

std::string CampaignProgress::getMerchantAdvice() const
{
    switch (currentStageIndex_)
    {
    case 0:
        return "First job is not here. Take the opened route to Obsidian Rookery and bring back 60 grave-gold.";
    case 1:
        return "Good. The Crimson Nave is open. Go there next and push the total haul to 180 gold.";
    case 2:
        return "The Bone Reliquary is open. Enter it and come back owning 3 relics. The gate likes prepared hands.";
    case 3:
        return "Trial of Embers is open. Buy or forge a real weapon before you come bragging about progress.";
    case 4:
        return "The Returning Veil is open. Enter it and bring the total haul to 420 gold.";
    case 5:
        return "One final demand: own 2 crafted weapons. Then the Dark Gate has no excuse left.";
    default:
        return "Every route is open. Buy for greed, comfort or sheer spectacle now.";
    }
}

const CampaignLevelInfo& CampaignProgress::getRecommendedLevelInfo() const
{
    switch (currentStageIndex_)
    {
    case 0:
        return getLevelInfo("level2.json");
    case 1:
        return getLevelInfo("level3.json");
    case 2:
        return getLevelInfo("level4.json");
    case 3:
        return getLevelInfo("level5.json");
    default:
        return getLevelInfo("level6.json");
    }
}

CampaignBoonState CampaignProgress::getActiveBoonState() const
{
    CampaignBoonState boon;

    if (currentStageIndex_ >= 1)
    {
        boon.title = "Rookery Mark";
        boon.description = "The first gate mark toughens your body.";
        boon.healthBonus += 60;
    }
    if (currentStageIndex_ >= 2)
    {
        boon.title = "Nave Tithe";
        boon.description = "The red road teaches your bolts to hit harder.";
        boon.damageBonus += 8;
    }
    if (currentStageIndex_ >= 3)
    {
        boon.title = "Reliquary Reserve";
        boon.description = "Recovered relic rites deepen your arcane reserve.";
        boon.maxEnergyBonus += 140;
        boon.energyGainBonus += 1;
    }
    if (currentStageIndex_ >= 4)
    {
        boon.title = "Ember Temper";
        boon.description = "Trial-forged rites carry farther through the ruins.";
        boon.bulletRangeBonus += 80;
    }
    if (currentStageIndex_ >= 5)
    {
        boon.title = "Veil Reflex";
        boon.description = "The returning path teaches swifter release.";
        boon.shootCooldownReduction += 12;
    }
    if (currentStageIndex_ >= kStoryStageCount)
    {
        boon.title = "Dark Gate Opened";
        boon.description = "The opened gate strengthens every hunt.";
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
        return hasVisitedLevel("level2.json") && totalGoldCollected_ >= 60;
    case 1:
        return hasVisitedLevel("level3.json") && totalGoldCollected_ >= 180;
    case 2:
        return hasVisitedLevel("level4.json") && relicsPurchased_ >= 3;
    case 3:
        return hasVisitedLevel("level5.json") && weaponsPurchased_ >= 1;
    case 4:
        return hasVisitedLevel("level6.json") && totalGoldCollected_ >= 420;
    case 5:
        return weaponsPurchased_ >= 2;
    default:
        return true;
    }
}

float CampaignProgress::getStageProgressRatio(std::size_t stageIndex) const
{
    switch (stageIndex)
    {
    case 0:
        return (normalizedProgress(totalGoldCollected_, 60) + (hasVisitedLevel("level2.json") ? 1.f : 0.f)) * 0.5f;
    case 1:
        return (normalizedProgress(totalGoldCollected_, 180) + (hasVisitedLevel("level3.json") ? 1.f : 0.f)) * 0.5f;
    case 2:
        return (normalizedProgress(relicsPurchased_, 3) + (hasVisitedLevel("level4.json") ? 1.f : 0.f)) * 0.5f;
    case 3:
        return (normalizedProgress(weaponsPurchased_, 1) + (hasVisitedLevel("level5.json") ? 1.f : 0.f)) * 0.5f;
    case 4:
        return (normalizedProgress(totalGoldCollected_, 420) + (hasVisitedLevel("level6.json") ? 1.f : 0.f)) * 0.5f;
    case 5:
        return normalizedProgress(weaponsPurchased_, 2);
    default:
        return 1.f;
    }
}

std::string CampaignProgress::getStageChapterTitle(std::size_t stageIndex) const
{
    switch (stageIndex)
    {
    case 0:
        return "I. First Mark: Obsidian Rookery";
    case 1:
        return "II. Second Mark: Crimson Nave";
    case 2:
        return "III. Third Mark: Bone Reliquary";
    case 3:
        return "IV. Fourth Mark: Trial of Embers";
    case 4:
        return "V. Fifth Mark: The Returning Veil";
    case 5:
        return "VI. Final Payment";
    default:
        return "VII. Open Gate";
    }
}

std::string CampaignProgress::getStageNarrative(std::size_t stageIndex) const
{
    switch (stageIndex)
    {
    case 0:
        return "The Sanctuary is safe, but the Dark Gate will not open from comfort. The trader marks Obsidian Rookery first.";
    case 1:
        return "The first mark is paid. A red route opens, and the trader wants the next fragment from Crimson Nave.";
    case 2:
        return "The Reliquary seal asks for proof that you can turn loot into lasting power.";
    case 3:
        return "The Trial of Embers wants a weapon, not optimism.";
    case 4:
        return "Only the Returning Veil remains between the trader's map and the Dark Gate.";
    case 5:
        return "The route marks are gathered. The trader demands one final investment before he turns the key.";
    default:
        return "The harvest goes on.";
    }
}

std::string CampaignProgress::getStageObjective(std::size_t stageIndex) const
{
    switch (stageIndex)
    {
    case 0:
        return "Enter Obsidian Rookery and return with 60 total grave-gold.";
    case 1:
        return "Enter Crimson Nave and reach 180 total grave-gold.";
    case 2:
        return "Enter Bone Reliquary and own 3 relics.";
    case 3:
        return "Enter Trial of Embers and own 1 crafted weapon.";
    case 4:
        return "Enter The Returning Veil and reach 420 total grave-gold.";
    case 5:
        return "Own 2 crafted weapons to open the Dark Gate.";
    default:
        return "Keep the opened routes rich.";
    }
}

std::string CampaignProgress::getStageProgressText(std::size_t stageIndex) const
{
    switch (stageIndex)
    {
    case 0:
        return boolProgressLabel(hasVisitedLevel("level2.json"), "Rookery entered", "Rookery pending") +
            std::string("  |  ") + std::to_string(std::min(totalGoldCollected_, 60)) + " / 60 gold";
    case 1:
        return boolProgressLabel(hasVisitedLevel("level3.json"), "Nave entered", "Nave pending") +
            std::string("  |  ") + std::to_string(std::min(totalGoldCollected_, 180)) + " / 180 gold";
    case 2:
        return boolProgressLabel(hasVisitedLevel("level4.json"), "Reliquary entered", "Reliquary pending") +
            std::string("  |  ") + std::to_string(std::min(relicsPurchased_, 3)) + " / 3 relics";
    case 3:
        return boolProgressLabel(hasVisitedLevel("level5.json"), "Trial entered", "Trial pending") +
            std::string("  |  ") + std::to_string(std::min(weaponsPurchased_, 1)) + " / 1 weapon";
    case 4:
        return boolProgressLabel(hasVisitedLevel("level6.json"), "Veil entered", "Veil pending") +
            std::string("  |  ") + std::to_string(std::min(totalGoldCollected_, 420)) + " / 420 gold";
    case 5:
        return std::to_string(std::min(weaponsPurchased_, 2)) + " / 2 crafted weapons";
    default:
        return "Every route is open.";
    }
}

std::string CampaignProgress::getStageReward(std::size_t stageIndex) const
{
    switch (stageIndex)
    {
    case 0:
        return "Reward: Level III opens and Rookery Mark grants +60 Vitality.";
    case 1:
        return "Reward: Level IV opens and Nave Tithe grants +8 Damage.";
    case 2:
        return "Reward: Level V opens and Reliquary Reserve grants +140 Energy and +1 Energy gain.";
    case 3:
        return "Reward: Level VI opens and Ember Temper grants +80 Range.";
    case 4:
        return "Reward: Veil Reflex grants -12 ms Shot CD.";
    case 5:
        return "Reward: Dark Gate Opened grants +90 Vitality and +10 Damage.";
    default:
        return "Reward: endless harvest.";
    }
}
