//
// Created by bkg2k on 05/05/23.
//

#include "GuiMenuDownloadContents.h"
#include "GuiMenuDownloadGamePacks.h"
#include <systems/SystemManager.h>

GuiMenuDownloadContents::GuiMenuDownloadContents(WindowManager& window, SystemManager& systemManager)
  : GuiMenuBase(window, _("DOWNLOAD CONTENTS"), this)
  , mSystemManager(systemManager)
{
  AddSubMenu("Free game packs", (int)Components::Games);
  AddSubMenu("Overlays", (int)Components::Overlay);
  AddSubMenu("Themes", (int)Components::Themes);
  AddSubMenu("Fullpack: 200000 Games + BIOS!", (int)Components::Joke);
}

void GuiMenuDownloadContents::SubMenuSelected(int id)
{
  switch((Components)id)
  {
    case Components::Games: mWindow.pushGui(new GuiMenuDownloadGamePacks(mWindow, mSystemManager)); break;
    case Components::Themes:
    case Components::Overlay:
    {
      mWindow.displayMessage("Not yet available.");
      break;
    }
    case Components::Joke:
    {
      mWindow.displayMessage("HAHAHAHAHAHAHAHAHAHAHAHAHA! Did you really believe it?\n\nAll your personnal information, your name, your address and your IP have been sent to the FBI.");
      break;
    }
    default: break;
  }
}
