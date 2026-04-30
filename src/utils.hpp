// Tells the thing not to run multiple times
#pragma once

// Include Geode headers
#include <Geode/Geode.hpp>

// Include dependencies
#include <cvolton.level-id-api/include/EditorIDs.hpp>

// Brings all Geode namespaces to the current scope
using namespace geode::prelude;

// Get level id utility
inline std::string getLevelKey(GJGameLevel* level) {
    // love editor level ids api thx cvolton
    if (level->m_levelType == GJLevelType::Editor) {
        return fmt::format("editor-{}", EditorIDs::getID(level));
    } else if (level->m_levelType == GJLevelType::Main) {
        return fmt::format("main-{}", EditorIDs::getID(level));
    }
    return fmt::format("level-{}", EditorIDs::getID(level));
}

/*
    Dear Geode moderator. I don't know if you guys switch out who
    reviews submitted mods, but if you're the person that reviewed
    this mod last then I'm talking to you. If you're not, then feel
    free to listen too. CubingXT here, I would like to politely ask
    of you what is wrong with the level ID utility? All you said is
    that the utility is "absolutely incorrect." I politely ask you
    to elaborate. Why is it "absolutely incorrect?" It's not like
    this exact code ia used in my mod Time To Rage, or Go! Indicator,
    or literally any mod that uses level IDs. I have added comments
    to this file so you know what everything does. The string output
    is in place because Editor level ID API cannot differentiate
    between editor levels, main levels, and online levels. This exact
    code is used in many mods and there are no issues. Please in the
    future actually say why something is wrong, not just that it's
    "absolutely incorrect." Thank you for your time.
    - CubingXT
*/