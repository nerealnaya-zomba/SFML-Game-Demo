#pragma once

#include <Item.h>

#include <cstddef>
#include <set>
#include <string>
#include <vector>

struct CampaignLevelInfo
{
    std::string levelName;
    std::string title;
    std::string flavor;
    std::string description;
    std::string farmingFocus;
    std::string unlockRequirement;
};

struct CampaignObjectiveSnapshot
{
    std::string campaignTitle;
    std::string chapterTitle;
    std::string narrative;
    std::string objective;
    std::string progressText;
    std::string rewardText;
    float progressRatio = 0.f;
    bool storyComplete = false;
};

struct CampaignSaveState
{
    int totalGoldCollected = 0;
    int totalGoldSpent = 0;
    int relicsPurchased = 0;
    int weaponsPurchased = 0;
    std::vector<std::string> visitedLevels;
};

struct CampaignBoonState
{
    std::string title{"Dormant Covenant"};
    std::string description{"The sanctuary still sleeps."};
    int healthBonus = 0;
    int maxEnergyBonus = 0;
    int energyGainBonus = 0;
    int damageBonus = 0;
    int bulletRangeBonus = 0;
    int shootCooldownReduction = 0;
};

class CampaignProgress
{
public:
    CampaignProgress() = default;

    void onGoldCollected(int amount);
    void onGoldSpent(int amount);
    void onItemPurchased(Item::Category category);
    void onLevelEntered(const std::string& levelName);

    bool isLevelUnlocked(const std::string& levelName) const;
    std::vector<std::string> filterUnlockedLevels(const std::vector<std::string>& levels) const;
    CampaignObjectiveSnapshot buildSnapshot() const;
    std::string getLevelUnlockHint(const std::string& levelName) const;
    std::string getMerchantGreeting() const;
    std::string getMerchantAdvice() const;
    const CampaignLevelInfo& getRecommendedLevelInfo() const;
    CampaignBoonState getActiveBoonState() const;
    CampaignSaveState buildSaveState() const;
    void loadSaveState(const CampaignSaveState& state);

    int getTotalGoldCollected() const;
    int getTotalGoldSpent() const;
    int getRelicsPurchased() const;
    int getWeaponsPurchased() const;
    bool hasVisitedLevel(const std::string& levelName) const;
    std::size_t getCurrentStageIndex() const;

    static const CampaignLevelInfo& getLevelInfo(const std::string& levelName);

private:
    void refreshProgression();
    bool isStageComplete(std::size_t stageIndex) const;
    float getStageProgressRatio(std::size_t stageIndex) const;
    std::string getStageChapterTitle(std::size_t stageIndex) const;
    std::string getStageNarrative(std::size_t stageIndex) const;
    std::string getStageObjective(std::size_t stageIndex) const;
    std::string getStageProgressText(std::size_t stageIndex) const;
    std::string getStageReward(std::size_t stageIndex) const;

    int totalGoldCollected_ = 0;
    int totalGoldSpent_ = 0;
    int relicsPurchased_ = 0;
    int weaponsPurchased_ = 0;
    std::set<std::string> visitedLevels_{};
    std::size_t currentStageIndex_ = 0;
};
