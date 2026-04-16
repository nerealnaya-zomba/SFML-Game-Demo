#include "LevelRegistry.h"

#include <algorithm>
#include <fstream>
#include <nlohmann/json.hpp>

namespace
{
std::string readStringValue(
    const nlohmann::json& object,
    const char* key,
    const std::string& fallback
)
{
    if (!object.contains(key) || !object[key].is_string())
    {
        return fallback;
    }

    const std::string value = object[key].get<std::string>();
    return value.empty() ? fallback : value;
}
}

LevelRegistry::LevelRegistry(const std::filesystem::path& levelsFolder)
{
    scan(levelsFolder);
}

bool LevelRegistry::scan(const std::filesystem::path& levelsFolder)
{
    levels_.clear();
    levelIdToIndex_.clear();
    fileNameToIndex_.clear();

    if (!std::filesystem::exists(levelsFolder) || !std::filesystem::is_directory(levelsFolder))
    {
        return false;
    }

    for (const auto& entry : std::filesystem::directory_iterator(levelsFolder))
    {
        if (!entry.is_regular_file() || entry.path().extension() != ".json")
        {
            continue;
        }

        std::optional<LevelDescriptor> descriptor = readLevelDescriptor(entry.path());
        if (!descriptor.has_value())
        {
            continue;
        }

        if (std::any_of(levels_.begin(), levels_.end(), [&](const LevelDescriptor& existing) {
            return existing.id == descriptor->id;
        }))
        {
            descriptor->id = descriptor->fileName;
        }

        levels_.push_back(std::move(*descriptor));
    }

    std::sort(levels_.begin(), levels_.end(), [](const LevelDescriptor& lhs, const LevelDescriptor& rhs) {
        if (lhs.menuOrder != rhs.menuOrder)
        {
            return lhs.menuOrder < rhs.menuOrder;
        }

        if (lhs.title != rhs.title)
        {
            return lhs.title < rhs.title;
        }

        return lhs.fileName < rhs.fileName;
    });

    rebuildIndexes();
    return true;
}

const std::vector<LevelDescriptor>& LevelRegistry::getLevels() const
{
    return levels_;
}

std::optional<LevelDescriptor> LevelRegistry::findById(const std::string& levelId) const
{
    const auto it = levelIdToIndex_.find(levelId);
    if (it == levelIdToIndex_.end())
    {
        return std::nullopt;
    }

    return levels_.at(it->second);
}

std::optional<LevelDescriptor> LevelRegistry::findByFileName(const std::string& fileName) const
{
    const auto it = fileNameToIndex_.find(fileName);
    if (it == fileNameToIndex_.end())
    {
        return std::nullopt;
    }

    return levels_.at(it->second);
}

std::optional<LevelDescriptor> LevelRegistry::resolve(const std::string& levelIdentifier) const
{
    if (std::optional<LevelDescriptor> byId = findById(levelIdentifier); byId.has_value())
    {
        return byId;
    }

    return findByFileName(levelIdentifier);
}

std::string LevelRegistry::getDisplayName(const std::string& levelIdentifier) const
{
    if (std::optional<LevelDescriptor> descriptor = resolve(levelIdentifier); descriptor.has_value())
    {
        return descriptor->title;
    }

    return levelIdentifier;
}

std::optional<LevelDescriptor> LevelRegistry::readLevelDescriptor(const std::filesystem::path& filePath)
{
    std::ifstream input(filePath);
    if (!input.is_open())
    {
        return std::nullopt;
    }

    try
    {
        const nlohmann::json document = nlohmann::json::parse(input);
        const nlohmann::json presets = document.value("Presets", nlohmann::json::object());
        const std::string fileName = filePath.filename().string();
        const std::string levelId = readStringValue(presets, "LevelId", fileName);
        const std::string title = readStringValue(presets, "Title", levelId);

        return LevelDescriptor{
            levelId,
            title,
            fileName,
            filePath,
            presets.value("MenuOrder", 1000),
            presets.value("isAvaiable", true)
        };
    }
    catch (const nlohmann::json::exception&)
    {
        return std::nullopt;
    }
}

void LevelRegistry::rebuildIndexes()
{
    levelIdToIndex_.clear();
    fileNameToIndex_.clear();

    for (std::size_t index = 0; index < levels_.size(); ++index)
    {
        levelIdToIndex_[levels_[index].id] = index;
        fileNameToIndex_[levels_[index].fileName] = index;
    }
}
