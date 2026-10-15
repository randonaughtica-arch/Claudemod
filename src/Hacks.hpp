#pragma once

#include <cstddef>
#include <span>

// Every toggle in the menu is a bool setting in mod.json, so it also shows up
// in Geode's settings page and is saved by Geode. This header is the single
// place hooks read hack state from.
namespace hacks {
    enum class Tab { Level, Editor, Other };

    enum class Hack : std::size_t {
        // Level
        Noclip,
        SafeMode,
        AutoSafeMode,
        ShowHitboxes,
        AccuratePercentage,
        NoDeathEffect,
        HideAttempts,
        HidePauseButton,
        NoMirror,
        InfoOverlay,
        Speedhack,
        // Editor
        VerifyHack,
        NoCopyMark,
        CopyHack,
        FreeScroll,
        ZoomBypass,
        // Other
        UnlockIcons,
        PracticeMusic,
        NoTransition,
        MainMenuButton,

        Count,
    };

    struct HackInfo {
        Hack hack;
        Tab tab;
        // Setting key in mod.json; the setting's name and description are
        // shown in the menu too
        char const* key;
    };

    inline constexpr float MIN_SPEED = 0.1f;
    inline constexpr float MAX_SPEED = 10.f;

    std::span<HackInfo const> all();
    HackInfo const& info(Hack hack);

    bool isOn(Hack hack);
    void setOn(Hack hack, bool on);

    float speed();
    void setSpeed(float speed);
    // 1 unless Speedhack is on
    float activeSpeed();

    // Something is on that would make a completion illegitimate
    bool cheatsActive();
    bool safeModeActive();

    // Reads the saved settings and keeps them in sync afterwards
    void loadSettings();
}
