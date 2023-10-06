//
// Created by bkg2k on 31/01/2022.
//
// As part of the RECALBOX Project
// http://www.recalbox.com
//

#include "GuiMenuResolutionByEmulator.h"
#include "guis/MenuMessages.h"
#include "ResolutionAdapter.h"
#include <systems/SystemManager.h>

GuiMenuResolutionByEmulator::GuiMenuResolutionByEmulator(WindowManager& window, SystemManager& systemManager)
  : GuiMenuBase(window, _("SYSTEM RESOLUTIONS"), nullptr)
  , mSystemManager(systemManager)
{
  // For each activated system
  const SystemManager::List& systems = systemManager.AllSystems();
  for(int i = 0; i < (int)systems.Count(); ++i)
    if (!systems[i]->IsVirtual())
      AddList<String>(systems[i]->FullName(), i, this, GetResolutionEntries(*systems[i]), _(MENUMESSAGE_ADVANCED_RESOLUTION_SYSTEM_HELP_MSG));
}

std::vector<GuiMenuBase::ListEntry<String>> GuiMenuResolutionByEmulator::GetResolutionEntries(SystemData& system)
{
  std::vector<GuiMenuBase::ListEntry<String>> result;
  result.push_back({ _("USE GLOBAL"), "", !RecalboxConf::Instance().IsDefinedSystemVideoMode(system) });
  result.push_back({ _("NATIVE"), "default", RecalboxConf::Instance().GetSystemVideoMode(system) == "default" });
  ResolutionAdapter resolutionAdapter;
  for(const ResolutionAdapter::Resolution& resolution : resolutionAdapter.Resolutions(false))
  {
    String reso = resolution.ToRawString();
    result.push_back({ resolution.ToString(), reso, reso == RecalboxConf::Instance().GetSystemVideoMode(system) });
  }
  return result;
}

void GuiMenuResolutionByEmulator::OptionListComponentChanged(int id, int index, const String& value)
{
  (void)index;
  const SystemManager::List& systems = mSystemManager.AllSystems();
  if (value.empty()) RecalboxConf::Instance().DeleteSystemVideoMode(*systems[id]).Save();
  else RecalboxConf::Instance().SetSystemVideoMode(*systems[id], value).Save();
}
