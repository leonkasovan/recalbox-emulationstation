//
// Created by thierry.imbert on 12/02/2020.
//
#pragma once

#include <guis/menus/GuiMenuBase.h>

class GuiMenuQuit : public GuiMenuBase
                  , private IGuiMenuBase
{
  public:
    explicit GuiMenuQuit(WindowManager& window);

    static void PushQuitGui(WindowManager& window);

  private:
    enum class Components
    {
      Shutdown,
      FastShutdown,
      Reboot,
      QuitGui,
    };

    std::vector<String> mMountPoints;

    /*
     * IGuiMenuBase implementation
     */

    void SubMenuSelected(int id) override;
};
