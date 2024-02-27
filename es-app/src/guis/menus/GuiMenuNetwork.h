//
// Created by bkg2k on 26/10/2020.
//
#pragma once

#include <guis/menus/GuiMenuBase.h>
#include <components/MenuComponent.h>
#include <components/IEditableComponent.h>
#include <components/OptionListComponent.h>
#include <guis/GuiWaitLongExecution.h>
#include <guis/IGuiArcadeVirtualKeyboardInterface.h>

enum class NetworkOperation
{
  StartWIFI,
  StopWIFI,
  NewConnection,
  ScanSSID,
  WPS,
};

class GuiMenuNetwork : public GuiMenuBase
                     , private ILongExecution<NetworkOperation, bool>
                     , private IGuiArcadeVirtualKeyboardInterface
                     , private IOptionListComponent<String>
                     , private IEditableComponent
                     , private ISwitchComponent
{
  public:
    /*!
     * @brief Default constructor
     * @param window Global window
     */
    explicit GuiMenuNetwork(WindowManager& window);

  private:
    enum class Components
    {
      WifiOnOff,
      WifiSSID,
      WifiKey,
      Hostname,
    };

    //! SSID list
    std::shared_ptr<OptionListComponent<String>> mSSIDList;
    //! Hostname
    std::shared_ptr<EditableComponent> mHostname;
    //! WIFI On/Off
    std::shared_ptr<SwitchComponent> mWifiOnOff;
    //! WIFI Key
    std::shared_ptr<EditableComponent> mWifiKey;
    //! Status
    std::shared_ptr<TextComponent> mStatus;
    //! IP
    std::shared_ptr<TextComponent> mIP;

    //! Edited text Backup
    String mBackupedText;

    //! Captured WPS SSID
    String mWpsSSID;
    //! Captured WPS PSK
    String mWpsPSK;

    //! Last SSID Scan
    String::List mScannedSSIDList;

    //! Anti-renentry flag
    bool mFillingList;

    /*!
     * @brief Try WPS connection
     */
    void TryWPS();

    /*!
     * @brief Connect using WPS
     * @param from Current Operation
     * @return True if the WPS connection is successful
     */
    bool ConnectWps(GuiWaitLongExecution<NetworkOperation, bool>& from);

    /*!
     * @brief Set WIFI ssid
     * @param ssid new wifi ssid
     * @param save True to save configuration immediately
     */
    static void SetWifiSSID(const String& ssid, bool save)
    {
      RecalboxConf::Instance().SetWifiSSID(ssid);
      if (save) RecalboxConf::Instance().Save();
    }

    /*
     * ILongExecution implementation
     */

    /*!
     * @brief Execture network operation
     * @param parameter Network operation required to execute
     * @return True if the operation was successful
     */
    bool Execute(GuiWaitLongExecution<NetworkOperation, bool>& from, const NetworkOperation& parameter) final;

    /*!
     * @brief Receive the status of network operations
     * @param parameter original input parameter
     * @param result Result state
     */
    void Completed(const NetworkOperation& parameter, const bool& result) final;

    /*
     * IOptionListComponent implementation
     */

    void OptionListComponentChanged(int id, int index, const String& value) override;

    /*
     * IEditableComponent implementation
     */

    void EditableComponentTextChanged(int id, const String& text) override;

    /*
     * ISwitchComponent implementation
     */

    void SwitchComponentChanged(int id, bool& status) override;

    /*
     * IGuiArcadeVirtualKeyboardInterface implementation
     */

    void ArcadeVirtualKeyboardTextChange(GuiArcadeVirtualKeyboard&, const String&) override {}

    void ArcadeVirtualKeyboardValidated(GuiArcadeVirtualKeyboard& vk, const String& text) override;

    void ArcadeVirtualKeyboardCanceled(GuiArcadeVirtualKeyboard&) override {}
};

