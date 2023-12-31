//
// Created by bkg2k on 31/01/2022.
//
// As part of the RECALBOX Project
// http://www.recalbox.com
//
#pragma once

#include "GuiMenuBase.h"

class SystemManager;

class GuiMenuResolutionByEmulator : public GuiMenuBase
                                  , private IOptionListComponent<String>
{
  public:
    /*!
     * @brief Default constructor
     * @param window Global window
     */
    explicit GuiMenuResolutionByEmulator(WindowManager& window, SystemManager& systemManager);

  private:
    //! System manager reference
    SystemManager& mSystemManager;

    static std::vector<ListEntry<String>> GetResolutionEntries(SystemData& system);

    /*
     * IOptionListComponent<Overclocking> implementation
     */

    void OptionListComponentChanged(int id, int index, const String& value) override;
};



