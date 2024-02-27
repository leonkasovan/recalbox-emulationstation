
//
// Created by bkg2k on 15/06/23.
//
#pragma once

#include <WindowManager.h>
#include <guis/menus/GuiMenuBase.h>
#include "views/gamelist/IArcadeGamelistInterface.h"
#include "systems/SystemManager.h"

class GuiMenuArcade : public GuiMenuBase
                    , private IGuiMenuBase
                    , private IOptionListMultiComponent<String>
                    , private IOptionListMultiComponent<int>
                    , private ISwitchComponent
{
  public:
    /*!
     * @brief Constructor
     */
    explicit GuiMenuArcade(WindowManager& window, SystemManager& systemManager, IArcadeGamelistInterface* arcadeInterface);

  private:
    enum class Components
    {
      EnhancedView,
      FoldClones,
      HideBios,
      HideNonWorking,
      UseDatabasesNames,
      ManufacturersVirtual,
      ManufacturersFilter,
      GlobalArcadeSystem,
    };

    //! System manager reference
    SystemManager& mSystemManager;

    // IArcadeGamelistInterface for gamelist options
    IArcadeGamelistInterface* mArcade;

    // Manufacturer virtual system cached initial list
    String::List mManufacturersIdentifiers;

    //! Get virtual manufacturer/system entries
    std::vector<GuiMenuBase::ListEntry<String>> GetManufacturersVirtualEntries();

    //! Get filter by manufacturer/system entries
    std::vector<GuiMenuBase::ListEntry<int>> GetManufacturerFilterEntries();

    //! Format driver name
    String FormatManufacturer(const ArcadeDatabase::Manufacturer& driver);

    /*
     * ISwitchComponent implementation
     */

    void SwitchComponentChanged(int id, bool& status) override;

    /*
     * IOptionListComponent<String> implementation
     */

    void OptionListMultiComponentChanged(int id, const String::List& value) override;

    /*
     * IOptionListMultiComponent<int> implementation
     */

    void OptionListMultiComponentChanged(int id, const std::vector<int>& value) override;

    /*
     * IGuiMenuBase implementation
     */

    void SubMenuSelected(int id) override;
};
