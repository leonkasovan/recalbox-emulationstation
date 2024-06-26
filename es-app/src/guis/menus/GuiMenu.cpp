#include <guis/menus/GuiMenu.h>
#include <guis/MenuMessages.h>
#include <recalbox/RecalboxSystem.h>
#include <animations/LambdaAnimation.h>

#include <guis/menus/GuiMenuSystem.h>
#include <guis/menus/GuiMenuUpdates.h>
#include <guis/menus/GuiMenuGameSettings.h>
#include <guis/menus/GuiMenuPads.h>
#include <guis/menus/GuiMenuUserInterface.h>
#include <guis/menus/GuiMenuSound.h>
#include <guis/menus/GuiMenuNetwork.h>
#include <guis/menus/GuiMenuAdvancedSettings.h>
#include <guis/menus/GuiMenuCRT.h>
#include "guis/GuiBiosScan.h"
#include "guis/menus/GuiMenuQuit.h"
#include <emulators/run/GameRunner.h>
#include "GuiMenuScraper.h"
#include "GuiMenuTate.h"
#include "GuiMenuDownloadContents.h"
#include "GuiMenuArcade.h"
#include "GuiMenuDownloadGamePacks.h"
#include <guis/GuiScraperRun.h>
#include <guis/GuiMsgBoxScroll2.h>
#include <guis/wizards/WizardRG353X.h>

GuiMenu::GuiMenu(WindowManager& window, SystemManager& systemManager)
  : GuiMenuBase(window, _("MAIN MENU"), this)
  , mSystemManager(systemManager)
{
  // Bartop mode?
  bool bartop = RecalboxConf::Instance().GetMenuType() == RecalboxConf::Menu::Bartop;

  // Kodi
  if (RecalboxSystem::kodiExists() && RecalboxConf::Instance().GetKodiEnabled())
    AddSubMenu(_("KODI MEDIA CENTER"), mTheme.menuIconSet.kodi, (int)Components::Kodi, _(MENUMESSAGE_START_KODI_HELP_MSG));

  // System menu
  if (!bartop)
    AddSubMenu(_("SYSTEM SETTINGS"), mTheme.menuIconSet.system, (int)Components::System, _(MENUMESSAGE_SYSTEM_HELP_MSG));

  // Update menu
  if (!bartop)
    AddSubMenu(_("UPDATES"), mTheme.menuIconSet.updates, (int)Components::Update, _(MENUMESSAGE_UPDATE_HELP_MSG));

  // Recalbox RGB Dual menu
  if(Board::Instance().CrtBoard().GetCrtAdapter() == CrtAdapterType::RGBDual)
    AddSubMenu(_("RECALBOX RGB DUAL"), mTheme.menuIconSet.recalboxrgbdual, (int)Components::RecalboxRGBDual, _(MENUMESSAGE_RECALBOXRGBDUAL_HELP_MSG));
  if(Board::Instance().CrtBoard().GetCrtAdapter() == CrtAdapterType::RGBJamma || Board::Instance().CrtBoard().GetCrtAdapter() == CrtAdapterType::RGBJammaV2)
    AddSubMenu(_("RECALBOX RGB JAMMA"), mTheme.menuIconSet.recalboxrgbdual, (int)Components::RecalboxRGBDual, _(MENUMESSAGE_RECALBOXRGBDUAL_HELP_MSG));

  // Games menu
  AddSubMenu(_("GAMES SETTINGS"), mTheme.menuIconSet.games, (int)Components::Games, _(MENUMESSAGE_GAME_SETTINGS_HELP_MSG));

  // Download menu
  if (!bartop)
    AddSubMenu(_("DOWNLOAD CONTENTS"), mTheme.menuIconSet.download, (int)Components::ContentDoanwloader, _(MENUMESSAGE_DOWNLOADERS_SETTINGS_HELP_MSG));

  // Controllers menu
  if (!bartop)
    AddSubMenu(_("CONTROLLERS SETTINGS"), mTheme.menuIconSet.controllers, (int)Components::Controllers, _(MENUMESSAGE_CONTROLLER_HELP_MSG));

  // UI Settings menu
  if (!bartop)
    AddSubMenu(_("UI SETTINGS"), mTheme.menuIconSet.ui, (int)Components::UISettings, _(MENUMESSAGE_UI_HELP_MSG));

  // Atcade menu
  if (!bartop)
    AddSubMenu(_("ARCADE SETTINGS"), mTheme.menuIconSet.arcade, (int)Components::Arcade, _(MENUMESSAGE_ARCADE_HELP_MSG));

  // TATE menu
  if (!bartop)
    AddSubMenu(_("TATE SETTINGS"), mTheme.menuIconSet.tate, (int)Components::Tate, _(MENUMESSAGE_TATE_HELP_MSG));

  // Sound menu
  AddSubMenu(_("SOUND SETTINGS"), mTheme.menuIconSet.sound, (int)Components::Sound, _(MENUMESSAGE_SOUND_HELP_MSG));

  // Network
  if (!bartop)
    AddSubMenu(_("NETWORK SETTINGS"), mTheme.menuIconSet.network, (int)Components::Network, _(MENUMESSAGE_NETWORK_HELP_MSG));

  // Scraper
  if (!bartop)
    AddSubMenu(_("SCRAPER"), mTheme.menuIconSet.scraper, (int)Components::Scraper, _(MENUMESSAGE_SCRAPER_HELP_MSG));

  // Advanced
  if (!bartop)
    AddSubMenu(_("ADVANCED SETTINGS"), mTheme.menuIconSet.advanced, (int)Components::Advanced, _(MENUMESSAGE_ADVANCED_HELP_MSG));

  // Bios
  if (!bartop)
    AddSubMenu(_("BIOS CHECKING"), mTheme.menuIconSet.games, (int)Components::Bios, _(MENUMESSAGE_BIOS_HELP_MSG));

  // License
  AddSubMenu(_("OPEN-SOURCE LICENSE"), mTheme.menuIconSet.license, (int)Components::License);

  // Help and Guide
  AddSubMenu(_("HELP"), mTheme.menuIconSet.license, (int)Components::Help);

  // Wizard
  AddSubMenu(_("WIZARD"), mTheme.menuIconSet.license, (int)Components::Wizard);

  // Quit
  AddSubMenu(_("QUIT"), mTheme.menuIconSet.quit, (int)Components::Quit);

  // Animation
  auto fadeFunc = [this](float t)
  {
    setOpacity((int)lerp<float>(0, 255, t));
    setPosition(getPosition().x(),
                lerp<float>(Renderer::Instance().DisplayHeightAsFloat(), (Renderer::Instance().DisplayHeightAsFloat() - mSize.y()) / 2, t));
  };

  setOpacity(0);
  setAnimation(new LambdaAnimation(fadeFunc, 200), 0);
}

void GuiMenu::SubMenuSelected(int id)
{
  switch((Components)id)
  {
    case Components::Kodi: if (!GameRunner::Instance().RunKodi()) { LOG(LogWarning) << "[Kodi] Error running Kodi."; } break;
    case Components::System: mWindow.pushGui(new GuiMenuSystem(mWindow, mSystemManager)); break;
    case Components::Update: mWindow.pushGui(new GuiMenuUpdates(mWindow)); break;
    case Components::RecalboxRGBDual: mWindow.pushGui(new GuiMenuCRT(mWindow, Board::Instance().CrtBoard().GetCrtAdapter() == CrtAdapterType::RGBJamma ? _("JAMMA SETTINGS") : _("CRT SETTINGS"))); break;
    case Components::Games: mWindow.pushGui(new GuiMenuGameSettings(mWindow, mSystemManager)); break;
    case Components::ContentDoanwloader:
    {
      #if defined(BETA) || defined(DEBUG)
        mWindow.pushGui(new GuiMenuDownloadContents(mWindow, mSystemManager)); break;
      #else
        mWindow.pushGui(new GuiMenuDownloadGamePacks(mWindow, mSystemManager)); break;
      #endif
    }
    case Components::Controllers: mWindow.pushGui(new GuiMenuPads(mWindow)); break;
    case Components::UISettings: mWindow.pushGui(new GuiMenuUserInterface(mWindow, mSystemManager)); break;
    case Components::Arcade: mWindow.pushGui(new GuiMenuArcade(mWindow, mSystemManager, nullptr)); break;
    case Components::Tate: mWindow.pushGui(new GuiMenuTate(mWindow, mSystemManager)); break;
    case Components::Sound: mWindow.pushGui(new GuiMenuSound(mWindow)); break;
    case Components::Network: mWindow.pushGui(new GuiMenuNetwork(mWindow)); break;
    case Components::Scraper:
    {
      if (GuiScraperRun::IsRunning())
        GuiScraperRun::CreateOrShow(mWindow, mSystemManager, SystemManager::List(), ScrapingMethod::All, &GameRunner::Instance(), Renderer::Instance().DisplayHeightAsFloat() <= 576);
      else
        mWindow.pushGui(new GuiMenuScraper(mWindow, mSystemManager));
      break;
    }
    case Components::Advanced: mWindow.pushGui(new GuiMenuAdvancedSettings(mWindow, mSystemManager)); break;
    case Components::Bios: mWindow.pushGui(new GuiBiosScan(mWindow, mSystemManager)); break;
    case Components::License:
    {
      mWindow.pushGui(
        new GuiMsgBoxScroll(mWindow, "RECALBOX",
                            ScrambleSymetric2(String(MenuMessages::LICENCE_MSG, MenuMessages::LICENCE_MSG_SIZE), __MESSAGE_DECORATOR),
                            _("OK"), nullptr, "", nullptr, "", nullptr, TextAlignment::Left));
      break;
    }
    case Components::Quit: mWindow.pushGui(new GuiMenuQuit(mWindow)); break;
    case Components::Help: mWindow.pushGui(new GuiMsgBoxScroll2(mWindow, "Recalbox Guides", "content", _("OK"), nullptr, "", nullptr, "", nullptr, TextAlignment::Left)); break;
    case Components::Wizard: mWindow.pushGui(new WizardRG353X(mWindow)); break;
      break;
  }
}

String GuiMenu::ScrambleSymetric2(const String& input, const String& key)
{
  String buffer = input;
  int l = (int)key.size();

  for (size_t i = 0; i < input.size(); ++i)
    buffer[i] = (char)(input[i] ^ (key[i % l] + (i*17)));

  return buffer;
}
