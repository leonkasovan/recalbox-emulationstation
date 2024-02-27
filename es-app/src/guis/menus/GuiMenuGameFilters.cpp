//
// Created by gugue_u on 29/03/2022.
//
//

#include "GuiMenuGameFilters.h"
#include <RecalboxConf.h>
#include <views/gamelist/ISimpleGameListView.h>
#include <games/GameFilesUtils.h>
#include "views/ViewController.h"
#include "components/SwitchComponent.h"
#include "utils/locale/LocaleHelper.h"
#include "guis/MenuMessages.h"
#include "GuiMenu.h"
#include "guis/GuiMsgBox.h"

GuiMenuGameFilters::GuiMenuGameFilters(WindowManager& window, SystemManager& systemManager)
  :	GuiMenuBase(window, _("GAME FILTERS"), nullptr)
  , mSystemManager(systemManager)
{
  AddSwitch(_("SHOW ONLY LATEST VERSION") + " (BETA)", RecalboxConf::Instance().GetShowOnlyLatestVersion(), (int)Components::ShowOnlyLatestVersion, this, _(MENUMESSAGE_UI_ONLY_LAST_VERSION_MSG));

  AddSwitch(_("SHOW ONLY FAVORITES"), RecalboxConf::Instance().GetFavoritesOnly(), (int)Components::FavoritesOnly, this, _(MENUMESSAGE_UI_FAVORITES_ONLY_MSG));

  AddSwitch(_("SHOW HIDDEN GAMES"), RecalboxConf::Instance().GetShowHidden(), (int)Components::ShowHidden, this, _(MENUMESSAGE_UI_SHOW_HIDDEN_MSG));

  AddSwitch(_("HIDE ADULT GAMES"), RecalboxConf::Instance().GetFilterAdultGames(), (int)Components::Adult, this, _(MENUMESSAGE_UI_HIDE_ADULT_MSG));

  AddSwitch(_("HIDE PREINSTALLED GAMES"), RecalboxConf::Instance().GetGlobalHidePreinstalled(), (int)Components::HidePreinstalled, this, _(MENUMESSAGE_UI_HIDE_PREINSTALLED_MSG));

  AddSwitch(_("HIDE NO GAMES"), RecalboxConf::Instance().GetHideNoGames(), (int)Components::NoGames, this, _(MENUMESSAGE_UI_HIDE_NO_GAMES_MSG));
}

void GuiMenuGameFilters::SwitchComponentChanged(int id, bool& status)
{
  switch((Components)id)
  {
    case Components::ShowOnlyLatestVersion:
    {
      RecalboxConf::Instance().SetShowOnlyLatestVersion(status);
      if (mSystemManager.UpdatedTopLevelFilter())
        RecalboxConf::Instance().Save();
      else
      {
        mWindow.displayMessage(_("There is no game to show after this filter is changed! No change recorded."));
        RecalboxConf::Instance().SetShowOnlyLatestVersion(!status);
        status = !status;
      }
      break;
    }
    case Components::FavoritesOnly:
    {
      RecalboxConf::Instance().SetFavoritesOnly(status);
      if (mSystemManager.UpdatedTopLevelFilter())
        RecalboxConf::Instance().Save();
      else
      {
        mWindow.displayMessage(_("There is no favorite games in any system!"));
        RecalboxConf::Instance().SetFavoritesOnly(!status);
        status = !status;
      }
      break;
    }
    case Components::ShowHidden:
    {
      RecalboxConf::Instance().SetShowHidden(status);
      if (mSystemManager.UpdatedTopLevelFilter())
        RecalboxConf::Instance().Save();
      else
      {
        mWindow.displayMessage(_("There is no game to show after this filter is changed! No change recorded."));
        RecalboxConf::Instance().SetShowHidden(!status);
        status = !status;
      }
      break;
    }
    case Components::Adult:
    {
      RecalboxConf::Instance().SetFilterAdultGames(status);
      if (mSystemManager.UpdatedTopLevelFilter())
        RecalboxConf::Instance().Save();
      else
      {
        mWindow.displayMessage(_("There is no game to show after this filter is changed! No change recorded."));
        RecalboxConf::Instance().SetFilterAdultGames(!status);
        status = !status;
      }
      break;
    }
    case Components::HidePreinstalled:
    {
      RecalboxConf::Instance().SetGlobalHidePreinstalled(status);
      if (mSystemManager.UpdatedTopLevelFilter())
        RecalboxConf::Instance().Save();
      else
      {
        mWindow.displayMessage(_("There is no game to show after this filter is changed! No change recorded."));
        RecalboxConf::Instance().SetGlobalHidePreinstalled(!status);
        status = !status;
      }
      break;
    }
    case Components::NoGames:
    {
      RecalboxConf::Instance().SetHideNoGames(status);
      if (mSystemManager.UpdatedTopLevelFilter())
        RecalboxConf::Instance().Save();
      else
      {
        mWindow.displayMessage(_("There is no game to show after this filter is changed! No change recorded."));
        RecalboxConf::Instance().SetHideNoGames(!status);
        status = !status;
      }
      break;
    }
    default: break;
  }
}
