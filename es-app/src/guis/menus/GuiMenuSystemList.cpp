//
// Created by bkg2k on 04/01/2021.
//
// As part of the RECALBOX Project
// http://www.recalbox.com
//

#include "GuiMenuSystemList.h"
#include "GuiMenuSystemConfiguration.h"
#include <systems/SystemManager.h>

GuiMenuSystemList::GuiMenuSystemList(WindowManager& window, SystemManager& systemManager)
  : GuiMenuBase(window, _("ADVANCED EMULATOR CONFIGURATION"), this)
  , mSystemManager(systemManager)
{
  // For each activated system
  const SystemManager::List& systems = systemManager.AllSystems();
  for(int i = 0; i < (int)systems.Count(); ++i)
    if (!systems[i]->IsVirtual())
      mMenus[systems[i]] = AddSubMenu("", i);
}

void GuiMenuSystemList::SubMenuSelected(int id)
{
  SystemData& system = *mSystemManager.AllSystems()[id];
  GuiMenuSystemConfiguration::AdvancedMenuOptions options = GuiMenuSystemConfiguration::allOptions;
  if (Board::Instance().CrtBoard().IsCrtAdapterAttached())
    options = {.emulator = true, .ratio=false, .smooth=false, .rewind=true, .autosave=true, .shaders=false, .shaderSet=false};
  mWindow.pushGui(new GuiMenuSystemConfiguration(mWindow, system, mSystemManager, options));
}

void GuiMenuSystemList::onShow()
{
  for(auto& kv : mMenus)
  {
    String emulator;
    String core;
    String name(kv.first->FullName());
    if (!Renderer::Instance().Is480pOrLower())
    {
      name.UpperCase();
      EmulatorManager::GetSystemEmulator(*kv.first, emulator, core);
      if (!emulator.empty())
      {
        name.Append(" - ").Append(emulator);
        if (emulator != core) name.Append(' ').Append(core);
      }
    }
    kv.second->setText(name);
  }
}
