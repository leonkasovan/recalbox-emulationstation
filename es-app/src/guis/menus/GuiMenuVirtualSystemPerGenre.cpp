//
// Created by bkg2k on 08/01/2021.
//
// As part of the RECALBOX Project
// http://www.recalbox.com
//

#include <guis/menus/GuiMenuVirtualSystemPerGenre.h>
#include <components/SwitchComponent.h>
#include <systems/SystemManager.h>
#include <utils/locale/LocaleHelper.h>
#include <guis/GuiMsgBox.h>
#include <MainRunner.h>

GuiMenuVirtualSystemPerGenre::GuiMenuVirtualSystemPerGenre(WindowManager& window)
  : GuiMenuBase(window, _("VIRTUAL SYSTEMS PER GENRE"), nullptr)
{
  // All games
  for(const auto & genre : Genres::GetOrderedList())
  {
    String shortName = Genres::GetShortName(genre);
    String longName = Genres::GetFullName(genre);
    String prefix = Genres::IsSubGenre(genre) ? "   \u21B3 " : "";
    Path icon = Path(Genres::GetResourcePath(genre));
    bool value = RecalboxConf::Instance().GetCollection(shortName);
    auto component = AddSwitch(icon, prefix + _S(longName), value, 0, nullptr, _S(longName));
    mComponents[component] = shortName;
  }
}

GuiMenuVirtualSystemPerGenre::~GuiMenuVirtualSystemPerGenre()
{
  bool relaunch = false;
  for(const auto& component : mComponents)
    if (component.first->getState() != RecalboxConf::Instance().GetCollection(component.second))
    {
      RecalboxConf::Instance().SetCollection(component.second, component.first->getState());
      relaunch = true;
    }
  if (relaunch) RequestRelaunch();
}

