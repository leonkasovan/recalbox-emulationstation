//
// Created by bkg2k on 29/05/23.
//

#pragma once

#include <guis/menus/GuiMenuBase.h>

class GuiMenuArcadeOptions : public GuiMenuBase
                           , private ISwitchComponent
{
  public:
    /*!
     * @brief Constructor
     */
    explicit GuiMenuArcadeOptions(WindowManager&window);

    ~GuiMenuArcadeOptions() override;

  private:
    enum class Components
    {
      EnhancedView,
      FoldClones,
      HideBios,
      HideNonWorking,
      UseDatabasesNames,
    };

    /*
     * ISwitchComponent implementation
     */

    void SwitchComponentChanged(int id, bool status) override;
};