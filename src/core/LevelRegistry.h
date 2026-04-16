#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

struct LevelDescriptor
{
    std::string id;
    std::string title;
    std::string fileName;
    std::filesystem::path filePath;
    int menuOrder = 0;
    bool isAvailable = true;
};

class LevelRegistry
{
public:
    LevelRegistry() = default;
    explicit LevelRegistry(const std::filesystem::path& levelsFolder);

    bool scan(const std::filesystem::path& levelsFolder);

    const std::vector<LevelDescriptor>& getLevels() const;
    std::optional<LevelDescriptor> findById(const std::string& levelId) const;
    std::optional<LevelDescriptor> findByFileName(const std::string& fileName) const;
    std::optional<LevelDescriptor> resolve(const std::string& levelIdentifier) const;
    std::string getDisplayName(const std::string& levelIdentifier) const;

    static std::optional<LevelDescriptor> readLevelDescriptor(const std::filesystem::path& filePath);

private:
    std::vector<LevelDescriptor> levels_{};
    std::unordered_map<std::string, std::size_t> levelIdToIndex_{};
    std::unordered_map<std::string, std::size_t> fileNameToIndex_{};

    void rebuildIndexes();
};
