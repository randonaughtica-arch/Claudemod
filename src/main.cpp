#include "Hacks.hpp"
#include "MenuPopup.hpp"

#include <Geode/Geode.hpp>

using namespace geode::prelude;

// Hooks live in src/hooks, grouped by the menu tab they belong to
$on_mod(Loaded) {
    hacks::loadSettings();

    listenForKeybindSettingPresses("open-menu", [](Keybind const&, bool down, bool repeat, double) {
        if (down && !repeat) {
            MenuPopup::toggle();
        }
    });
}
