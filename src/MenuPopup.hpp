#pragma once

#include "Hacks.hpp"

#include <Geode/Geode.hpp>

#include <array>

class MenuPopup : public geode::Popup {
protected:
    // One page per tab. Labels live on the page node, buttons on its menu.
    std::array<cocos2d::CCNode*, 3> m_pages {};
    std::array<cocos2d::CCMenu*, 3> m_pageMenus {};
    std::array<ButtonSprite*, 3> m_tabSprites {};
    geode::TextInput* m_speedInput = nullptr;
    cocos2d::CCLabelBMFont* m_statusLabel = nullptr;

    bool setup();
    void createPage(hacks::Tab tab);
    void addToggle(cocos2d::CCNode* page, cocos2d::CCMenu* menu, hacks::Hack hack, cocos2d::CCPoint const& pos);
    void addSpeedInput(cocos2d::CCNode* page, cocos2d::CCPoint const& pos);
    void switchTab(hacks::Tab tab);
    void updateStatus();

public:
    static MenuPopup* create();
    // Opens the menu, or closes it if it's already open
    static void toggle();
};

// Round button that opens the menu, for adding to other layers
CCMenuItemSpriteExtra* createMenuButton(geode::CircleBaseSize size);
