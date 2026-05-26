#include <CampaignProgress.h>
#include <Localization.h>

#include <algorithm>
#include <array>
#include <sstream>

namespace
{
constexpr std::size_t kStoryStageCount = 16;
constexpr std::size_t kRookeryBoonQuest = 4;
constexpr std::size_t kNaveBoonQuest = 7;
constexpr std::size_t kReliquaryBoonQuest = 10;
constexpr std::size_t kEmberBoonQuest = 13;
constexpr std::size_t kVeilBoonQuest = 15;

const CampaignLevelInfo kFallbackLevelInfo{
    "unknown",
    "Unwritten Ground",
    "A route not yet named by the covenant.",
    "No chronicler has written what waits there.",
    "Best for: unknown returns.",
    "Its seal is not yet understood."
};

const CampaignLevelInfo kFallbackLevelInfoRu{
    "unknown",
    "Неназванная земля",
    "Путь, которому ковенант еще не дал имени.",
    "Ни один летописец еще не записал, что там ждет.",
    "Лучше всего для: неизвестной добычи.",
    "Его печать пока не разгадана."
};

const std::array<CampaignLevelInfo, 6> kCampaignLevels{{
    {
        "level1.json",
        "Veilfall Cascades",
        "A quiet hub beneath the sealed gate, watched over by the trader.",
        "No enemies cross this threshold. Return here to buy gear, hear the next task and choose the next opened route.",
        "Best for: trading, planning and quest handoff.",
        "Open from the first step."
    },
    {
        "level2.json",
        "Skybound Spires",
        "A cold vertical roost where the trader sends you for the first gate fragment.",
        "Basalt scaffolds and bat-haunted spans hold the first proof that the Dark Gate can still be opened.",
        "Best for: the first combat quest and early grave-gold.",
        "Open after first entering the hub."
    },
    {
        "level3.json",
        "Sunset Bastion",
        "A ceremonial hall holding the second gate fragment.",
        "The red nave still burns with old ritual heat. The trader will not point you here until the Rookery debt is paid.",
        "Best for: the second quest step and denser enemy lines.",
        "Open after first entering Skybound Spires."
    },
    {
        "level4.json",
        "Grave Hollow",
        "A pale vault where the third fragment waits among old relic shelves.",
        "This crypt once stored covenant tools. Now it tests whether your kit is strong enough for deeper orders.",
        "Best for: the third quest step and relic-focused farming.",
        "Open after first entering Sunset Bastion."
    },
    {
        "level5.json",
        "Ruins of the Elder City",
        "A compact proving ground where the fourth fragment is sealed in flame.",
        "The trader sends only armed hunters here. Weak builds burn out before the gate hears them.",
        "Best for: the fourth quest step and weapon checks.",
        "Open after first entering Grave Hollow."
    },
    {
        "level6.json",
        "Aurora Nightpeaks",
        "The final route before the Dark Gate answers.",
        "Beyond the last seal the ruined estate folds back on itself. Bring the trader enough proof and the gate will open.",
        "Best for: the final quest step and endgame farming.",
        "Open after first entering Ruins of the Elder City."
    }
}};

const std::array<CampaignLevelInfo, 6> kCampaignLevelsRu{{
    {
        "level1.json",
        "Водопады Вуали",
        "Тихий хаб под запечатанными вратами, где ждет торговец.",
        "Враги не переступают этот порог. Возвращайтесь сюда за снаряжением, следующей задачей и новым открытым путем.",
        "Лучше всего для: торговли, подготовки и получения заданий.",
        "Открыто с самого начала."
    },
    {
        "level2.json",
        "Небесные шпили",
        "Холодная вертикальная насестная локация, куда торговец отправляет за первым фрагментом врат.",
        "Базальтовые леса и пролеты с летучими тварями хранят первое доказательство, что Темные врата еще можно открыть.",
        "Лучше всего для: первого боевого задания и раннего могильного золота.",
        "Открывается после первого посещения хаба."
    },
    {
        "level3.json",
        "Закатный бастион",
        "Ритуальный зал, где хранится второй фрагмент врат.",
        "Красный неф все еще горит древним обрядовым жаром. Торговец укажет сюда только после платы за Гнездовье.",
        "Лучше всего для: второго этапа задания и более плотных волн врагов.",
        "Открывается после первого посещения Обсидианового гнездовья."
    },
    {
        "level4.json",
        "Могильная лощина",
        "Бледное хранилище, где третий фрагмент ждет среди старых полок с реликвиями.",
        "Эта крипта когда-то хранила инструменты ковенанта. Теперь она проверяет, достаточно ли силен ваш набор для более глубоких приказов.",
        "Лучше всего для: третьего этапа задания и фарма реликвий.",
        "Открывается после первого посещения Багрового нефа."
    },
    {
        "level5.json",
        "Руины старого города",
        "Компактный полигон, где четвертый фрагмент запечатан в пламени.",
        "Торговец отправляет сюда только вооруженных охотников. Слабые сборки сгорают раньше, чем врата их услышат.",
        "Лучше всего для: четвертого этапа задания и проверки оружия.",
        "Открывается после первого посещения Костяного реликвария."
    },
    {
        "level6.json",
        "Ночные пики Авроры",
        "Последний путь перед ответом Темных врат.",
        "За последней печатью разрушенное поместье сворачивается само в себя. Принесите торговцу достаточно доказательств, и врата откроются.",
        "Лучше всего для: финального этапа задания и позднего фарма.",
        "Открывается после первого посещения Испытания углей."
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
        return hasVisitedLevel("level2.json");
    }
    if (levelName == "level3.json")
    {
        return hasVisitedLevel("level3.json");
    }
    if (levelName == "level4.json")
    {
        return hasVisitedLevel("level4.json");
    }
    if (levelName == "level5.json")
    {
        return hasVisitedLevel("level5.json");
    }
    if (levelName == "level6.json")
    {
        return hasVisitedLevel("level6.json");
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
    snapshot.campaignTitle = Localization::isRussian() ? "Открыть Темные врата" : "Open the Dark Gate";
    snapshot.storyComplete = currentStageIndex_ >= kStoryStageCount;

    if (snapshot.storyComplete)
    {
        snapshot.chapterTitle = Localization::isRussian() ? "XVII. Открытые врата" : "XVII. The Gate Stands Open";
        snapshot.narrative = Localization::isRussian()
            ? "Карта торговца завершена. Каждый найденный путь теперь питает открытые Темные врата."
            : "The trader's map is complete. Every route now feeds the open Dark Gate.";
        snapshot.objective = Localization::isRussian()
            ? "Продолжайте охотиться, покупать снаряжение и возвращаться на открытые маршруты."
            : "Keep farming, forging and pushing the opened routes.";
        snapshot.progressText = Localization::isRussian()
            ? std::to_string(weaponsPurchased_) + " оружия выковано  |  " + std::to_string(totalGoldCollected_) + " могильного золота собрано"
            : std::to_string(weaponsPurchased_) + " weapons forged  |  " + std::to_string(totalGoldCollected_) + " grave-gold harvested";
        snapshot.rewardText = Localization::isRussian()
            ? "Награда: свободная охота на всех открытых путях."
            : "Reward: free hunt across every opened route.";
        snapshot.progressRatio = 1.f;
        return snapshot;

        snapshot.chapterTitle = Localization::isRussian() ? "VII. Врата открыты" : "VII. The Gate Stands Open";
        snapshot.narrative = Localization::isRussian()
            ? "Карта торговца завершена. Каждый путь теперь питает открытые Темные врата."
            : "The trader's map is complete. Every route now feeds the open Dark Gate.";
        snapshot.objective = Localization::isRussian()
            ? "Продолжайте фармить, ковать и продвигаться по открытым путям."
            : "Keep farming, forging and pushing the opened routes.";
        snapshot.progressText = std::to_string(weaponsPurchased_) + " weapons forged  |  " +
            std::to_string(totalGoldCollected_) + " grave-gold harvested";
        if (Localization::isRussian())
        {
            snapshot.progressText = std::to_string(weaponsPurchased_) + " оружия выковано  |  " +
                std::to_string(totalGoldCollected_) + " могильного золота собрано";
        }
        snapshot.rewardText = Localization::isRussian()
            ? "Награда: свободная охота на всех открытых путях."
            : "Reward: free hunt across every opened route.";
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
    const auto makePortalHint = [&](const std::string& targetLevel, const std::string& targetTitle) {
        if (Localization::isRussian())
        {
            return std::string("Путь появится после первого входа через портал: ") + targetTitle + ".\n" +
                "Прогресс: " + boolProgressLabel(hasVisitedLevel(targetLevel), "уровень посещен", "уровень еще не посещен");
        }
        return std::string("Route appears after first entering through its portal: ") + targetTitle + ".\n" +
            "Progress: " + boolProgressLabel(hasVisitedLevel(targetLevel), "level entered", "level not entered");
    };

    if (levelName == "level1.json")
    {
        return Localization::isRussian() ? "Хаб открыт всегда." : "The hub is always open.";
    }
    if (levelName == "level2.json")
    {
        return makePortalHint("level2.json", getLevelInfo("level2.json").title);
    }
    if (levelName == "level3.json")
    {
        return makePortalHint("level3.json", getLevelInfo("level3.json").title);
    }
    if (levelName == "level4.json")
    {
        return makePortalHint("level4.json", getLevelInfo("level4.json").title);
    }
    if (levelName == "level5.json")
    {
        return makePortalHint("level5.json", getLevelInfo("level5.json").title);
    }
    if (levelName == "level6.json")
    {
        return makePortalHint("level6.json", getLevelInfo("level6.json").title);
    }

    std::ostringstream stream;

    if (levelName == "level2.json")
    {
        if (Localization::isRussian())
        {
            stream << "Печать: открыто из хаба. Это первое задание торговца.\n"
                   << "Прогресс: путь доступен.";
            return stream.str();
        }
        stream << "Seal: open from the hub. This is the trader's first task.\n"
               << "Progress: route available.";
        return stream.str();
    }
    if (levelName == "level3.json")
    {
        if (Localization::isRussian())
        {
            stream << "Печать: завершите задачу в Обсидиановом гнездовье.\n"
                   << "Прогресс: "
                   << boolProgressLabel(hasVisitedLevel("level2.json"), "Гнездовье посещено", "Гнездовье не посещено")
                   << ", " << std::min(totalGoldCollected_, 60) << "/60 золота.";
            return stream.str();
        }
        stream << "Seal: finish the Skybound Spires task.\n"
               << "Progress: "
               << boolProgressLabel(hasVisitedLevel("level2.json"), "Rookery entered", "Rookery not entered")
               << ", " << std::min(totalGoldCollected_, 60) << "/60 gold.";
        return stream.str();
    }
    if (levelName == "level4.json")
    {
        if (Localization::isRussian())
        {
            stream << "Печать: завершите задачу в Багровом нефе.\n"
                   << "Прогресс: "
                   << boolProgressLabel(hasVisitedLevel("level3.json"), "Неф посещен", "Неф не посещен")
                   << ", " << std::min(totalGoldCollected_, 180) << "/180 золота.";
            return stream.str();
        }
        stream << "Seal: finish the Sunset Bastion task.\n"
               << "Progress: "
               << boolProgressLabel(hasVisitedLevel("level3.json"), "Nave entered", "Nave not entered")
               << ", " << std::min(totalGoldCollected_, 180) << "/180 gold.";
        return stream.str();
    }
    if (levelName == "level5.json")
    {
        if (Localization::isRussian())
        {
            stream << "Печать: завершите задачу в Костяном реликварии.\n"
                   << "Прогресс: "
                   << boolProgressLabel(hasVisitedLevel("level4.json"), "Реликварий посещен", "Реликварий не посещен")
                   << ", " << std::min(relicsPurchased_, 3) << "/3 реликвии.";
            return stream.str();
        }
        stream << "Seal: finish the Grave Hollow task.\n"
               << "Progress: "
               << boolProgressLabel(hasVisitedLevel("level4.json"), "Reliquary entered", "Reliquary not entered")
               << ", " << std::min(relicsPurchased_, 3) << "/3 relics.";
        return stream.str();
    }
    if (levelName == "level6.json")
    {
        if (Localization::isRussian())
        {
            stream << "Печать: завершите Испытание углей.\n"
                   << "Прогресс: "
                   << boolProgressLabel(hasVisitedLevel("level5.json"), "Испытание посещено", "Испытание не посещено")
                   << ", " << std::min(weaponsPurchased_, 1) << "/1 оружие.";
            return stream.str();
        }
        stream << "Seal: finish the Ruins of the Elder City task.\n"
               << "Progress: "
               << boolProgressLabel(hasVisitedLevel("level5.json"), "Trial entered", "Trial not entered")
               << ", " << std::min(weaponsPurchased_, 1) << "/1 weapon.";
        return stream.str();
    }

    return Localization::isRussian() ? "Этот путь не скован печатью." : "No seal binds this route.";
}

std::string CampaignProgress::getMerchantGreeting() const
{
    const CampaignLevelInfo& recommended = getRecommendedLevelInfo();
    if (Localization::isRussian())
    {
        if (currentStageIndex_ >= kStoryStageCount)
        {
            return "Торговец улыбается под открытыми Темными вратами.";
        }
        return "Торговец отмечает маршрут: " + recommended.title + ".";
    }

    if (currentStageIndex_ >= kStoryStageCount)
    {
        return "The trader grins beneath the opened Dark Gate.";
    }
    return "The trader marks the route: " + recommended.title + ".";

    if (Localization::isRussian())
    {
        if (currentStageIndex_ >= kStoryStageCount)
        {
            return "Торговец улыбается под открытыми Темными вратами.";
        }
        switch (currentStageIndex_)
        {
        case 0: return "Торговец ждет под Темными вратами с помеченной картой.";
        case 1: return "Торговец запечатывает метку Гнездовья и открывает красную дорогу.";
        case 2: return "Купец чертит багровую руну в сторону Реликвария.";
        case 3: return "Он проверяет ваши реликвии и указывает на угольную печать.";
        case 4: return "Старый сборщик перестает улыбаться. Последний путь требует доказательств.";
        case 5: return "Он слушает Вуаль так, будто она отвечает ему.";
        default: return "Торговец ждет вашего решения.";
        }
    }

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
    return getStageObjective(currentStageIndex_);

    if (Localization::isRussian())
    {
        switch (currentStageIndex_)
        {
        case 0: return "Первое дело не здесь. Идите открытым путем в Обсидиановое гнездовье и принесите 60 могильного золота.";
        case 1: return "Хорошо. Багровый неф открыт. Идите туда дальше и доведите общую добычу до 180 золота.";
        case 2: return "Костяной реликварий открыт. Войдите туда и вернитесь с 3 реликвиями. Врата любят подготовленные руки.";
        case 3: return "Испытание углей открыто. Купите или выкуйте настоящее оружие, прежде чем хвастаться прогрессом.";
        case 4: return "Возвращающаяся вуаль открыта. Войдите туда и доведите общую добычу до 420 золота.";
        case 5: return "Последнее требование: владейте 2 выкованными оружиями. Тогда у Темных врат не останется оправданий.";
        default: return "Все пути открыты. Покупайте ради жадности, удобства или чистого зрелища.";
        }
    }

    switch (currentStageIndex_)
    {
    case 0:
        return "First job is not here. Take the opened route to Skybound Spires and bring back 60 grave-gold.";
    case 1:
        return "Good. Sunset Bastion is open. Go there next and push the total haul to 180 gold.";
    case 2:
        return "Grave Hollow is open. Enter it and come back owning 3 relics. The gate likes prepared hands.";
    case 3:
        return "Ruins of the Elder City is open. Buy or forge a real weapon before you come bragging about progress.";
    case 4:
        return "Aurora Nightpeaks is open. Enter it and bring the total haul to 420 gold.";
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
    case 1:
    case 2:
    case 3:
        return getLevelInfo("level2.json");
    case 4:
    case 5:
    case 6:
        return getLevelInfo("level3.json");
    case 7:
    case 8:
    case 9:
        return getLevelInfo("level4.json");
    case 10:
    case 11:
    case 12:
        return getLevelInfo("level5.json");
    default:
        return getLevelInfo("level6.json");
    }
}

CampaignBoonState CampaignProgress::getActiveBoonState() const
{
    CampaignBoonState boon;
    if (Localization::isRussian())
    {
        boon.title = "Спящий договор";
        boon.description = "Святилище все еще спит.";
    }

    if (currentStageIndex_ >= kRookeryBoonQuest)
    {
        boon.title = Localization::isRussian() ? "Метка Гнездовья" : "Rookery Mark";
        boon.description = Localization::isRussian() ? "Первая метка врат укрепляет тело." : "The first gate mark toughens your body.";
        boon.healthBonus += 60;
    }
    if (currentStageIndex_ >= kNaveBoonQuest)
    {
        boon.title = Localization::isRussian() ? "Десятина Нефа" : "Nave Tithe";
        boon.description = Localization::isRussian() ? "Красная дорога учит заряды бить сильнее." : "The red road teaches your bolts to hit harder.";
        boon.damageBonus += 8;
    }
    if (currentStageIndex_ >= kReliquaryBoonQuest)
    {
        boon.title = Localization::isRussian() ? "Запас Реликвария" : "Reliquary Reserve";
        boon.description = Localization::isRussian() ? "Возвращенные реликтовые обряды расширяют тайный резерв." : "Recovered relic rites deepen your arcane reserve.";
        boon.maxEnergyBonus += 140;
        boon.energyGainBonus += 1;
    }
    if (currentStageIndex_ >= kEmberBoonQuest)
    {
        boon.title = Localization::isRussian() ? "Угольная закалка" : "Ember Temper";
        boon.description = Localization::isRussian() ? "Ритуалы, закаленные испытанием, летят дальше сквозь руины." : "Trial-forged rites carry farther through the ruins.";
        boon.bulletRangeBonus += 80;
    }
    if (currentStageIndex_ >= kVeilBoonQuest)
    {
        boon.title = Localization::isRussian() ? "Рефлекс Вуали" : "Veil Reflex";
        boon.description = Localization::isRussian() ? "Возвращающийся путь учит быстрее выпускать заряды." : "The returning path teaches swifter release.";
        boon.shootCooldownReduction += 12;
    }
    if (currentStageIndex_ >= kStoryStageCount)
    {
        boon.title = Localization::isRussian() ? "Темные врата открыты" : "Dark Gate Opened";
        boon.description = Localization::isRussian() ? "Открытые врата усиливают каждую охоту." : "The opened gate strengthens every hunt.";
        boon.healthBonus += 90;
        boon.damageBonus += 10;
    }

    if (Localization::isRussian())
    {
        if (currentStageIndex_ >= kStoryStageCount)
        {
            boon.title = "Темные врата открыты";
            boon.description = "Открытые врата усиливают каждую охоту.";
        }
        else if (currentStageIndex_ >= kVeilBoonQuest)
        {
            boon.title = "Рефлекс Вуали";
            boon.description = "Возвращающийся путь учит быстрее выпускать заряды.";
        }
        else if (currentStageIndex_ >= kEmberBoonQuest)
        {
            boon.title = "Угольная закалка";
            boon.description = "Ритуалы, закаленные испытанием, летят дальше сквозь руины.";
        }
        else if (currentStageIndex_ >= kReliquaryBoonQuest)
        {
            boon.title = "Запас Реликвария";
            boon.description = "Возвращенные реликтовые обряды расширяют тайный резерв.";
        }
        else if (currentStageIndex_ >= kNaveBoonQuest)
        {
            boon.title = "Десятина Нефа";
            boon.description = "Красная дорога учит заряды бить сильнее.";
        }
        else if (currentStageIndex_ >= kRookeryBoonQuest)
        {
            boon.title = "Метка Гнездовья";
            boon.description = "Первая метка врат укрепляет тело.";
        }
        else
        {
            boon.title = "Спящий договор";
            boon.description = "Святилище все еще спит.";
        }
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
    const auto& levels = Localization::isRussian() ? kCampaignLevelsRu : kCampaignLevels;
    const auto levelIt = std::find_if(
        levels.begin(),
        levels.end(),
        [&](const CampaignLevelInfo& levelInfo) {
            return levelInfo.levelName == levelName;
        }
    );

    return levelIt != levels.end() ? *levelIt : (Localization::isRussian() ? kFallbackLevelInfoRu : kFallbackLevelInfo);
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
        return hasVisitedLevel("level2.json");
    case 1:
        return hasVisitedLevel("level3.json");
    case 2:
        return relicsPurchased_ >= 1;
    case 3:
        return hasVisitedLevel("level3.json");
    case 4:
        return totalGoldCollected_ >= 180;
    case 5:
        return weaponsPurchased_ >= 1;
    case 6:
        return hasVisitedLevel("level4.json");
    case 7:
        return relicsPurchased_ >= 3;
    case 8:
        return totalGoldCollected_ >= 300;
    case 9:
        return hasVisitedLevel("level5.json");
    case 10:
        return totalGoldCollected_ >= 420;
    case 11:
        return weaponsPurchased_ >= 2;
    case 12:
        return hasVisitedLevel("level6.json");
    case 13:
        return totalGoldCollected_ >= 600;
    case 14:
        return relicsPurchased_ >= 4;
    case 15:
        return weaponsPurchased_ >= 3;
    default:
        return true;
    }
}

float CampaignProgress::getStageProgressRatio(std::size_t stageIndex) const
{
    switch (stageIndex)
    {
    case 0:
        return hasVisitedLevel("level2.json") ? 1.f : 0.f;
    case 1:
        return hasVisitedLevel("level3.json") ? 1.f : 0.f;
    case 2:
        return normalizedProgress(relicsPurchased_, 1);
    case 3:
        return hasVisitedLevel("level3.json") ? 1.f : 0.f;
    case 4:
        return normalizedProgress(totalGoldCollected_, 180);
    case 5:
        return normalizedProgress(weaponsPurchased_, 1);
    case 6:
        return hasVisitedLevel("level4.json") ? 1.f : 0.f;
    case 7:
        return normalizedProgress(relicsPurchased_, 3);
    case 8:
        return normalizedProgress(totalGoldCollected_, 300);
    case 9:
        return hasVisitedLevel("level5.json") ? 1.f : 0.f;
    case 10:
        return normalizedProgress(totalGoldCollected_, 420);
    case 11:
        return normalizedProgress(weaponsPurchased_, 2);
    case 12:
        return hasVisitedLevel("level6.json") ? 1.f : 0.f;
    case 13:
        return normalizedProgress(totalGoldCollected_, 600);
    case 14:
        return normalizedProgress(relicsPurchased_, 4);
    case 15:
        return normalizedProgress(weaponsPurchased_, 3);
    default:
        return 1.f;
    }
}

std::string CampaignProgress::getStageChapterTitle(std::size_t stageIndex) const
{
    if (Localization::isRussian())
    {
        switch (stageIndex)
        {
        case 0: return "I. Водопады: верхний портал";
        case 1: return "II. Небесные шпили: поиск портала";
        case 2: return "III. Гнездовье: первая реликвия";
        case 3: return "IV. Гнездовье: портал в Неф";
        case 4: return "V. Неф: красная десятина";
        case 5: return "VI. Неф: первое оружие";
        case 6: return "VII. Неф: портал в Реликварий";
        case 7: return "VIII. Реликварий: запас силы";
        case 8: return "IX. Реликварий: глубокая добыча";
        case 9: return "X. Реликварий: портал к Углям";
        case 10: return "XI. Угли: тяжелая плата";
        case 11: return "XII. Угли: второй клинок";
        case 12: return "XIII. Угли: портал в Вуаль";
        case 13: return "XIV. Вуаль: последняя жатва";
        case 14: return "XV. Вуаль: четыре реликвии";
        case 15: return "XVI. Темные врата: финальный ключ";
        default: return "XVII. Открытые врата";
        }
    }

    switch (stageIndex)
    {
    case 0: return "I. Cascades: Upper Portal";
    case 1: return "II. Sky Spires: Find the Portal";
    case 2: return "III. Rookery: First Relic";
    case 3: return "IV. Rookery: Portal to the Nave";
    case 4: return "V. Nave: Crimson Tithe";
    case 5: return "VI. Nave: First Weapon";
    case 6: return "VII. Nave: Portal to the Reliquary";
    case 7: return "VIII. Reliquary: Stored Power";
    case 8: return "IX. Reliquary: Deep Haul";
    case 9: return "X. Reliquary: Portal to Embers";
    case 10: return "XI. Embers: Heavy Payment";
    case 11: return "XII. Embers: Second Blade";
    case 12: return "XIII. Embers: Portal to the Veil";
    case 13: return "XIV. Veil: Last Harvest";
    case 14: return "XV. Veil: Four Relics";
    case 15: return "XVI. Dark Gate: Final Key";
    default: return "XVII. Open Gate";
    }

    if (Localization::isRussian())
    {
        switch (stageIndex)
        {
        case 0: return "I. Первая метка: Обсидиановое гнездовье";
        case 1: return "II. Вторая метка: Багровый неф";
        case 2: return "III. Третья метка: Костяной реликварий";
        case 3: return "IV. Четвертая метка: Испытание углей";
        case 4: return "V. Пятая метка: Возвращающаяся вуаль";
        case 5: return "VI. Последняя плата";
        default: return "VII. Открытые врата";
        }
    }

    switch (stageIndex)
    {
    case 0:
        return "I. First Mark: Skybound Spires";
    case 1:
        return "II. Second Mark: Sunset Bastion";
    case 2:
        return "III. Third Mark: Grave Hollow";
    case 3:
        return "IV. Fourth Mark: Ruins of the Elder City";
    case 4:
        return "V. Fifth Mark: Aurora Nightpeaks";
    case 5:
        return "VI. Final Payment";
    default:
        return "VII. Open Gate";
    }
}

std::string CampaignProgress::getStageNarrative(std::size_t stageIndex) const
{
    if (Localization::isRussian())
    {
        switch (stageIndex)
        {
        case 0: return "Найдите портал наверху Водопадов Вуали.";
        case 1: return "Обыщите Небесные шпили и найдите портал дальше.";
        case 2: return "Первая реликвия закрепит маршрут. Торговец хочет видеть не обещания, а вещь в инвентаре.";
        case 3: return "Где-то в Гнездовье спрятан путь дальше. Найдите портал и пройдите в Багровый неф.";
        case 4: return "Неф требует большую десятину. Красная дорога открывается тем, кто умеет выживать долго.";
        case 5: return "Без оружия глубже идти глупо. Купите или выкуйте первый настоящий инструмент охоты.";
        case 6: return "Неф не конец. Найдите портал, ведущий к Костяному реликварию.";
        case 7: return "Реликварий проверяет, можете ли вы превратить найденное в постоянную силу.";
        case 8: return "В глубине реликвария достаточно золота, чтобы подготовить следующий переход.";
        case 9: return "Осталось найти выход из реликвария к Испытанию углей.";
        case 10: return "Угли требуют тяжелую плату: запас, оружие и терпение.";
        case 11: return "Один клинок открывает дорогу, второй позволяет пережить ее.";
        case 12: return "В Испытании есть портал к Возвращающейся вуали. Найдите его.";
        case 13: return "Вуаль собирает последнюю жатву перед Темными вратами.";
        case 14: return "Четыре реликвии удержат маршрут, когда врата начнут отвечать.";
        case 15: return "Финальный ключ - третье оружие. После этого торговцу нечем будет вас задержать.";
        default: return "Темные врата открыты. Охота продолжается по всем найденным путям.";
        }
    }

    switch (stageIndex)
    {
    case 0: return "Find the portal above Veilfall Cascades.";
    case 1: return "Search Skybound Spires for the next portal.";
    case 2: return "A first relic will anchor the route. The trader wants a thing in your pack, not a promise.";
    case 3: return "Somewhere in the Spires is the way onward. Find the portal and enter Sunset Bastion.";
    case 4: return "The Nave asks for a larger tithe. The red road opens to hunters who can last.";
    case 5: return "Going deeper without a weapon is foolish. Buy or forge the first real hunting tool.";
    case 6: return "The Bastion is not the end. Find the portal to Grave Hollow.";
    case 7: return "The Reliquary tests whether loot can become lasting power.";
    case 8: return "Deep in the Reliquary lies enough gold to prepare the next crossing.";
    case 9: return "Find the Hollow exit that leads to Ruins of the Elder City.";
    case 10: return "The Embers demand a heavy payment: reserves, weaponry and patience.";
    case 11: return "One blade opens the road. A second helps you survive it.";
    case 12: return "A portal in the Trial leads to the Returning Veil. Find it.";
    case 13: return "The Veil gathers the last harvest before the Dark Gate answers.";
    case 14: return "Four relics will hold the route when the gate begins to answer.";
    case 15: return "The final key is a third weapon. After that, the trader has no excuse left.";
    default: return "The Dark Gate is open. The hunt continues across every found route.";
    }

    if (Localization::isRussian())
    {
        switch (stageIndex)
        {
        case 0: return "Святилище безопасно, но Темные врата не откроются от удобства. Торговец первым отмечает Обсидиановое гнездовье.";
        case 1: return "Первая метка оплачена. Красный путь открыт, и торговец хочет следующий фрагмент из Багрового нефа.";
        case 2: return "Печать Реликвария требует доказать, что вы умеете превращать добычу в постоянную силу.";
        case 3: return "Испытание углей требует оружие, а не оптимизм.";
        case 4: return "Между картой торговца и Темными вратами осталась только Возвращающаяся вуаль.";
        case 5: return "Метки путей собраны. Торговец требует последнее вложение, прежде чем повернуть ключ.";
        default: return "Жатва продолжается.";
        }
    }

    switch (stageIndex)
    {
    case 0:
        return "The Cascades are safe, but the Dark Gate will not open from comfort. The trader marks Skybound Spires first.";
    case 1:
        return "The first mark is paid. A red route opens, and the trader wants the next fragment from Sunset Bastion.";
    case 2:
        return "The Reliquary seal asks for proof that you can turn loot into lasting power.";
    case 3:
        return "Ruins of the Elder City wants a weapon, not optimism.";
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
    if (Localization::isRussian())
    {
        switch (stageIndex)
        {
        case 0: return "Поднимитесь наверх и войдите в портал на уровень II.";
        case 1: return "Найдите портал на уровень III.";
        case 2: return "Купите или получите 1 реликвию.";
        case 3: return "Найдите портал в Багровый неф и войдите на уровень III.";
        case 4: return "Доведите общую добычу до 180 могильного золота.";
        case 5: return "Купите или получите 1 оружие.";
        case 6: return "Найдите портал в Костяной реликварий и войдите на уровень IV.";
        case 7: return "Соберите 3 реликвии.";
        case 8: return "Доведите общую добычу до 300 могильного золота.";
        case 9: return "Найдите портал к Испытанию углей и войдите на уровень V.";
        case 10: return "Доведите общую добычу до 420 могильного золота.";
        case 11: return "Получите 2 оружия.";
        case 12: return "Найдите портал в Возвращающуюся вуаль и войдите на уровень VI.";
        case 13: return "Доведите общую добычу до 600 могильного золота.";
        case 14: return "Соберите 4 реликвии.";
        case 15: return "Получите 3 оружия, чтобы открыть Темные врата.";
        default: return "Свободно охотьтесь на открытых маршрутах.";
        }
    }

    switch (stageIndex)
    {
    case 0: return "Climb up and enter the portal to Level II.";
    case 1: return "Find the portal to Level III.";
    case 2: return "Own 1 relic.";
    case 3: return "Find the portal to Sunset Bastion and enter Level III.";
    case 4: return "Reach 180 total grave-gold.";
    case 5: return "Own 1 weapon.";
    case 6: return "Find the portal to Grave Hollow and enter Level IV.";
    case 7: return "Own 3 relics.";
    case 8: return "Reach 300 total grave-gold.";
    case 9: return "Find the portal to Ruins of the Elder City and enter Level V.";
    case 10: return "Reach 420 total grave-gold.";
    case 11: return "Own 2 weapons.";
    case 12: return "Find the portal to Aurora Nightpeaks and enter Level VI.";
    case 13: return "Reach 600 total grave-gold.";
    case 14: return "Own 4 relics.";
    case 15: return "Own 3 weapons to open the Dark Gate.";
    default: return "Hunt freely across the opened routes.";
    }

    if (Localization::isRussian())
    {
        switch (stageIndex)
        {
        case 0: return "Войдите в Обсидиановое гнездовье и вернитесь с 60 могильного золота всего.";
        case 1: return "Войдите в Багровый неф и доведите добычу до 180 могильного золота.";
        case 2: return "Войдите в Костяной реликварий и соберите 3 реликвии.";
        case 3: return "Войдите в Испытание углей и получите 1 выкованное оружие.";
        case 4: return "Войдите в Возвращающуюся вуаль и доведите добычу до 420 могильного золота.";
        case 5: return "Получите 2 выкованных оружия, чтобы открыть Темные врата.";
        default: return "Делайте открытые пути богаче.";
        }
    }

    switch (stageIndex)
    {
    case 0:
        return "Enter Skybound Spires and return with 60 total grave-gold.";
    case 1:
        return "Enter Sunset Bastion and reach 180 total grave-gold.";
    case 2:
        return "Enter Grave Hollow and own 3 relics.";
    case 3:
        return "Enter Ruins of the Elder City and own 1 crafted weapon.";
    case 4:
        return "Enter Aurora Nightpeaks and reach 420 total grave-gold.";
    case 5:
        return "Own 2 crafted weapons to open the Dark Gate.";
    default:
        return "Keep the opened routes rich.";
    }
}

std::string CampaignProgress::getStageProgressText(std::size_t stageIndex) const
{
    if (Localization::isRussian())
    {
        switch (stageIndex)
        {
        case 0: return boolProgressLabel(hasVisitedLevel("level2.json"), "Портал найден", "Портал наверху");
        case 1: return boolProgressLabel(hasVisitedLevel("level3.json"), "Портал найден", "Ищите портал");
        case 2: return std::to_string(std::min(relicsPurchased_, 1)) + " / 1 реликвия";
        case 3: return boolProgressLabel(hasVisitedLevel("level3.json"), "Неф посещен", "Портал в Неф не найден");
        case 4: return std::to_string(std::min(totalGoldCollected_, 180)) + " / 180 золота";
        case 5: return std::to_string(std::min(weaponsPurchased_, 1)) + " / 1 оружие";
        case 6: return boolProgressLabel(hasVisitedLevel("level4.json"), "Реликварий посещен", "Портал в Реликварий не найден");
        case 7: return std::to_string(std::min(relicsPurchased_, 3)) + " / 3 реликвии";
        case 8: return std::to_string(std::min(totalGoldCollected_, 300)) + " / 300 золота";
        case 9: return boolProgressLabel(hasVisitedLevel("level5.json"), "Испытание посещено", "Портал к Углям не найден");
        case 10: return std::to_string(std::min(totalGoldCollected_, 420)) + " / 420 золота";
        case 11: return std::to_string(std::min(weaponsPurchased_, 2)) + " / 2 оружия";
        case 12: return boolProgressLabel(hasVisitedLevel("level6.json"), "Вуаль посещена", "Портал в Вуаль не найден");
        case 13: return std::to_string(std::min(totalGoldCollected_, 600)) + " / 600 золота";
        case 14: return std::to_string(std::min(relicsPurchased_, 4)) + " / 4 реликвии";
        case 15: return std::to_string(std::min(weaponsPurchased_, 3)) + " / 3 оружия";
        default: return "Все пути открыты.";
        }
    }

    switch (stageIndex)
    {
    case 0: return boolProgressLabel(hasVisitedLevel("level2.json"), "Portal found", "Upper portal pending");
    case 1: return boolProgressLabel(hasVisitedLevel("level3.json"), "Portal found", "Search for the portal");
    case 2: return std::to_string(std::min(relicsPurchased_, 1)) + " / 1 relic";
    case 3: return boolProgressLabel(hasVisitedLevel("level3.json"), "Nave entered", "Nave portal not found");
    case 4: return std::to_string(std::min(totalGoldCollected_, 180)) + " / 180 gold";
    case 5: return std::to_string(std::min(weaponsPurchased_, 1)) + " / 1 weapon";
    case 6: return boolProgressLabel(hasVisitedLevel("level4.json"), "Reliquary entered", "Reliquary portal not found");
    case 7: return std::to_string(std::min(relicsPurchased_, 3)) + " / 3 relics";
    case 8: return std::to_string(std::min(totalGoldCollected_, 300)) + " / 300 gold";
    case 9: return boolProgressLabel(hasVisitedLevel("level5.json"), "Embers entered", "Embers portal not found");
    case 10: return std::to_string(std::min(totalGoldCollected_, 420)) + " / 420 gold";
    case 11: return std::to_string(std::min(weaponsPurchased_, 2)) + " / 2 weapons";
    case 12: return boolProgressLabel(hasVisitedLevel("level6.json"), "Veil entered", "Veil portal not found");
    case 13: return std::to_string(std::min(totalGoldCollected_, 600)) + " / 600 gold";
    case 14: return std::to_string(std::min(relicsPurchased_, 4)) + " / 4 relics";
    case 15: return std::to_string(std::min(weaponsPurchased_, 3)) + " / 3 weapons";
    default: return "Every route is open.";
    }

    if (Localization::isRussian())
    {
        switch (stageIndex)
        {
        case 0:
            return boolProgressLabel(hasVisitedLevel("level2.json"), "Гнездовье посещено", "Гнездовье ждет") +
                std::string("  |  ") + std::to_string(std::min(totalGoldCollected_, 60)) + " / 60 золота";
        case 1:
            return boolProgressLabel(hasVisitedLevel("level3.json"), "Неф посещен", "Неф ждет") +
                std::string("  |  ") + std::to_string(std::min(totalGoldCollected_, 180)) + " / 180 золота";
        case 2:
            return boolProgressLabel(hasVisitedLevel("level4.json"), "Реликварий посещен", "Реликварий ждет") +
                std::string("  |  ") + std::to_string(std::min(relicsPurchased_, 3)) + " / 3 реликвии";
        case 3:
            return boolProgressLabel(hasVisitedLevel("level5.json"), "Испытание посещено", "Испытание ждет") +
                std::string("  |  ") + std::to_string(std::min(weaponsPurchased_, 1)) + " / 1 оружие";
        case 4:
            return boolProgressLabel(hasVisitedLevel("level6.json"), "Вуаль посещена", "Вуаль ждет") +
                std::string("  |  ") + std::to_string(std::min(totalGoldCollected_, 420)) + " / 420 золота";
        case 5:
            return std::to_string(std::min(weaponsPurchased_, 2)) + " / 2 выкованных оружия";
        default:
            return "Все пути открыты.";
        }
    }

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
    if (Localization::isRussian())
    {
        switch (stageIndex)
        {
        case 0: return "Награда: откроются Небесные шпили.";
        case 1: return "Награда: откроется уровень III.";
        case 2: return "Награда: маршрут Гнездовья закреплен.";
        case 3: return "Награда: Метка Гнездовья дает +60 здоровья.";
        case 4: return "Награда: красная дорога становится устойчивее.";
        case 5: return "Награда: торговец признает вашу готовность.";
        case 6: return "Награда: Десятина Нефа дает +8 урона.";
        case 7: return "Награда: запас реликвий растет.";
        case 8: return "Награда: следующий путь становится безопаснее.";
        case 9: return "Награда: Запас Реликвария дает +140 энергии и +1 регенерации.";
        case 10: return "Награда: испытание признает вашу плату.";
        case 11: return "Награда: второй клинок укрепляет маршрут.";
        case 12: return "Награда: Угольная закалка дает +80 дальности.";
        case 13: return "Награда: Вуаль почти подчинена.";
        case 14: return "Награда: Рефлекс Вуали дает -12 мс КД выстрела.";
        case 15: return "Награда: Темные врата открыты, +90 здоровья и +10 урона.";
        default: return "Награда: свободная охота.";
        }
    }

    switch (stageIndex)
    {
    case 0: return "Reward: Skybound Spires opens.";
    case 1: return "Reward: Level III opens.";
    case 2: return "Reward: the Rookery route is anchored.";
    case 3: return "Reward: Rookery Mark grants +60 Vitality.";
    case 4: return "Reward: the red road steadies.";
    case 5: return "Reward: the trader accepts your readiness.";
    case 6: return "Reward: Nave Tithe grants +8 Damage.";
    case 7: return "Reward: your relic reserve grows.";
    case 8: return "Reward: the next route is safer.";
    case 9: return "Reward: Reliquary Reserve grants +140 Energy and +1 Energy gain.";
    case 10: return "Reward: the trial accepts your payment.";
    case 11: return "Reward: the second blade strengthens the route.";
    case 12: return "Reward: Ember Temper grants +80 Range.";
    case 13: return "Reward: the Veil is nearly bound.";
    case 14: return "Reward: Veil Reflex grants -12 ms Shot CD.";
    case 15: return "Reward: Dark Gate Opened grants +90 Vitality and +10 Damage.";
    default: return "Reward: free hunt.";
    }

    if (Localization::isRussian())
    {
        switch (stageIndex)
        {
        case 0: return "Награда: открывается уровень III, а Метка Гнездовья дает +60 здоровья.";
        case 1: return "Награда: открывается уровень IV, а Десятина Нефа дает +8 урона.";
        case 2: return "Награда: открывается уровень V, а Запас Реликвария дает +140 энергии и +1 к регенерации.";
        case 3: return "Награда: открывается уровень VI, а Угольная закалка дает +80 дальности.";
        case 4: return "Награда: Рефлекс Вуали дает -12 мс КД выстрела.";
        case 5: return "Награда: открытые Темные врата дают +90 здоровья и +10 урона.";
        default: return "Награда: бесконечная жатва.";
        }
    }

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
