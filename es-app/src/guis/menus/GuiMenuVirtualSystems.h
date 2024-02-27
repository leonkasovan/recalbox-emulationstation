//
// Created by bkg2k on 08/01/2021.
//
// As part of the RECALBOX Project
// http://www.recalbox.com
//
#pragma once

#include <guis/menus/GuiMenuBase.h>

// Forward declaration
class SystemManager;
template<class T> class OptionListComponent;
class SwitchComponent;

class GuiMenuVirtualSystems : public GuiMenuBase
                            , private IGuiMenuBase
                            , private ISwitchComponent
{
  public:
    /*!
     * @brief Default constructor
     * @param window Global window
     * @param systemManager System manager reference
     */
    explicit GuiMenuVirtualSystems(WindowManager& window, SystemManager& systemManager);

  private:
    enum class Components
    {
      AllGames,
      Multiplayers,
      LastPlayed,
      VirtualPerGenre,
      LightGun,
      Ports,
    };

    //! System manager reference
    SystemManager& mSystemManager;

    /*
     * IGuiMenuBase implementation
     */

    void SubMenuSelected(int id) override;

    /*
     * ISwitchComponent implementation
     */

    void SwitchComponentChanged(int id, bool& status) override;
};



