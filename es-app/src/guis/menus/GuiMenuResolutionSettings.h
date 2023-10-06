//
// Created by bkg2k on 31/01/2022.
//
// As part of the RECALBOX Project
// http://www.recalbox.com
//
#pragma once

#include "GuiMenuBase.h"
#include "ResolutionAdapter.h"

class SystemManager;

class GuiMenuResolutionSettings : public GuiMenuBase
  , private IOptionListComponent<String>
  , private IGuiMenuBase
{
  public:
    /*!
     * @brief Default constructor
     * @param window Global window
     */
    explicit GuiMenuResolutionSettings(WindowManager& window, SystemManager& systemManager);

  private:
    enum class Components
    {
      GlobalResolution,
      FrontendResolution,
      Emulators,
    };

    //! System manager reference
    SystemManager& mSystemManager;

    // Resolution Adapter
    ResolutionAdapter mResolutionAdapter;

    std::vector<ListEntry<String>> GetGlobalResolutionEntries();
    std::vector<ListEntry<String>> GetFrontEndResolutionEntries();

    /*
     * IOptionListComponent<Overclocking> implementation
     */

    void OptionListComponentChanged(int id, int index, const String& value) override;

    /*
     * IGuiMenuBase implementation
     */

    void SubMenuSelected(int id) override;
};



