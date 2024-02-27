//
// Created by bkg2k on 05/05/23.
//
#pragma once

#include <guis/menus/GuiMenuBase.h>
#include <components/SwitchComponent.h>

// Forward declarations
class SystemManager;

class GuiMenuDownloadContents: public GuiMenuBase
                             , private IGuiMenuBase
{
  public:
    //! Constructor
    GuiMenuDownloadContents(WindowManager& window, SystemManager& systemManager);

  private:
    enum class Components
    {
      Games,
      Themes,
      Overlay,
      Joke,
    };

    //! System manager reference
    SystemManager& mSystemManager;

    /*
     * IGuiMenuBase implementation
     */

    void SubMenuSelected(int id) override;
};
