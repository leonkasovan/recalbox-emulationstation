//
// Created by bkg2k on 04/01/2021.
//
// As part of the RECALBOX Project
// http://www.recalbox.com
//
#pragma once

#include <guis/menus/GuiMenuBase.h>

// Forward declaration
class SystemManager;
class SystemData;
template<class T> class OptionListComponent;
class SwitchComponent;

class GuiMenuSystemConfiguration : public GuiMenuBase
                                 , private IOptionListComponent<String>
                                 , private ISwitchComponent
{
  public:
    struct AdvancedMenuOptions {
      bool emulator, ratio, smooth, rewind, autosave, shaders, shaderSet;
    };

    constexpr const static AdvancedMenuOptions allOptions =
        {.emulator = true, .ratio=true, .smooth=true, .rewind=true, .autosave=true, .shaders=true, .shaderSet=true};

  /*!
     * @brief Default constructor
     * @param window Global window
     */
    explicit GuiMenuSystemConfiguration(WindowManager& window, SystemData& system, SystemManager& systemManager, AdvancedMenuOptions options = allOptions);

  private:
    enum class Components
    {
      Emulator,
      Ratio,
      Smooth,
      Rewind,
      AutoSave,
      Shaders,
      ShaderSet,
    };

    //! System manager
    SystemManager& mSystemManager;
    //! Target system
    SystemData& mSystem;
    //! Default emulator
    String mDefaultEmulator;
    //! Default core
    String mDefaultCore;

    //! Get Emulator List
    std::vector<ListEntry<String>> GetEmulatorEntries();
    //! Get Ratio List
    std::vector<ListEntry<String>> GetRatioEntries();
    //! Get Shaders List
    std::vector<ListEntry<String>> GetShadersEntries();
    //! Get ShaderSet List
    std::vector<ListEntry<String>> GetShaderSetEntries();

    /*
     * IOptionListComponent<String> implementation
     */

    void OptionListComponentChanged(int id, int index, const String& value) override;

    /*
     * ISwitchComponent implementation
     */

    void SwitchComponentChanged(int id, bool& status) override;
};



