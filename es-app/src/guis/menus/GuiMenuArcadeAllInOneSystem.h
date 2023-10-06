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
    explicit GuiMenuArcadeAllInOneSystem(WindowManager& window);

    //! Destructor
    ~GuiMenuArcadeAllInOneSystem() override;

  private:
    enum class Components
    {
      ArcadeOnOff,
      IncludeNeogeo,
      HideOriginals,
    };

    //! Original manufacturer list
    String mOriginalManufacturerList;
    //! Original Arcade ON/OFF value
    bool mOriginalArcadeOnOff;
    //! Original Include neogeo value
    bool mOriginalIncludeNeogeo;
    //! Original Hide Original value
    bool mOriginalHideOriginals;


    /*
     * ISwitchComponent implementation
     */

    void SwitchComponentChanged(int id, bool status) override;

};
