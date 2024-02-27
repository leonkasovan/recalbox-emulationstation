//
// Created by bkg2k on 04/01/2021.
//
// As part of the RECALBOX Project
// http://www.recalbox.com
//

#include "GuiMenuBootSettings.h"
#include <guis/MenuMessages.h>
#include <recalbox/RecalboxSystem.h>
#include <systems/SystemManager.h>
#include <components/SwitchComponent.h>

GuiMenuBootSettings::GuiMenuBootSettings(WindowManager& window, SystemManager& systemManager)
  : GuiMenuBase(window, _("BOOT SETTINGS"), nullptr)
  , mSystemManager(systemManager)
{
  // Kodi at startup
  if (RecalboxSystem::kodiExists() && RecalboxConf::Instance().GetKodiEnabled())
    AddSwitch(_("KODI AT START"), RecalboxConf::Instance().GetKodiAtStartup(), (int)Components::KodiAtStartup, this, _(MENUMESSAGE_ADVANCED_KODI_AT_START_HELP_MSG));

  // Gamelists only
  AddSwitch(_("GAMELIST ONLY"), RecalboxConf::Instance().GetStartupGamelistOnly(), (int)Components::GamelistOnly, this, _(MENUMESSAGE_ADVANCED_GAMELISTONLY_HELP_MSG));

  // Selected System
  AddList<String>(_("BOOT ON SYSTEM"), (int)Components::SelectedSystem, this, GetSystemEntries(), _(MENUMESSAGE_ADVANCED_BOOT_ON_SYSTEM_HELP_MSG));

  // Boot on gamelist
  AddSwitch(_("BOOT ON GAMELIST"), RecalboxConf::Instance().GetStartupStartOnGamelist(), (int)Components::StartOnGamelist, this, _(MENUMESSAGE_ADVANCED_BOOTGAMELIST_HELP_MSG));

  // Boot videos
  AddSwitch(_("BOOT VIDEOS"), RecalboxConf::Instance().GetSplashEnabled(), (int)Components::SplashEnabled, this, _(MENUMESSAGE_ADVANCED_SPLASH_HELP_MSG));

  // Hide system view
  AddSwitch(_("HIDE SYSTEM VIEW"), RecalboxConf::Instance().GetStartupHideSystemView(), (int)Components::HideSystemView, this, _(MENUMESSAGE_ADVANCED_HIDESYSTEMVIEW_HELP_MSG));
}

std::vector<GuiMenuBase::ListEntry<String>> GuiMenuBootSettings::GetSystemEntries()
{
  std::vector<GuiMenuBase::ListEntry<String>> list;

  String selectedsystem = RecalboxConf::Instance().GetStartupSelectedSystem();
  if (selectedsystem.empty()) selectedsystem = SystemManager::sFavoriteSystemShortName;
  // For each activated system
  bool found = false;
  for (SystemData* system : mSystemManager.VisibleSystemList())
    if(system->Name() != SystemManager::sFavoriteSystemShortName)
    {
      bool match = (selectedsystem == system->Name());
      found |= match;
      list.push_back({ system->FullName(), system->Name(), match });
    }
  list.push_back({ _("Favorites"), SystemManager::sFavoriteSystemShortName, !found });

  return list;
}

void GuiMenuBootSettings::OptionListComponentChanged(int id, int index, const String& value)
{
  (void)index;
  if ((Components)id == Components::SelectedSystem)
  {
    RecalboxConf::Instance().SetStartupSelectedSystem(value).Save();
  }
}

void GuiMenuBootSettings::SwitchComponentChanged(int id, bool& status)
{
  switch((Components)id)
  {
    case Components::KodiAtStartup: RecalboxConf::Instance().SetKodiAtStartup(status).Save(); break;
    case Components::GamelistOnly: RecalboxConf::Instance().SetStartupGamelistOnly(status).Save(); break;
    case Components::StartOnGamelist: RecalboxConf::Instance().SetStartupStartOnGamelist(status).Save(); break;
    case Components::HideSystemView: RecalboxConf::Instance().SetStartupHideSystemView(status).Save(); break;
    case Components::SplashEnabled: RecalboxConf::Instance().SetSplashEnabled(status).Save(); break;
    case Components::SelectedSystem:break;
  }
}
