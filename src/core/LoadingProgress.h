#pragma once

#include <algorithm>
#include <mutex>
#include <string>

struct LoadingProgressSnapshot
{
    int completedOperations = 0;
    int totalOperations = 0;
    bool finished = false;
    bool failed = false;
    std::string stageLabel{};
    std::string errorMessage{};
};

class LoadingProgress
{
public:
    void setTotalOperations(int totalOperations)
    {
        std::scoped_lock lock(mutex_);
        state_.totalOperations = std::max(totalOperations, 0);
        state_.completedOperations = std::min(state_.completedOperations, state_.totalOperations);
    }

    void setStage(std::string stageLabel)
    {
        std::scoped_lock lock(mutex_);
        state_.stageLabel = std::move(stageLabel);
    }

    void advance()
    {
        std::scoped_lock lock(mutex_);
        ++state_.completedOperations;
        if (state_.totalOperations > 0)
        {
            state_.completedOperations = std::min(state_.completedOperations, state_.totalOperations);
        }
    }

    void finish(std::string stageLabel = {})
    {
        std::scoped_lock lock(mutex_);
        state_.finished = true;
        if (!stageLabel.empty())
        {
            state_.stageLabel = std::move(stageLabel);
        }
        if (state_.totalOperations > 0)
        {
            state_.completedOperations = state_.totalOperations;
        }
    }

    void fail(std::string errorMessage)
    {
        std::scoped_lock lock(mutex_);
        state_.failed = true;
        state_.finished = true;
        state_.errorMessage = std::move(errorMessage);
    }

    LoadingProgressSnapshot snapshot() const
    {
        std::scoped_lock lock(mutex_);
        return state_;
    }

private:
    mutable std::mutex mutex_{};
    LoadingProgressSnapshot state_{};
};
