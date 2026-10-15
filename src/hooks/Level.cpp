#include "../Hacks.hpp"

#include <Geode/Geode.hpp>
#include <Geode/modify/CCScheduler.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>

#include <chrono>
#include <cmath>
#include <deque>

using namespace geode::prelude;
using hacks::Hack;

namespace {
    using Clock = std::chrono::steady_clock;

    // Jump presses from the last second, for the CPS counter
    std::deque<Clock::time_point> s_clicks;

    void dropOldClicks() {
        auto cutoff = Clock::now() - std::chrono::seconds(1);
        while (!s_clicks.empty() && s_clicks.front() < cutoff) {
            s_clicks.pop_front();
        }
    }

    // Marks the run as a test run while GD's saving code runs. That's how GD
    // itself avoids saving progress from start positions.
    class SafeModeGuard {
        GJBaseGameLayer* m_layer;
        bool m_active;
        bool m_wasTestMode;

    public:
        explicit SafeModeGuard(GJBaseGameLayer* layer)
          : m_layer(layer),
            m_active(hacks::safeModeActive()),
            m_wasTestMode(layer->m_isTestMode)
        {
            if (m_active) {
                m_layer->m_isTestMode = true;
            }
        }

        ~SafeModeGuard() {
            if (m_active) {
                m_layer->m_isTestMode = m_wasTestMode;
            }
        }

        SafeModeGuard(SafeModeGuard const&) = delete;
        SafeModeGuard& operator=(SafeModeGuard const&) = delete;
    };
}

class $modify(CMMScheduler, CCScheduler) {
    void update(float dt) {
        CCScheduler::update(dt * hacks::activeSpeed());
    }
};

class $modify(CMMGameLayer, GJBaseGameLayer) {
    void handleButton(bool down, int button, bool isPlayer1) {
        // Button 1 is jump; 2 and 3 are platformer left/right
        if (down && button == 1) {
            s_clicks.push_back(Clock::now());
            dropOldClicks();
        }
        GJBaseGameLayer::handleButton(down, button, isPlayer1);
    }

    void toggleFlipped(bool flip, bool noEffects) {
        // Only block flipping, so turning the hack on while mirrored can
        // still unflip the screen on the next reset
        if (flip && hacks::isOn(Hack::NoMirror)) {
            return;
        }
        GJBaseGameLayer::toggleFlipped(flip, noEffects);
    }
};

class $modify(CMMPlayLayer, PlayLayer) {
    struct Fields {
        CCLabelBMFont* overlay = nullptr;
        float overlayTime = 0.f;
        int overlayFrames = 0;
        bool hidAttempts = false;
        bool hidPauseButton = false;
    };

    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) {
            return false;
        }

        auto winSize = CCDirector::sharedDirector()->getWinSize();
        auto overlay = CCLabelBMFont::create("", "bigFont.fnt");
        overlay->setAnchorPoint({ 0.f, 1.f });
        overlay->setScale(.3f);
        overlay->setOpacity(200);
        overlay->setPosition({ 5.f, winSize.height - 5.f });
        overlay->setVisible(false);
        overlay->setID("info-overlay"_spr);
        this->addChild(overlay, 1000);
        m_fields->overlay = overlay;

        return true;
    }

    void destroyPlayer(PlayerObject* player, GameObject* object) {
        // The anticheat spike has to go through or GD flags the attempt
        if (hacks::isOn(Hack::Noclip) && object != m_anticheatSpike) {
            return;
        }
        SafeModeGuard guard(this);
        PlayLayer::destroyPlayer(player, object);
    }

    void levelComplete() {
        SafeModeGuard guard(this);
        PlayLayer::levelComplete();
    }

    void updateDebugDrawSettings() {
        PlayLayer::updateDebugDrawSettings();
        if (!m_debugDrawNode) {
            return;
        }
        if (hacks::isOn(Hack::ShowHitboxes)) {
            m_isDebugDrawEnabled = true;
            m_debugDrawNode->setVisible(true);
        }
        else if (!m_isDebugDrawEnabled) {
            // Don't leave the last drawn frame on screen after turning it off
            m_debugDrawNode->clear();
        }
    }

    void updateProgressbar() {
        PlayLayer::updateProgressbar();
        if (hacks::isOn(Hack::AccuratePercentage) && !m_isPlatformer && m_percentageLabel) {
            m_percentageLabel->setString(fmt::format("{:.2f}%", this->getCurrentPercent()).c_str());
        }
    }

    void postUpdate(float dt) {
        bool forceHitboxes = hacks::isOn(Hack::ShowHitboxes);
        if (forceHitboxes) {
            m_isDebugDrawEnabled = true;
        }

        PlayLayer::postUpdate(dt);

        if (forceHitboxes && m_debugDrawNode && !m_debugDrawNode->isVisible()) {
            m_debugDrawNode->setVisible(true);
        }
        this->updateHiddenNodes();
        this->updateOverlay();
    }

    void updateHiddenNodes() {
        auto fields = m_fields.self();

        // Only touch visibility we changed ourselves, so GD's own options
        // for hiding these keep working
        if (m_attemptLabel) {
            bool hide = hacks::isOn(Hack::HideAttempts);
            if (hide) {
                m_attemptLabel->setVisible(false);
            }
            else if (fields->hidAttempts) {
                m_attemptLabel->setVisible(true);
            }
            fields->hidAttempts = hide;
        }

        auto pauseButton = m_uiLayer ? m_uiLayer->m_pauseBtn : nullptr;
        if (pauseButton) {
            bool hide = hacks::isOn(Hack::HidePauseButton);
            if (hide) {
                pauseButton->setVisible(false);
            }
            else if (fields->hidPauseButton) {
                pauseButton->setVisible(true);
            }
            fields->hidPauseButton = hide;
        }
    }

    void updateOverlay() {
        auto fields = m_fields.self();
        if (!fields->overlay) {
            return;
        }

        bool show = hacks::isOn(Hack::InfoOverlay);
        fields->overlay->setVisible(show);
        if (!show) {
            return;
        }

        // Director delta is real frame time, so speedhack doesn't skew FPS
        fields->overlayTime += CCDirector::sharedDirector()->getDeltaTime();
        fields->overlayFrames += 1;
        if (fields->overlayTime < .25f) {
            return;
        }

        auto fps = static_cast<int>(std::round(fields->overlayFrames / fields->overlayTime));
        fields->overlayTime = 0.f;
        fields->overlayFrames = 0;

        dropOldClicks();
        auto text = fmt::format("FPS: {}\nCPS: {}\nAttempt: {}", fps, s_clicks.size(), m_attempts);
        if (hacks::safeModeActive()) {
            text += "\nSafe Mode";
        }
        fields->overlay->setString(text.c_str());
    }
};

class $modify(CMMPlayerObject, PlayerObject) {
    void playDeathEffect() {
        if (hacks::isOn(Hack::NoDeathEffect)) {
            return;
        }
        PlayerObject::playDeathEffect();
    }
};
