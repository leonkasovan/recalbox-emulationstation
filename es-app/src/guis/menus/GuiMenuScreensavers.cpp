//
// Created by bkg2k on 31/10/2020.
//

#include <systems/SystemManager.h>
#include "GuiMenuScreensavers.h"
#include "guis/MenuMessages.h"

GuiMenuScreensavers::GuiMenuScreensavers(WindowManager& window, SystemManager& systemManager)
  : GuiMenuBase(window, _("SCREENSAVER"), nullptr)
  , mSystemManager(systemManager)
{
  // screensaver time
  AddSlider(_("SCREENSAVER AFTER"), 0.f, 30.f, 1.f, (float)RecalboxConf::Instance().GetScreenSaverTime(), "m", (int)Components::Time, this, _(MENUMESSAGE_UI_SCREENSAVER_AFTER_HELP_MSG));

  // screensaver behavior
  AddList<RecalboxConf::Screensaver>(_("SCREENSAVER BEHAVIOR"), (int)Components::Type, this, GetTypeEntries(), _(MENUMESSAGE_UI_SCREENSAVER_BEHAVIOR_HELP_MSG));

  // add systems
  AddMultiList(_("SYSTEMS TO SHOW IN DEMO"), (int)Components::SystemList, this, GetSystemEntries());
}

std::vector<GuiMenuBase::ListEntry<RecalboxConf::Screensaver>> GuiMenuScreensavers::GetTypeEntries()
{
  std::vector<ListEntry<RecalboxConf::Screensaver>> list;

  RecalboxConf::Screensaver type = RecalboxConf::Instance().GetScreenSaverType();
  if (Board::Instance().HasSuspendResume())
    list.push_back({ _("suspend"), RecalboxConf::Screensaver::Suspend, type == RecalboxConf::Screensaver::Suspend });
  list.push_back({ _("dim"), RecalboxConf::Screensaver::Dim, type == RecalboxConf::Screensaver::Dim });
  list.push_back({ _("black"), RecalboxConf::Screensaver::Black, type == RecalboxConf::Screensaver::Black });
  list.push_back({ _("demo"), RecalboxConf::Screensaver::Demo, type == RecalboxConf::Screensaver::Demo });
  list.push_back({ _("gameclip"), RecalboxConf::Screensaver::Gameclip, type == RecalboxConf::Screensaver::Gameclip });

  return list;
}

std::vector<GuiMenuBase::ListEntry<String>> GuiMenuScreensavers::GetSystemEntries()
{
  std::vector<ListEntry<String>> list;

  for (const SystemData* system : mSystemManager.AllSystems())
    if (system->HasGame())
      list.push_back({ system->FullName(), system->Name(), RecalboxConf::Instance().IsInScreenSaverSystemList(system->Name()) });

  return list;
}

void GuiMenuScreensavers::SliderMoved(int id, float value)
{
  if ((Components)id == Components::Time)
    RecalboxConf::Instance().SetScreenSaverTime((int)value).Save();
}

void GuiMenuScreensavers::OptionListComponentChanged(int id, int index, const RecalboxConf::Screensaver& value)
{
  (void)index;
  if ((Components)id == Components::Type) RecalboxConf::Instance().SetScreenSaverType(value).Save();
}

void GuiMenuScreensavers::OptionListMultiComponentChanged(int id, const String::List& value)
{
  if ((Components)id == Components::SystemList)
  {
    RecalboxConf::Instance().SetScreenSaverSystemList(value).Save();
  }
}

