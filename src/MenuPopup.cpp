#include "MenuPopup.hpp"

#include <cstdlib>
#include <string>

using namespace geode::prelude;
using hacks::Hack;
using hacks::Tab;

namespace {
    constexpr float WIDTH = 400.f;
    constexpr float HEIGHT = 270.f;
    constexpr float PAGE_WIDTH = 380.f;
    constexpr float PAGE_HEIGHT = 168.f;
    constexpr float COLUMN_WIDTH = 190.f;
    constexpr float ROW_HEIGHT = 28.f;
    constexpr int ROWS_PER_COLUMN = 6;

    constexpr std::array<Tab, 3> TABS { Tab::Level, Tab::Editor, Tab::Other };
    constexpr std::array<char const*, 3> TAB_NAMES { "Level", "Editor", "Other" };

    // Reopen the menu on the tab it was closed on
    Tab s_lastTab = Tab::Level;

    std::size_t tabIndex(Tab tab) {
        return static_cast<std::size_t>(tab);
    }

    CCPoint slotPosition(int slot) {
        auto column = slot / ROWS_PER_COLUMN;
        auto row = slot % ROWS_PER_COLUMN;
        return ccp(column * COLUMN_WIDTH, PAGE_HEIGHT - ROW_HEIGHT / 2 - row * ROW_HEIGHT);
    }

    std::string settingName(char const* key) {
        if (auto setting = Mod::get()->getSetting(key)) {
            return setting->getDisplayName();
        }
        return key;
    }

    std::string settingDescription(char const* key) {
        if (auto setting = Mod::get()->getSetting(key)) {
            if (auto description = setting->getDescription()) {
                return *description;
            }
        }
        return "";
    }
}

MenuPopup* MenuPopup::create() {
    auto ret = new MenuPopup();
    if (ret->setup()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

void MenuPopup::toggle() {
    auto scene = CCDirector::sharedDirector()->getRunningScene();
    if (!scene) {
        return;
    }
    if (auto existing = typeinfo_cast<MenuPopup*>(scene->getChildByID("menu-popup"_spr))) {
        existing->onClose(nullptr);
        return;
    }
    if (auto popup = MenuPopup::create()) {
        popup->show();
    }
}

bool MenuPopup::setup() {
    if (!Popup::init(WIDTH, HEIGHT)) {
        return false;
    }
    this->setID("menu-popup"_spr);
    this->setTitle("Claude Mod Menu");

    auto tabMenu = CCMenu::create();
    tabMenu->ignoreAnchorPointForPosition(false);
    tabMenu->setAnchorPoint({ .5f, .5f });
    tabMenu->setContentSize({ PAGE_WIDTH, 30.f });
    tabMenu->setLayout(RowLayout::create()->setGap(8.f));
    for (auto tab : TABS) {
        auto sprite = ButtonSprite::create(TAB_NAMES[tabIndex(tab)], "bigFont.fnt", "GJ_button_04.png", .6f);
        m_tabSprites[tabIndex(tab)] = sprite;
        auto button = CCMenuItemExt::createSpriteExtra(sprite, [this, tab](CCMenuItemSpriteExtra*) {
            this->switchTab(tab);
        });
        tabMenu->addChild(button);
    }
    tabMenu->updateLayout();
    m_mainLayer->addChildAtPosition(tabMenu, Anchor::Top, ccp(0.f, -50.f));

    for (auto tab : TABS) {
        this->createPage(tab);
    }

    m_statusLabel = CCLabelBMFont::create("", "bigFont.fnt");
    m_statusLabel->setScale(.35f);
    m_mainLayer->addChildAtPosition(m_statusLabel, Anchor::Bottom, ccp(0.f, 16.f));

    // Geode's own settings page has the keybind and the speed slider
    auto settingsButton = CCMenuItemExt::createSpriteExtraWithFrameName("GJ_optionsBtn_001.png", .5f, [](CCMenuItemSpriteExtra*) {
        openSettingsPopup(Mod::get());
    });
    m_buttonMenu->addChildAtPosition(settingsButton, Anchor::BottomRight, ccp(-22.f, 22.f));

    this->switchTab(s_lastTab);
    this->updateStatus();
    return true;
}

void MenuPopup::createPage(Tab tab) {
    auto page = CCNode::create();
    page->setAnchorPoint({ .5f, .5f });
    page->setContentSize({ PAGE_WIDTH, PAGE_HEIGHT });

    auto menu = CCMenu::create();
    menu->setPosition({ 0.f, 0.f });
    page->addChild(menu);

    int slot = 0;
    for (auto const& entry : hacks::all()) {
        if (entry.tab == tab) {
            this->addToggle(page, menu, entry.hack, slotPosition(slot++));
        }
    }
    if (tab == Tab::Level) {
        this->addSpeedInput(page, slotPosition(slot++));
    }

    m_pages[tabIndex(tab)] = page;
    m_pageMenus[tabIndex(tab)] = menu;
    m_mainLayer->addChildAtPosition(page, Anchor::Center, ccp(0.f, -24.f));
}

void MenuPopup::addToggle(CCNode* page, CCMenu* menu, Hack hack, CCPoint const& pos) {
    auto key = hacks::info(hack).key;
    auto name = settingName(key);

    // Flip our own state instead of reading the toggler, since GD runs the
    // callback before the toggler updates itself
    auto toggler = CCMenuItemExt::createTogglerWithStandardSprites(.6f, [this, hack](CCMenuItemToggler*) {
        hacks::setOn(hack, !hacks::isOn(hack));
        this->updateStatus();
    });
    toggler->toggle(hacks::isOn(hack));
    toggler->setPosition(pos + ccp(15.f, 0.f));
    menu->addChild(toggler);

    auto label = CCLabelBMFont::create(name.c_str(), "bigFont.fnt");
    label->setAnchorPoint({ 0.f, .5f });
    label->limitLabelWidth(125.f, .45f, .1f);
    label->setPosition(pos + ccp(32.f, 0.f));
    page->addChild(label);

    auto description = settingDescription(key);
    if (!description.empty()) {
        auto infoButton = CCMenuItemExt::createSpriteExtraWithFrameName(
            "GJ_infoIcon_001.png", .45f,
            [name, description](CCMenuItemSpriteExtra*) {
                FLAlertLayer::create(name.c_str(), description.c_str(), "OK")->show();
            }
        );
        infoButton->setPosition(pos + ccp(172.f, 0.f));
        menu->addChild(infoButton);
    }
}

void MenuPopup::addSpeedInput(CCNode* page, CCPoint const& pos) {
    auto label = CCLabelBMFont::create("Speed", "bigFont.fnt");
    label->setAnchorPoint({ 0.f, .5f });
    label->setScale(.45f);
    label->setPosition(pos + ccp(32.f, 0.f));
    page->addChild(label);

    auto input = TextInput::create(80.f, "1.00");
    input->setCommonFilter(CommonFilter::Float);
    input->setMaxCharCount(6);
    input->setScale(.65f);
    input->setString(fmt::format("{:.2f}", hacks::speed()).c_str());
    input->setCallback([this](std::string const& text) {
        char* end = nullptr;
        auto value = std::strtof(text.c_str(), &end);
        if (end != text.c_str() && value > 0.f) {
            hacks::setSpeed(value);
            this->updateStatus();
        }
    });
    input->setPosition(pos + ccp(135.f, 0.f));
    page->addChild(input);
    m_speedInput = input;
}

void MenuPopup::switchTab(Tab tab) {
    for (auto other : TABS) {
        auto i = tabIndex(other);
        bool active = other == tab;
        m_pages[i]->setVisible(active);
        m_pageMenus[i]->setEnabled(active);
        m_tabSprites[i]->updateBGImage(active ? "GJ_button_01.png" : "GJ_button_04.png");
    }
    if (m_speedInput) {
        m_speedInput->setEnabled(tab == Tab::Level);
    }
    s_lastTab = tab;
}

void MenuPopup::updateStatus() {
    if (!m_statusLabel) {
        return;
    }
    if (hacks::safeModeActive()) {
        m_statusLabel->setString("Safe Mode on: progress won't be saved");
        m_statusLabel->setColor({ 120, 255, 120 });
    }
    else if (hacks::cheatsActive()) {
        m_statusLabel->setString("Cheats on and Safe Mode off");
        m_statusLabel->setColor({ 255, 120, 120 });
    }
    else {
        m_statusLabel->setString("Safe Mode off");
        m_statusLabel->setColor({ 200, 200, 200 });
    }
}

CCMenuItemSpriteExtra* createMenuButton(CircleBaseSize size) {
    auto label = CCLabelBMFont::create("M", "bigFont.fnt");
    auto sprite = CircleButtonSprite::create(label, CircleBaseColor::Cyan, size);
    auto button = CCMenuItemExt::createSpriteExtra(sprite, [](CCMenuItemSpriteExtra*) {
        MenuPopup::toggle();
    });
    button->setID("menu-button"_spr);
    return button;
}
