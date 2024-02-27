#pragma once

#include <guis/menus/GuiMenuBase.h>

template<typename T>
class OptionListComponent;
class SwitchComponent;

class GuiMenuArcadeAllInOneSystem : public GuiMenuBase
                                  , private ISwitchComponent
{
  public:
    //! Constructor
    explicit GuiMenuArcadeAllInOneSystem(WindowManager& window, SystemManager& systemManager);

  private:
    enum class Components
    {
      ArcadeOnOff,
      IncludeNeogeo,
      HideOriginals,
    };

    //! System manager reference
    SystemManager& mSystemManager;

    /*
     * ISwitchComponent implementation
     */

    void SwitchComponentChanged(int id, bool& status) override;
};
