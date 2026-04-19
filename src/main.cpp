#include <Geode/Geode.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include <fmt/format.h>
#include <cmath>

using namespace geode::prelude;

bool g_isAttemptOver = false;
float g_currentStreak;

// Helper to get the best streak as a string
std::string getBestStreakString(int levelID) {
    std::string bestKey = "best-streak-" + std::to_string(levelID);
    float val = Mod::get()->getSavedValue<float>(bestKey, 0.0f);
    return fmt::format("{:.0f}%", std::floor(val));
}

// Helper to check if we should show the streak
bool shouldShowStreak(int levelID) {
    std::string bestKey = "best-streak-" + std::to_string(levelID);
    float val = Mod::get()->getSavedValue<float>(bestKey, 0.0f);
    return val > 100.0f; // Only show if greater than 100
}

class $modify(MyPlayLayer, PlayLayer) {
    bool init(GJGameLevel* level, bool p1, bool p2) {
        if (!PlayLayer::init(level, p1, p2)) return false;

        std::string streakKey = "current-streak-" + std::to_string(level->m_levelID.value());
        g_currentStreak = Mod::get()->getSavedValue<float>(streakKey, 0.0f);
        g_isAttemptOver = false;
        
        return true;
    }

    void resetLevel() {
        PlayLayer::resetLevel();
        g_isAttemptOver = false; 
    }

    void levelComplete() {
        PlayLayer::levelComplete();
        if (!g_isAttemptOver) {
            g_isAttemptOver = true; 
            g_currentStreak = (g_currentStreak >= 100.0f) ? g_currentStreak + 100.0f : 100.0f;
            this->updateStats(g_currentStreak);
        }
    }

    void onQuit() {
        if (!g_isAttemptOver) {
            float sessionPercent = this->getCurrentPercent();
            float totalAtQuit = (g_currentStreak >= 100.0f) ? g_currentStreak + sessionPercent : sessionPercent;
            this->updateStats(totalAtQuit);

            int id = m_level->m_levelID.value();
            Mod::get()->setSavedValue("current-streak-" + std::to_string(id), 0.0f);
            g_currentStreak = 0.0f;
            g_isAttemptOver = true;
        }
        PlayLayer::onQuit();
    }

    void updateStats(float total) {
        if (!m_level) return;
        int id = m_level->m_levelID.value();
        std::string streakKey = "current-streak-" + std::to_string(id);
        std::string bestKey = "best-streak-" + std::to_string(id);

        Mod::get()->setSavedValue(streakKey, total);

        float currentBest = Mod::get()->getSavedValue<float>(bestKey, 0.0f);
        if (total > currentBest) {
            Mod::get()->setSavedValue(bestKey, total);
        }
    }
};

class $modify(MyLevelInfoLayer, LevelInfoLayer) {
    bool init(GJGameLevel* level, bool challenge) {
        if (!LevelInfoLayer::init(level, challenge)) return false;

        if (shouldShowStreak(level->m_levelID.value())) {
            std::string normalBest = std::to_string(level->m_normalPercent.value()) + "%";
            std::string bestStreak = getBestStreakString(level->m_levelID.value());

            auto children = this->getChildren();
            for (int i = 0; i < children->count(); ++i) {
                if (auto label = typeinfo_cast<CCLabelBMFont*>(children->objectAtIndex(i))) {
                    if (std::string(label->getString()) == normalBest) {
                        label->setString(bestStreak.c_str());
                    }
                }
            }
        }
        return true;
    }
};

class $modify(MyPauseLayer, PauseLayer) {
    void customSetup() {
        PauseLayer::customSetup();
        auto pl = PlayLayer::get();
        if (pl && shouldShowStreak(pl->m_level->m_levelID.value())) {
            std::string normalBest = std::to_string(pl->m_level->m_normalPercent.value()) + "%";
            std::string bestStreak = getBestStreakString(pl->m_level->m_levelID.value());

            auto children = this->getChildren();
            for (int i = 0; i < children->count(); ++i) {
                if (auto label = typeinfo_cast<CCLabelBMFont*>(children->objectAtIndex(i))) {
                    if (label->getString() == normalBest) {
                        label->setString(bestStreak.c_str());
                    }
                }
            }
        }
    }
};

class $modify(PlayerObject) {
    void playerDestroyed(bool p0) {
        if (!g_isAttemptOver) {
            if (auto playLayer = PlayLayer::get()) {
                float sessionPercent = playLayer->getCurrentPercent();
                float totalAtDeath = (g_currentStreak >= 100.0f) ? g_currentStreak + sessionPercent : sessionPercent;
                static_cast<MyPlayLayer*>(playLayer)->updateStats(totalAtDeath);
                
                int id = playLayer->m_level->m_levelID.value();
                Mod::get()->setSavedValue("current-streak-" + std::to_string(id), 0.0f);
                g_currentStreak = 0.0f;
                g_isAttemptOver = true;
            }
        }
        PlayerObject::playerDestroyed(p0);
    }
};
