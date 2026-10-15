#include "Hacks.hpp"

#include <Geode/Geode.hpp>
#include <Geode/binding/PlayLayer.hpp>

#include <algorithm>
#include <array>

using namespace geode::prelude;

namespace hacks {
    namespace {
        constexpr std::array<HackInfo, static_cast<std::size_t>(Hack::Count)> HACKS {{
            { Hack::Noclip,             Tab::Level,  "noclip" },
            { Hack::SafeMode,           Tab::Level,  "safe-mode" },
            { Hack::AutoSafeMode,       Tab::Level,  "auto-safe-mode" },
            { Hack::ShowHitboxes,       Tab::Level,  "show-hitboxes" },
            { Hack::AccuratePercentage, Tab::Level,  "accurate-percentage" },
            { Hack::NoDeathEffect,      Tab::Level,  "no-death-effect" },
            { Hack::HideAttempts,       Tab::Level,  "hide-attempts" },
            { Hack::HidePauseButton,    Tab::Level,  "hide-pause-button" },
            { Hack::NoMirror,           Tab::Level,  "no-mirror" },
            { Hack::InfoOverlay,        Tab::Level,  "info-overlay" },
            { Hack::Speedhack,          Tab::Level,  "speedhack" },
            { Hack::VerifyHack,         Tab::Editor, "verify-hack" },
            { Hack::NoCopyMark,         Tab::Editor, "no-copy-mark" },
            { Hack::CopyHack,           Tab::Editor, "copy-hack" },
            { Hack::FreeScroll,         Tab::Editor, "free-scroll" },
            { Hack::ZoomBypass,         Tab::Editor, "zoom-bypass" },
            { Hack::UnlockIcons,        Tab::Other,  "unlock-icons" },
            { Hack::PracticeMusic,      Tab::Other,  "practice-music" },
            { Hack::NoTransition,       Tab::Other,  "no-transition" },
            { Hack::MainMenuButton,     Tab::Other,  "main-menu-button" },
        }};

        constexpr bool tableMatchesEnum() {
            for (std::size_t i = 0; i < HACKS.size(); ++i) {
                if (static_cast<std::size_t>(HACKS[i].hack) != i) {
                    return false;
                }
            }
            return true;
        }
        static_assert(tableMatchesEnum(), "HACKS must list hacks in the same order as the Hack enum");

        constexpr char const* SPEED_KEY = "speed";

        std::array<bool, HACKS.size()> s_state {};
        float s_speed = 1.f;

        std::size_t toIndex(Hack hack) {
            return static_cast<std::size_t>(hack);
        }

        // Updates the cached value and reacts to it. Called both when the menu
        // flips a toggle and when the value changes from Geode's settings page.
        void applyState(Hack hack, bool on) {
            auto& state = s_state[toIndex(hack)];
            if (state == on) {
                return;
            }
            state = on;

            // The level only recomputes whether it draws hitboxes on reset,
            // so nudge it to make the toggle take effect right away
            if (hack == Hack::ShowHitboxes) {
                if (auto pl = PlayLayer::get()) {
                    pl->updateDebugDrawSettings();
                }
            }
        }
    }

    std::span<HackInfo const> all() {
        return HACKS;
    }

    HackInfo const& info(Hack hack) {
        return HACKS[toIndex(hack)];
    }

    bool isOn(Hack hack) {
        return s_state[toIndex(hack)];
    }

    void setOn(Hack hack, bool on) {
        applyState(hack, on);
        Mod::get()->setSettingValue<bool>(info(hack).key, on);
    }

    float speed() {
        return s_speed;
    }

    void setSpeed(float value) {
        s_speed = std::clamp(value, MIN_SPEED, MAX_SPEED);
        Mod::get()->setSettingValue<double>(SPEED_KEY, s_speed);
    }

    float activeSpeed() {
        return isOn(Hack::Speedhack) ? s_speed : 1.f;
    }

    bool cheatsActive() {
        return isOn(Hack::Noclip)
            || (isOn(Hack::Speedhack) && s_speed != 1.f)
            || isOn(Hack::NoMirror);
    }

    bool safeModeActive() {
        return isOn(Hack::SafeMode) || (isOn(Hack::AutoSafeMode) && cheatsActive());
    }

    void loadSettings() {
        auto mod = Mod::get();
        for (auto const& entry : HACKS) {
            s_state[toIndex(entry.hack)] = mod->getSettingValue<bool>(entry.key);
            listenForSettingChanges<bool>(entry.key, [hack = entry.hack](bool value) {
                applyState(hack, value);
            });
        }

        s_speed = std::clamp(static_cast<float>(mod->getSettingValue<double>(SPEED_KEY)), MIN_SPEED, MAX_SPEED);
        listenForSettingChanges<double>(SPEED_KEY, [](double value) {
            s_speed = std::clamp(static_cast<float>(value), MIN_SPEED, MAX_SPEED);
        });
    }
}
