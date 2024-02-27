//
// Created by bkg2k on 08/01/2021.
//
// As part of the RECALBOX Project
// http://www.recalbox.com
//

#include <guis/menus/GuiMenuVirtualSystems.h>
#include <components/SwitchComponent.h>
#include <utils/locale/LocaleHelper.h>
#include <MainRunner.h>
#include <guis/MenuMessages.h>
#include <guis/menus/GuiMenuVirtualSystemPerGenre.h>
#include <systems/SystemManager.h>

GuiMenuVirtualSystems::GuiMenuVirtualSystems(WindowManager& window, SystemManager& systemManager)
  : GuiMenuBase(window, _("VIRTUAL SYSTEMS"), this)
  , mSystemManager(systemManager)
{
  // All games
  AddSwitch(_("SHOW ALL-GAMES SYSTEM"), RecalboxConf::Instance().GetCollectionAllGames(), (int)Components::AllGames, this, _(MENUMESSAGE_ADVANCED_ALLGAMES_HELP_MSG));

  // Multiplayers
  AddSwitch(_("SHOW MULTIPLAYER SYSTEM"), RecalboxConf::Instance().GetCollectionMultiplayer(), (int)Components::Multiplayers, this, _(MENUMESSAGE_ADVANCED_MULTIPLAYERS_HELP_MSG));

  // Last Played
  AddSwitch(_("SHOW LAST-PLAYED SYSTEM"), RecalboxConf::Instance().GetCollectionLastPlayed(), (int)Components::LastPlayed, this, _(MENUMESSAGE_ADVANCED_LASTPLAYED_HELP_MSG));

  // Lightgun
  BoardType board = Board::Instance().GetBoardType();
  if (board != BoardType::OdroidAdvanceGo && board != BoardType::OdroidAdvanceGoSuper && board != BoardType::RG351V && board != BoardType::RG351P)
    AddSwitch(_("SHOW LIGHTGUN SYSTEM"), RecalboxConf::Instance().GetCollectionLightGun(), (int)Components::LightGun, this, _(MENUMESSAGE_ADVANCED_LIGHTGUN_HELP_MSG));

  // Ports
  AddSwitch(_("SHOW PORTS SYSTEM"), RecalboxConf::Instance().GetCollectionPorts(), (int)Components::Ports, this, _(MENUMESSAGE_ADVANCED_PORTS_HELP_MSG));

  AddSubMenu(_("VIRTUAL SYSTEMS PER GENRE"), (int)Components::VirtualPerGenre, _(MENUMESSAGE_ADVANCED_VIRTUALGENRESYSTEMS_HELP_MSG));
}

void GuiMenuVirtualSystems::SubMenuSelected(int id)
{
  if ((Components)id == Components::VirtualPerGenre)
    mWindow.pushGui(new GuiMenuVirtualSystemPerGenre(mWindow, mSystemManager));
}

void GuiMenuVirtualSystems::SwitchComponentChanged(int id, bool& status)
{
  SystemManager::Visibility visibility = status ? SystemManager::Visibility::ShowAndSelect : SystemManager::Visibility::Hide;
  switch((Components)id)
  {
    case Components::AllGames:
    {
      RecalboxConf::Instance().SetCollectionAllGames(status).Save();
      mSystemManager.UpdateVirtualSystemsVisibility(VirtualSystemType::AllGames, visibility);
      break;
    }
    case Components::Multiplayers:
    {
      RecalboxConf::Instance().SetCollectionMultiplayer(status).Save();
      mSystemManager.UpdateVirtualSystemsVisibility(VirtualSystemType::Multiplayers, visibility);
      break;
    }
    case Components::LastPlayed:
    {
      RecalboxConf::Instance().SetCollectionLastPlayed(status).Save();
      mSystemManager.UpdateVirtualSystemsVisibility(VirtualSystemType::LastPlayed, visibility);
      break;
    }
    case Components::LightGun:
    {
      RecalboxConf::Instance().SetCollectionLightGun(status).Save();
      mSystemManager.UpdateVirtualSystemsVisibility(VirtualSystemType::Lightgun, visibility);
      break;
    }
    case Components::Ports:
    {
      RecalboxConf::Instance().SetCollectionPorts(status).Save();
      mSystemManager.UpdateVirtualSystemsVisibility(VirtualSystemType::Ports, visibility);
      break;
    }
    case Components::VirtualPerGenre: break;
  }
}
