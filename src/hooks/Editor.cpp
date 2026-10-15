#include "../Hacks.hpp"

#include <Geode/Geode.hpp>
#include <Geode/modify/EditLevelLayer.hpp>
#include <Geode/modify/EditorUI.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>

#include <algorithm>

using namespace geode::prelude;
using hacks::Hack;

namespace {
    // Vanilla allows 0.1 to 4; going much further makes the editor unusable
    constexpr float MIN_ZOOM = .02f;
    constexpr float MAX_ZOOM = 20.f;

    // A level password of 1 means "free to copy"
    constexpr int FREE_COPY_PASSWORD = 1;
}

class $modify(CMMEditLevelLayer, EditLevelLayer) {
    void onShare(CCObject* sender) {
        if (m_level) {
            if (hacks::isOn(Hack::VerifyHack)) {
                m_level->m_isVerified = 1;
                m_level->m_isVerifiedRaw = true;
            }
            if (hacks::isOn(Hack::NoCopyMark)) {
                m_level->m_originalLevel = 0;
            }
        }
        EditLevelLayer::onShare(sender);
    }
};

class $modify(CMMEditorUI, EditorUI) {
    void constrainGameLayerPosition(float x, float y) {
        if (hacks::isOn(Hack::FreeScroll)) {
            return;
        }
        EditorUI::constrainGameLayerPosition(x, y);
    }

    void updateZoom(float zoom) {
        // The vanilla function clamps the zoom, so apply the real value after
        EditorUI::updateZoom(zoom);
        if (hacks::isOn(Hack::ZoomBypass) && m_editorLayer && m_editorLayer->m_objectLayer) {
            m_editorLayer->m_objectLayer->setScale(std::clamp(zoom, MIN_ZOOM, MAX_ZOOM));
        }
    }
};

class $modify(CMMLevelInfoLayer, LevelInfoLayer) {
    bool init(GJGameLevel* level, bool challenge) {
        if (level && hacks::isOn(Hack::CopyHack)) {
            level->m_password = FREE_COPY_PASSWORD;
        }
        return LevelInfoLayer::init(level, challenge);
    }

    void tryCloneLevel(CCObject* sender) {
        // The level download can overwrite the password set in init
        if (m_level && hacks::isOn(Hack::CopyHack)) {
            m_level->m_password = FREE_COPY_PASSWORD;
        }
        LevelInfoLayer::tryCloneLevel(sender);
    }
};
