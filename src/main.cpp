#include "utils.hpp"
#include <Geode/Geode.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <cmath>
#include <cvolton.level-id-api/include/EditorIDs.hpp>

using namespace geode::prelude;

bool g_isAttemptOver = false;
float g_currentStreak;

bool isTrackable(PlayLayer *pl) {
  if (!pl || !pl->m_level)
    return false;
  if (pl->m_isPracticeMode)
    return false;
  if (pl->m_isTestMode)
    return false;
  if (pl->m_isPlatformer)
    return false;
  return true;
}

std::string getBestStreakString(std::string levelID) {
  std::string bestKey = "best-streak-" + levelID;
  float val = Mod::get()->getSavedValue<float>(bestKey, 0.0f);
  return fmt::format("{:.0f}%", std::floor(val));
}

bool shouldShowStreak(std::string levelID) {
  std::string bestKey = "best-streak-" + levelID;
  log::debug("bestKey is {}", bestKey);
  float val = Mod::get()->getSavedValue<float>(bestKey, 0.0f);
  return val > 100.0f;
}

class $modify(MoreThan100PlayLayer, PlayLayer) {
  bool init(GJGameLevel *level, bool p1, bool p2) {
    if (!PlayLayer::init(level, p1, p2))
      return false;

    std::string streakKey = "current-streak-" + getLevelKey(m_level);
    g_currentStreak = Mod::get()->getSavedValue<float>(
        streakKey, 0.0f); // Possible get the value out of streakKey
    g_isAttemptOver = false;
    return true;
  }

  void resetLevel() {
    PlayLayer::resetLevel();
    g_isAttemptOver = false;
  }

  void levelComplete() {
    PlayLayer::levelComplete();
    if (!g_isAttemptOver && isTrackable(this)) {
      g_isAttemptOver = true;
      g_currentStreak =
          (g_currentStreak >= 100.0f) ? g_currentStreak + 100.0f : 100.0f;
      this->updateStats(g_currentStreak);
    }
  }

  void onQuit() {
    if (!g_isAttemptOver && isTrackable(this)) {
      float sessionPercent = this->getCurrentPercent();
      float totalAtQuit = (g_currentStreak >= 100.0f)
                              ? g_currentStreak + sessionPercent
                              : sessionPercent;
      this->updateStats(totalAtQuit);
      Mod::get()->setSavedValue("current-streak-" + getLevelKey(m_level), 0.0f);
      g_currentStreak = 0.0f;
      g_isAttemptOver = true;
    }
    PlayLayer::onQuit();
  }

  void updateStats(float total) {
    if (!m_level)
      return;
    std::string streakKey = "current-streak-" + getLevelKey(m_level);
    std::string bestKey = "best-streak-" + getLevelKey(m_level);

    Mod::get()->setSavedValue(streakKey, total);

    float currentBest = Mod::get()->getSavedValue<float>(bestKey, 0.0f);
    if (total > currentBest) {
      Mod::get()->setSavedValue(bestKey, total);
    }
  }
};

class $modify(MoreThan100LevelInfoLayer, LevelInfoLayer) {
  bool init(GJGameLevel *level, bool challenge) {
    if (!LevelInfoLayer::init(level, challenge))
      return false;

    if (level->isPlatformer())
      return true;

    std::string key = getLevelKey(level);
    if (shouldShowStreak(key)) {
      std::string bestStreak = getBestStreakString(key);

      if (auto label = typeinfo_cast<CCLabelBMFont *>(
              this->getChildByID("normal-mode-percentage"))) {
        label->setString(bestStreak.c_str());
      }
    }
    return true;
  }
};

class $modify(MoreThan100PauseLayer, PauseLayer) {
  void updateStreakLabel(float) {
    auto pl = PlayLayer::get();
    if (!isTrackable(pl))
      return;

    std::string key = getLevelKey(pl->m_level);
    if (!shouldShowStreak(key))
      return;

    std::string bestStreak = getBestStreakString(key);

    if (auto label = typeinfo_cast<CCLabelBMFont *>(
            this->getChildByID("normal-progress-label"))) {
      label->setString(bestStreak.c_str());
    }
  }
/*
  Seriously Absolllute how could you hook literally half the game and
  not even call the original functions. Now I have to do this fcking scheduling
  sht because you can't type one line of code. Like it's not that hard bud. It's not
  that hard. At least try to make us modders not have to work around you.
*/
  void customSetup() {
    PauseLayer::customSetup();
    this->scheduleOnce(
        schedule_selector(MoreThan100PauseLayer::updateStreakLabel), 0.0f);
  }
};

class $modify(PlayerObject) {
  void playerDestroyed(bool p0) {
    if (!g_isAttemptOver) {
      if (auto pl = PlayLayer::get()) {
        if (isTrackable(pl)) {
          float sessionPercent = pl->getCurrentPercent();
          static_cast<MoreThan100PlayLayer *>(pl)->updateStats(
              (g_currentStreak >= 100.0f) ? g_currentStreak + sessionPercent
                                          : sessionPercent);
          Mod::get()->setSavedValue(
              "current-streak-" + getLevelKey(pl->m_level), 0.0f);
          g_currentStreak = 0.0f;
        }
        g_isAttemptOver = true;
      }
    }
    PlayerObject::playerDestroyed(p0);
  }
};
