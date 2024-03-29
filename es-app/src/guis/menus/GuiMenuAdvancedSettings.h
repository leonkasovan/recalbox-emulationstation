//
// Created by bkg2k on 11/01/2021.
//
// As part of the RECALBOX Project
// http://www.recalbox.com
//
#pragma once

#include <guis/menus/GuiMenuBase.h>
#include "ResolutionAdapter.h"
#include "hardware/Case.h"

// Forward declaration
class SystemManager;
class SystemData;
template<class T> class OptionListComponent;
class SwitchComponent;

//! O/C file & description
struct Overclocking
{
  String File;
  String Description;
  bool Hazardous;
  int Frequency;

  bool operator == (const Overclocking& r) const { return (File == r.File) && (Description == r.Description) && (Hazardous == r.Hazardous) && (Frequency == r.Frequency); }
};

class GuiMenuAdvancedSettings : public GuiMenuBase
                              , private IOptionListComponent<Overclocking>
                              , private ISwitchComponent
                              , private IGuiMenuBase
                              , private IOptionListComponent<String>
{
  public:
    /*!
     * @brief Default constructor
     * @param window Global window
     */
    explicit GuiMenuAdvancedSettings(WindowManager& window, SystemManager& systemManager);

    //! Destructor
    ~GuiMenuAdvancedSettings() override;

  private:
    enum class Components
    {
      DebugLogs,
      OverclockList,
      BootSubMenu,
      VirtualSubMenu,
      ResolutionSubMenu,
      AdvancedSubMenu,
      KodiSubMenu,
      Cases,
      SecuritySubMenu,
      Overscan,
      ShowFPS,
      AutorunEnabled,
      CrtSubMenu,
      Manager,
      FactoryReset,
      EepromUpdate
    };

    static constexpr const char* sOverclockBaseFolder = "/recalbox/system/configs/overclocking";
    static constexpr const char* sOverclockFile = "/boot/recalbox-oc-config.txt";

    //! Overclocking list
    typedef std::vector<Overclocking> OverclockList;

    //! System Manager
    SystemManager& mSystemManager;

    // Resolution Adapter
    ResolutionAdapter mResolutionAdapter;

    //! Default overclock value
    Overclocking mDefaultOverclock;
    //! Original overclock value
    String mOriginalOverclock;
    //! Last overclock hazardous?
    bool mLastHazardous;
    //! Is there at least a valid O/C?
    bool mValidOverclock;

    //! Overclock
    std::shared_ptr<OptionListComponent<Overclocking>> mOverclock;

    //! Get O/C list for the current board
    static OverclockList AvailableOverclocks();

    //! Get O/C List
    std::vector<ListEntry<Overclocking>> GetOverclockEntries();

    //! Reset overclock
    void ResetOverclock();

    //! Get supported cases List
    static std::vector<GuiMenuBase::ListEntry<String>> GetCasesEntries();

    //! Reset Factory requested Level 1
    void ResetFactory();

    //! Reset Factory requested Level 2
    static void ResetFactoryReally(WindowManager* This);

    //! Do Reset Factory
    static void DoResetFactory();

    /*
     * IOptionListComponent<Overclocking> implementation
     */

    void OptionListComponentChanged(int id, int index, const Overclocking& value) override;

    //! Case component management
    void OptionListComponentChanged(int id, int index, const String& value) override;

    /*
     * ISwitchComponent implementation
     */

    void SwitchComponentChanged(int id, bool& status) override;

    /*
     * IGuiMenuBase implementation
     */

    void SubMenuSelected(int id) override;

    void EepromUpdate();

    String ExtractVersion(String cmdResult, String updateType);
};



