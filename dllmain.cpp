#include "pch.h"
#include "share.hpp"
#include <nfd.h>

gd::EditLevelLayer* editLevelLayer;
gd::LevelBrowserLayer* levelBrowserLayer;
gd::LevelInfoLayer* levelInfoLayer;

class ImportExportCB {
public:
    void onExportLevelELL(CCObject* obj) {
        auto* const level = editLevelLayer->level();
        nfdchar_t* path = nullptr;
        if (NFD_SaveDialog("gmd", nullptr, &path) == NFD_OKAY) {
            std::ofstream file(path);
            dump_level(level, file);
            free(path);
            gd::FLAlertLayer::create(nullptr, "Success", "The level has been saved", "OK", nullptr, 320.f, false, 0)->show();
        }
    }
    void onExportLevelLIL(CCObject* obj) {
        auto* const level = levelInfoLayer->level();
        nfdchar_t* path = nullptr;
        if (NFD_SaveDialog("gmd", nullptr, &path) == NFD_OKAY) {
            std::ofstream file(path);
            dump_level(level, file);
            free(path);
            gd::FLAlertLayer::create(nullptr, "Success", "The level has been saved", "OK", nullptr, 320.f, false, 0)->show();
        }
    }
    void onImportLevel(CCObject* obj) {
        nfdchar_t* path = nullptr;
        if (NFD_OpenDialog("gmd", nullptr, &path) == NFD_OKAY) {
            std::ifstream file(path);
            auto* const level = import_level(file);
            free(path);
            if (!level) {
                gd::FLAlertLayer::create(nullptr, "Error", "Failed to import", "OK", nullptr, 320.f, false, 0)->show();
                return;
            }
            auto scene = gd::EditLevelLayer::scene(level);
            CCDirector::sharedDirector()->pushScene(scene);
        }
    }
};

namespace EditLevelLayer {
    bool(__thiscall* initO)(gd::EditLevelLayer*, gd::GJGameLevel*);
    bool __fastcall initH(gd::EditLevelLayer* self, void*, gd::GJGameLevel* level) {
        if (!EditLevelLayer::initO(self, level)) return false;
        editLevelLayer = self;

        auto director = CCDirector::sharedDirector();
        auto menu = CCMenu::create();

        auto btn_spr = CCSprite::createWithSpriteFrameName("GJ_downloadBtn_001.png");
        if (!btn_spr->initWithFile("BE_Export_File.png")) {
            btn_spr->initWithSpriteFrameName("GJ_downloadBtn_001.png");
        }
        auto button = gd::CCMenuItemSpriteExtra::create(btn_spr, nullptr, self, menu_selector(ImportExportCB::onExportLevelELL));
        button->setPosition({ -30, +30 });

        menu->setZOrder(1);
        menu->setPosition({ director->getScreenRight(), director->getScreenBottom() });
        menu->addChild(button);
        self->addChild(menu);

        return true;
    }
}

namespace LevelBrowserLayer {
    bool(__thiscall* initO)(gd::LevelBrowserLayer*, gd::GJSearchObject*);
    bool __fastcall initH(gd::LevelBrowserLayer* self, void*, gd::GJSearchObject* searchObject) {
        if (!LevelBrowserLayer::initO(self, searchObject)) return false;
        levelBrowserLayer = self;

        if (searchObject->m_type == gd::SearchType::MyLevels) {
            auto menu = CCMenu::create();
            menu->setPosition(CCDirector::sharedDirector()->getWinSize().width - 30.f, 90);
            self->addChild(menu);

            auto btn_spr = CCSprite::createWithSpriteFrameName("GJ_downloadBtn_001.png");
            if (!btn_spr->initWithFile("BE_Import_File.png")) {
                btn_spr->initWithSpriteFrameName("GJ_downloadBtn_001.png");
            }

            auto button = gd::CCMenuItemSpriteExtra::create(btn_spr, nullptr, self, menu_selector(ImportExportCB::onImportLevel));

            menu->setZOrder(1);
            menu->addChild(button);
        }

        return true;
    }
}

namespace LevelInfoLayer {
    bool(__thiscall* initO)(gd::LevelInfoLayer*, gd::GJGameLevel*);
    bool __fastcall initH(gd::LevelInfoLayer* self, void*, gd::GJGameLevel* level) {
        if (!LevelInfoLayer::initO(self, level)) return false;
        levelInfoLayer = self;

        auto director = CCDirector::sharedDirector();
        auto winSize = director->getWinSize();

        auto menu = from<CCMenu*>(self, 0x138);

        auto btn_spr = CCSprite::createWithSpriteFrameName("GJ_downloadBtn_001.png");
        if (!btn_spr->initWithFile("BE_Export_File.png")) {
            btn_spr->initWithSpriteFrameName("GJ_downloadBtn_001.png");
        }
        auto button = gd::CCMenuItemSpriteExtra::create(btn_spr, nullptr, self, menu_selector(ImportExportCB::onExportLevelLIL));

        button->setPosition({ -(winSize.width / 2) + 30, 24 });
        menu->addChild(button);

        return true;
    }
}

DWORD WINAPI my_thread(void* hModule) {
    if (MH_Initialize() != MH_OK) {
        FreeLibraryAndExitThread(reinterpret_cast<HMODULE>(hModule), 0);
    }

    auto cocos = GetModuleHandleA("libcocos2d.dll");
    auto cocos_ext = GetModuleHandleA("libExtensions.dll");

    MH_CreateHook(reinterpret_cast<void*>(gd::base + 0x3b5a0), EditLevelLayer::initH, reinterpret_cast<void**>(&EditLevelLayer::initO));
    MH_CreateHook(reinterpret_cast<void*>(gd::base + 0x89590), LevelBrowserLayer::initH, reinterpret_cast<void**>(&LevelBrowserLayer::initO));
    MH_CreateHook(reinterpret_cast<void*>(gd::base + 0x9bc10), LevelInfoLayer::initH, reinterpret_cast<void**>(&LevelInfoLayer::initO));

    MH_EnableHook(MH_ALL_HOOKS);

    return true;
}

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        CreateThread(0, 0x1000, my_thread, hModule, 0, 0);
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

