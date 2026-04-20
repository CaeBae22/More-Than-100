#pragma once

#include <Geode/Geode.hpp>
#include <cvolton.level-id-api/include/EditorIDs.hpp>

using namespace geode::prelude;

inline std::string getLevelKey(GJGameLevel *level) {
  if (level->m_levelType == GJLevelType::Editor) {
    return fmt::format("editor-{}", EditorIDs::getID(level));
  } else if (level->m_levelType == GJLevelType::Main) {
    return fmt::format("main-{}", EditorIDs::getID(level));
  }
  return fmt::format("level-{}", EditorIDs::getID(level));
}
