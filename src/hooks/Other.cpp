#include "../Hacks.hpp"

#include <Geode/Geode.hpp>
#include <Geode/modify/CCTransitionFade.hpp>
#include <Geode/modify/GameManager.hpp>
#include <Geode/modify/GameStatsManager.hpp>

using namespace geode::prelude;
using hacks::Hack;

namespace {
    // The shop item that unlocks music in practice mode
    constexpr int PRACTICE_MUSIC_ITEM = 17;
}

class $modify(MenuGameManager, GameManager) {
    bool isIconUnlocked(int id, IconType type) {
        if (hacks::isOn(Hack::UnlockIcons)) {
            return true;
        }
        return GameManager::isIconUnlocked(id, type);
    }

    bool isColorUnlocked(int id, UnlockType type) {
        if (hacks::isOn(Hack::UnlockIcons)) {
            return true;
        }
        return GameManager::isColorUnlocked(id, type);
    }
};

class $modify(MenuGameStatsManager, GameStatsManager) {
    bool isItemUnlocked(UnlockType type, int id) {
        if (hacks::isOn(Hack::PracticeMusic) && type == UnlockType::GJItem && id == PRACTICE_MUSIC_ITEM) {
            return true;
        }
        return GameStatsManager::isItemUnlocked(type, id);
    }
};

class $modify(MenuTransitionFade, CCTransitionFade) {
    static CCTransitionFade* create(float duration, CCScene* scene) {
        return CCTransitionFade::create(hacks::isOn(Hack::NoTransition) ? 0.f : duration, scene);
    }
};
