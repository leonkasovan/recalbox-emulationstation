//
// Created by bkg2k on 08/01/2021.
//
// As part of the RECALBOX Project
// http://www.recalbox.com
//
#pragma once

#include <guis/menus/GuiMenuBase.h>
#include "hardware/devices/storage/StorageDevices.h"

// Forward declaration
template<class T> class OptionListComponent;
class SwitchComponent;
class TextComponent;
class SystemManager;

class GuiMenuSystem : public GuiMenuBase
                    , private IOptionListComponent<String>
                    , private IOptionListComponent<StorageDevices::Device>
                    , private IGuiMenuBase
{
  public:
    /*!
     * @brief Default constructor
     * @param window Global window
     * @param systemManager System manager
     */
    explicit GuiMenuSystem(WindowManager& window, SystemManager& systemManager);

    //! Destructeur
    ~GuiMenuSystem() override;

  private:
    enum class Components
    {
      DiskUsage,
      Storage,
      Culture,
      Keyboard,
    };

    //! Kayboard name
    struct Keyboard
    {
      String Displayable; //!< Kayboard displayable name
      String Identifier;  //!< Keyboard identifier
    };
    //! Keyboard list
    typedef std::vector<Keyboard> KeyboardList;

    //! Culture structure
    struct Culture
    {
      String Displayable; //!< Language name
      String Identifier;  //!< Culture identifier
    };
    //! Culture list
    typedef std::vector<Culture> CultureList;

    //! System manager reference
    SystemManager& mSystemManager;

    //! Storage devices
    StorageDevices mStorageDevices;

    //! Share Free Space
    std::shared_ptr<TextComponent> mFreeSpace;
    //! Available storages
    std::shared_ptr<OptionListComponent<StorageDevices::Device>> mStorages;
    //! Culture
    std::shared_ptr<OptionListComponent<String>> mCulture;
    //! Keyboard
    std::shared_ptr<OptionListComponent<String>> mKeyboard;

    //! Original storage
    String mOriginalStorage;
    //! Original culture
    String mOriginalCulture;
    //! Original keyboard
    String mOriginalKeyboard;

    //! Get Storage List
    std::vector<ListEntry<StorageDevices::Device>> GetStorageEntries();
    //! Get Culture List
    std::vector<ListEntry<String>> GetCultureEntries();
    //! Get Keyboard List
    std::vector<ListEntry<String>> GetKeyboardEntries();

    /*
     * IOptionListComponent<String> implementation
     */

    void OptionListComponentChanged(int id, int index, const String& value) override;

    /*
     * IOptionListComponent<StorageDevices::Device> implementation
     */

    void OptionListComponentChanged(int id, int index, const StorageDevices::Device& value) override;

    /*
     * IGuiMenuBase implementation
     */

    void SubMenuSelected(int id) override;
};



