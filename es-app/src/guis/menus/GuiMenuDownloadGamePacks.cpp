//
// Created by bkg2k on 05/05/23.
//

#include "GuiMenuDownloadGamePacks.h"
#include "systems/DownloaderManager.h"
#include "guis/GuiDownloader.h"
#include "systems/SystemDeserializer.h"
#include <systems/SystemManager.h>

GuiMenuDownloadGamePacks::GuiMenuDownloadGamePacks(WindowManager& window, SystemManager& systemManager)
  : GuiMenuBase(window, _("DOWNLOAD CONTENTS"), this)
  , mSystemManager(systemManager)
{
  SystemDeserializer deserializer;
  bool loaded = deserializer.LoadSystems();

  if (loaded)
    for (int index = 0; index < deserializer.Count(); ++index)
      if (SystemDescriptor descriptor; deserializer.Deserialize(index, descriptor))
        if (descriptor.HasDownloader())
        {
          AddSubMenu(descriptor.FullName(), (int)mDescriptors.size());
          mDescriptors.push_back(descriptor);
        }
}

void GuiMenuDownloadGamePacks::SubMenuSelected(int id)
{
  SystemData& target = mSystemManager.GetOrCreateSystem(mDescriptors[id]);
  mWindow.pushGui(new GuiDownloader(mWindow, target, mSystemManager));
}
