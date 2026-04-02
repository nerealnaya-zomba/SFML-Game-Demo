#include "GameData.h"

#include <algorithm>
#include <stdexcept>

namespace
{
constexpr const char* kLaunchSettingsPath = "data/launchSettings.json";
constexpr int kDefaultLoadingOperationsCount = 61;
constexpr bool kDefaultVsyncEnabled = false;
constexpr int kDefaultMenuParticleCount = 200;

nlohmann::json loadLaunchSettingsDocument()
{
    std::ifstream input(kLaunchSettingsPath);
    if (!input.is_open())
    {
        return nlohmann::json::object();
    }

    try
    {
        return nlohmann::json::parse(input);
    }
    catch (const nlohmann::json::exception&)
    {
        return nlohmann::json::object();
    }
}
}

GameData::GameData(sf::Font* font, std::shared_ptr<LoadingProgress> loadingProgress)
    : gameFont(font)
    , loadingProgress_m(std::move(loadingProgress))
{
    loadData();
    if (loadingProgress_m)
    {
        loadingProgress_m->setTotalOperations(allOperations_count_m);
        loadingProgress_m->setStage("Reading the sealed archive");
    }
    loadEnemySettings();

    const auto loadDirectTextures = [&](const std::string& stageLabel,
                                        std::vector<sf::Texture>& textures,
                                        const std::vector<std::string>& paths,
                                        bool generateMipmaps = false,
                                        bool smooth = false) {
        beginLoadingStep(stageLabel);
        if (!initSatiroTextures(textures, paths))
        {
            failLoadingStep(stageLabel);
        }
        if (generateMipmaps)
        {
            generateMipmapTextures(textures);
        }
        if (smooth)
        {
            smoothTextures(textures);
        }
        finishLoadingStep();
    };

    const auto loadAnimatedTextures = [&](const std::string& stageLabel,
                                          std::vector<sf::Texture>& textures,
                                          const std::string& path,
                                          texturesIterHelper& helper,
                                          int pauseTillSwitch,
                                          bool generateMipmaps = false,
                                          bool smooth = false) {
        beginLoadingStep(stageLabel);
        if (!loadTexture(textures, path, helper, pauseTillSwitch))
        {
            failLoadingStep(stageLabel);
        }
        if (generateMipmaps)
        {
            generateMipmapTextures(textures);
        }
        if (smooth)
        {
            smoothTextures(textures);
        }
        finishLoadingStep();
    };

    const auto loadStaticTextures = [&](const std::string& stageLabel,
                                        std::map<std::string, sf::Texture>& textures,
                                        const std::string& path,
                                        bool clearOnReuse,
                                        bool generateMipmaps = false,
                                        bool smooth = false) {
        beginLoadingStep(stageLabel);
        if (!loadTexture(textures, path, clearOnReuse))
        {
            failLoadingStep(stageLabel);
        }
        if (generateMipmaps)
        {
            generateMipmapTextures(textures);
        }
        if (smooth)
        {
            smoothTextures(textures);
        }
        finishLoadingStep();
    };

    loadDirectTextures("Tracing idle stances", idleTextures, idleTexturesPaths);
    loadDirectTextures("Forging running stances", runningTextures, runningTexturesPaths);
    loadDirectTextures("Reading falling stances", fallingTextures, fallingTexturesPaths);

    loadAnimatedTextures("Tempering ember bolts", bulletTextures, bulletTexturesPath_, satiro_bullet_helper, 7, true);
    loadAnimatedTextures("Binding the death rite", satiro_dieTextures, satiro_diePath_, satiro_die_helper, 20, true);
    loadAnimatedTextures("Sharpening dash afterimages", satiro_dashTextures, satiro_dashPath_, satiro_dash_helper, 9, true);
    loadAnimatedTextures("Carving pain echoes", satiro_hurtTextures, satiro_hurtPath_, satiro_hurt_helper, 15, true);
    loadAnimatedTextures("Cutting the slide path", satiro_slideTextures, satiro_slidePath_, satiro_slide_helper, 9, true);
    loadAnimatedTextures("Drawing leap arcs", satiro_jumpTextures, satiro_jumpPath_, satiro_jump_helper, 9, true);
    loadAnimatedTextures("Sealing landing stances", satiro_landingTextures, satiro_landingPath_, satiro_landing_helper, 9, true);

    loadAnimatedTextures("Summoning pale skeleton idles", skeletonWhite_idleTextures_, skeletonWhite_idlePath_, skeletonWhite_idle_helper, 9, true);
    loadAnimatedTextures("Summoning pale skeleton march", skeletonWhite_walkTextures, skeletonWhite_walkPath_, skeletonWhite_walk_helper, 9, true);
    loadAnimatedTextures("Summoning pale skeleton wounds", skeletonWhite_hurtTextures, skeletonWhite_hurtPath_, skeletonWhite_hurt_helper, 9, true);
    loadAnimatedTextures("Summoning pale skeleton death", skeletonWhite_dieTextures, skeletonWhite_diePath_, skeletonWhite_die_helper, 9, true);
    loadAnimatedTextures("Summoning pale skeleton strike I", skeletonWhite_attack1Textures, skeletonWhite_attack1Path_, skeletonWhite_attack1_helper, 6, true);
    loadAnimatedTextures("Summoning pale skeleton strike II", skeletonWhite_attack2Textures, skeletonWhite_attack2Path_, skeletonWhite_attack2_helper, 6, true);

    loadAnimatedTextures("Calling yellow skeleton idles", skeletonYellow_idleTextures, skeletonYellow_idlePath_, skeletonYellow_idle_helper, 9, true);
    loadAnimatedTextures("Calling yellow skeleton march", skeletonYellow_walkTextures, skeletonYellow_walkPath_, skeletonYellow_walk_helper, 6, true);
    loadAnimatedTextures("Calling yellow skeleton wounds", skeletonYellow_hurtTextures, skeletonYellow_hurtPath_, skeletonYellow_hurt_helper, 9, true);
    loadAnimatedTextures("Calling yellow skeleton death", skeletonYellow_dieTextures, skeletonYellow_diePath_, skeletonYellow_die_helper, 9, true);
    loadAnimatedTextures("Calling yellow skeleton strike I", skeletonYellow_attack1Textures, skeletonYellow_attack1Path_, skeletonYellow_attack1_helper, 4, true);
    loadAnimatedTextures("Calling yellow skeleton strike II", skeletonYellow_attack2Textures, skeletonYellow_attack2Path_, skeletonYellow_attack2_helper, 4, true);

    loadAnimatedTextures("Unfurling the wraith bat wings", wraithBat_flyTextures, wraithBat_flyPath_, wraithBat_fly_helper, 5, true);
    loadAnimatedTextures("Sharpening the wraith bat dive", wraithBat_attackTextures, wraithBat_attackPath_, wraithBat_attack_helper, 4, true);
    loadAnimatedTextures("Breathing life into void slime", voidSlime_idleTextures, voidSlime_idlePath_, voidSlime_idle_helper, 12, true);
    loadAnimatedTextures("Stretching void slime leaps", voidSlime_leapTextures, voidSlime_leapPath_, voidSlime_leap_helper, 5, true);
    loadAnimatedTextures("Marching the dread scorpion", dreadScorpion_walkTextures, dreadScorpion_walkPath_, dreadScorpion_walk_helper, 4, true);
    loadAnimatedTextures("Forging the dread scorpion stab", dreadScorpion_stabTextures, dreadScorpion_stabPath_, dreadScorpion_stab_helper, 4, true);

    loadAnimatedTextures("Lighting the trader's lantern", trader_idleTextures, traderPath, trader_idle_helper, 25, true);

    loadAnimatedTextures("Growing grave moss I", plant1Textures, plant1Path, plant1, 8, true, true);
    loadAnimatedTextures("Growing grave moss II", plant2Textures, plant2Path, plant2, 8, true, true);
    loadAnimatedTextures("Growing grave moss III", plant3Textures, plant3Path, plant3, 8, true, true);
    loadAnimatedTextures("Growing grave moss IV", plant4Textures, plant4Path, plant4, 6, true, true);
    loadAnimatedTextures("Growing grave moss V", plant5Textures, plant5Path, plant5, 9, true, true);
    loadAnimatedTextures("Growing grave moss VI", plant6Textures, plant6Path, plant6, 9, true, true);
    loadAnimatedTextures("Growing grave moss VII", plant7Textures, plant7Path, plant7, 9, true, true);
    loadAnimatedTextures("Waking the sleeping cat", cat1Textures, cat1Path, catHelper, 72, true);
    loadAnimatedTextures("Awakening the jump bloom", jumpPlantTextures, jumpPlantPath, jumpPlant, 5, true, true);
    loadAnimatedTextures("Unfurling the second jump bloom", jumpPlant2Textures, jumpPlant2Path, jumpPlant2, 5, true, true);
    loadAnimatedTextures("Letting the grave reeds sway", plantWind1Textures, plantWind1Path, plantWind1, 4, true, true);
    loadAnimatedTextures("Opening the moonlit flower", blueFlower1Textures, blueFlower1Path, blueFlower1, 2, true, true);
    loadAnimatedTextures("Breathing through the closed flower", blueFlower2Textures, blueFlower2Path, blueFlower2, 2, true, true);
    loadAnimatedTextures("Stirring the poison bloom", plant8PoisonTextures, plant8PoisonPath, plant8Poison, 4, true, true);
    loadAnimatedTextures("Opening the green portal", portalGreenTextures, portalGreenPath, portalGreen, 5, true, true);

    constexpr int portalBluePauseTillSwitch = 2;
    loadAnimatedTextures("Charging blue portal I", portalBlue1Textures, PortalBlue1Path, portalBlue1Helper, portalBluePauseTillSwitch, true, true);
    loadAnimatedTextures("Charging blue portal II", portalBlue2Textures, PortalBlue2Path, portalBlue2Helper, portalBluePauseTillSwitch, true, true);
    loadAnimatedTextures("Charging blue portal III", portalBlue3Textures, PortalBlue3Path, portalBlue3Helper, portalBluePauseTillSwitch, true, true);
    loadAnimatedTextures("Charging blue portal IV", portalBlue4Textures, PortalBlue4Path, portalBlue4Helper, portalBluePauseTillSwitch, true, true);
    loadAnimatedTextures("Charging blue portal V", portalBlue5Textures, PortalBlue5Path, portalBlue5Helper, portalBluePauseTillSwitch, true, true);
    loadAnimatedTextures("Charging blue portal VI", portalBlue6Textures, PortalBlue6Path, portalBlue6Helper, portalBluePauseTillSwitch, true, true);
    loadAnimatedTextures("Charging blue portal VII", portalBlue7Textures, PortalBlue7Path, portalBlue7Helper, portalBluePauseTillSwitch, true, true);
    loadAnimatedTextures("Charging blue portal VIII", portalBlue8Textures, PortalBlue8Path, portalBlue8Helper, portalBluePauseTillSwitch, true, true);

    loadStaticTextures("Dressing mossy backdrops", allStaticTextures, MossyBackgroundDecorationsPath, false);
    loadStaticTextures("Sharpening mossy hazards", allStaticTextures, MossyDecorationsHazardsPath, false);
    loadStaticTextures("Hanging creeping roots", allStaticTextures, MossyHangingPlantsPath, false);
    loadStaticTextures("Raising distant hills", allStaticTextures, MossyHillsPath, false);
    loadStaticTextures("Stacking mossy stones", allStaticTextures, MossyTileSetPath, false, true, true);

    loadStaticTextures("Forging ground tiles", TileSetGreenTextures, groundTileSetGreenPath, true, true, true);
    loadStaticTextures("Layering haunted skies", backgroundTextures, backgroundPath, true, true, true);
    loadStaticTextures("Scattering relic items", itemsTextures, itemsPath, true);
    loadStaticTextures("Sealing interface glyphs", guiTextures, guiPath, true);

    if (succesedOperationsCount_m != allOperations_count_m)
    {
        saveOperationsData();
    }

    if (loadingProgress_m)
    {
        loadingProgress_m->finish("The gate yields");
    }
}

GameData::~GameData()
{

}

bool GameData::initSatiroTextures(std::vector<sf::Texture> &textures, std::vector<std::string> paths)
{
    textures.clear();
    textures.reserve(paths.size());
    for (size_t i = 0; i < paths.size(); i++)
    {
        textures.emplace_back();
        if(!textures.back().loadFromFile(paths[i]))
        {
            textures.pop_back();
            std::cout << "Error loading texture: " << paths[i] << std::endl;
            return false;
        }
        debugLog("Texture loaded: ", paths[i]);
    }
    return true;
}

bool GameData::loadTexture(std::vector<sf::Texture> &textures, std::string path, 
                           texturesIterHelper& helper, int pauseTillSwitch = -1)
{
    TextureSequenceInfo seq = analyzeTextureSequence(path);
    
    if (seq.count == 0) {
        std::cerr << "ERROR: No textures found for path: " << path << std::endl;
        exit(1);
        return false;
    }
    
    // Отладочный вывод
    debugLog("Loading textures: ", path,
             ", start=", seq.startCounter,
             ", count=", seq.count,
             ", digits=", seq.maxDigits);
    
    helper.countOfTextures = seq.count - 1;
    helper.iterationsTillSwitch = pauseTillSwitch;

    // ОЧЕНЬ ВАЖНО: передаем количество текстур, а не последний номер
    return initTextures(textures, path, seq.count, seq.maxDigits, seq.startCounter);
}
bool GameData::loadTexture(std::map<std::string,sf::Texture> &textures, std::string path, bool clearOnReuse = true)
{
    TextureSequenceInfo seq = analyzeTextureSequence(path);
    
    if (seq.count == 0) {
		std::string errorMsg = "ERROR: No textures found for path: " + path;
		throw std::runtime_error(errorMsg);
        return false;
    }
    
    // Отладочный вывод
    debugLog("Loading textures: ", path,
             ", start=", seq.startCounter,
             ", count=", seq.count,
             ", digits=", seq.maxDigits);
    
    // ОЧЕНЬ ВАЖНО: передаем количество текстур, а не последний номер
    if(clearOnReuse) return initTextures(textures, path, seq.count, seq.maxDigits, seq.startCounter);
    else return initTextures(textures, path, seq.count, seq.maxDigits, seq.startCounter, false);
}

void GameData::generateMipmapTextures(std::map<std::string, sf::Texture> &texturesArray)
{
    for (auto &i : texturesArray)
    {
        if(i.second.generateMipmap())
        {
            debugLog("Mipmap generated");
        }
        else
        {
            debugLog("Error while generating mipmap");
        }
    }
}

void GameData::generateMipmapTextures(std::vector<sf::Texture> &texturesArray)
{
    for (auto &i : texturesArray)
    {
        if(i.generateMipmap())
        {
            debugLog("Mipmap generated");
        }
        else
        {
            debugLog("Error while generating mipmap");
        }
    }
}

void GameData::smoothTextures(std::vector<sf::Texture> &texturesArray)
{
    for (auto &i : texturesArray)
    {
        i.setSmooth(true);
    }
}

void GameData::smoothTextures(std::map<std::string, sf::Texture> &texturesArray)
{
    for (auto &i : texturesArray)
    {
        i.second.setSmooth(true);
    }
}

void GameData::beginLoadingStep(const std::string& stageLabel)
{
    if (loadingProgress_m)
    {
        loadingProgress_m->setStage(stageLabel);
    }
}

void GameData::finishLoadingStep()
{
    ++succesedOperationsCount_m;
    if (loadingProgress_m)
    {
        loadingProgress_m->advance();
    }
}

[[noreturn]] void GameData::failLoadingStep(const std::string& stageLabel)
{
    const std::string errorMessage = "Failed while " + stageLabel;
    if (loadingProgress_m)
    {
        loadingProgress_m->fail(errorMessage);
    }
    throw std::runtime_error(errorMessage);
}

//
void GameData::saveOperationsData()
{
    allOperations_count_m = succesedOperationsCount_m;
    saveLaunchSettings();
}

void GameData::saveLaunchSettings() const
{
    nlohmann::json document = loadLaunchSettingsDocument();
    document["loadingGameAssets_operationsCount"] = allOperations_count_m;
    document["video"]["vsyncEnabled"] = launchPreferences_.vsyncEnabled;
    document["menu"]["particleCount"] = launchPreferences_.menuParticleCount;

    std::ofstream output(kLaunchSettingsPath);
    output << document.dump(4);
}

void GameData::loadData()
{
    const nlohmann::json document = loadLaunchSettingsDocument();
    allOperations_count_m = document.value(
        "loadingGameAssets_operationsCount",
        kDefaultLoadingOperationsCount
    );

    const nlohmann::json videoSettings = document.value("video", nlohmann::json::object());
    const nlohmann::json menuSettings = document.value("menu", nlohmann::json::object());
    launchPreferences_.vsyncEnabled = videoSettings.value(
        "vsyncEnabled",
        document.value("vsyncEnabled", kDefaultVsyncEnabled)
    );
    launchPreferences_.menuParticleCount = clampMenuParticleCount(
        menuSettings.value(
            "particleCount",
            document.value("menuParticleCount", kDefaultMenuParticleCount)
        )
    );
}

void GameData::loadEnemySettings()
{
    std::fstream f("data/enemySettings.json");
    enemySettings_m = nlohmann::json::parse(f);
}

const nlohmann::json& GameData::getEnemySettings() const
{
    return enemySettings_m;
}

bool GameData::isVsyncEnabled() const
{
    return launchPreferences_.vsyncEnabled;
}

int GameData::getMenuParticleCount() const
{
    return launchPreferences_.menuParticleCount;
}

GameData::LaunchPreferences GameData::getLaunchPreferences() const
{
    return launchPreferences_;
}

void GameData::setVsyncEnabled(const bool enabled)
{
    if (launchPreferences_.vsyncEnabled == enabled)
    {
        return;
    }

    launchPreferences_.vsyncEnabled = enabled;
    saveLaunchSettings();
}

void GameData::setMenuParticleCount(const int count)
{
    const int clampedCount = clampMenuParticleCount(count);
    if (launchPreferences_.menuParticleCount == clampedCount)
    {
        return;
    }

    launchPreferences_.menuParticleCount = clampedCount;
    saveLaunchSettings();
}

int GameData::clampMenuParticleCount(const int count)
{
    return std::clamp(count, 80, 260);
}
