#include "../Hacks.hpp"
#include "../MenuPopup.hpp"

#include <Geode/Geode.hpp>
#include <Geode/modify/EditorPauseLayer.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>

using namespace geode::prelude;
using hacks::Hack;

namespace {
    // Puts the menu button into one of the layer's existing menus (node IDs
    // come from the geode.node-ids dependency), or its own menu at
    // `fallbackPos` if that menu isn't there
    void addMenuButton(CCNode* layer, char const* menuID, CircleBaseSize size, CCPoint const& fallbackPos) {
        auto button = createMenuButton(size);
        if (auto existing = layer->getChildByID(menuID)) {
            existing->addChild(button);
            existing->updateLayout();
            return;
        }

        auto own = CCMenu::create();
        own->setID("menu-button-menu"_spr);
        own->setPosition(fallbackPos);
        own->addChild(button);
        layer->addChild(own, 100);
    }
}

class $modify(CMMMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) {
            return false;
        }
        if (hacks::isOn(Hack::MainMenuButton)) {
            auto winSize = CCDirector::sharedDirector()->getWinSize();
            addMenuButton(this, "bottom-menu", CircleBaseSize::MediumAlt, ccp(winSize.width / 2, 45.f));
        }
        return true;
    }
};

// PauseLayer::init is inlined on Windows, so buttons go in customSetup
class $modify(CMMPauseLayer, PauseLayer) {
    void customSetup() {
        PauseLayer::customSetup();
        auto winSize = CCDirector::sharedDirector()->getWinSize();
        addMenuButton(this, "right-button-menu", CircleBaseSize::Small, ccp(winSize.width - 36.f, 36.f));
    }
};

class $modify(CMMEditorPauseLayer, EditorPauseLayer) {
    bool init(LevelEditorLayer* editorLayer) {
        if (!EditorPauseLayer::init(editorLayer)) {
            return false;
        }
        auto winSize = CCDirector::sharedDirector()->getWinSize();
        addMenuButton(this, "settings-menu", CircleBaseSize::Small, ccp(winSize.width - 36.f, winSize.height - 80.f));
        return true;
    }
};
