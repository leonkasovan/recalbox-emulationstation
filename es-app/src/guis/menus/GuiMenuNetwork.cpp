//
// Created by bkg2k on 26/10/2020.
//

#include <recalbox/RecalboxSystem.h>
#include <utils/locale/LocaleHelper.h>
#include <components/SwitchComponent.h>
#include <components/EditableComponent.h>
#include "GuiMenuNetwork.h"
#include "guis/MenuMessages.h"
#include <guis/GuiArcadeVirtualKeyboard.h>

GuiMenuNetwork::GuiMenuNetwork(WindowManager& window)
  : GuiMenuBase(window, _("NETWORK SETTINGS"), nullptr)
  , mFillingList(false)
{
  // Network status
  mStatus = AddText(_("STATUS"), String::Empty, _(MENUMESSAGE_NETWORK_STATUS_HELP_MSG));

  // IP
  mIP = AddText(_("IP ADDRESS"), String::Empty, _(MENUMESSAGE_NETWORK_IP_HELP_MSG));

  // WIFI ON/OFF
  mWifiOnOff = AddSwitch(_("ENABLE WIFI"), RecalboxConf::Instance().GetWifiEnabled(), (int)Components::WifiKey, this, _(MENUMESSAGE_NETWORK_WIFI_HELP_MSG));

  // SSID
  mSSIDList = AddList<String>(_("WIFI SSID"), (int)Components::WifiSSID, this, _(MENUMESSAGE_NETWORK_KEY_HELP_MSG));

  // Password
  mWifiKey = AddEditable(_("WIFI KEY"), RecalboxConf::Instance().GetWifiKey(), (int)Components::WifiKey, this, _(MENUMESSAGE_NETWORK_KEY_HELP_MSG), true);

  // Hostname
  mHostname = AddEditable(_("HOSTNAME"), RecalboxConf::Instance().GetHostname(), (int)Components::Hostname, this, _(MENUMESSAGE_NETWORK_HOST_HELP_MSG), false);

  // Buttons
  mMenu.addButton(_("WPS CONNECTION"), _("AUTOMATIC WPS CONNECTION"), std::bind(&GuiMenuNetwork::TryWPS, this) );

  // Set current SSID
  Completed(NetworkOperation::ScanSSID, true);
}

void GuiMenuNetwork::OptionListComponentChanged(int id, int index, const String& value)
{
  if (mFillingList) return;
    if ((Components)id == Components::WifiSSID)
    {
      if (index == 0)
      {
        mBackupedText = RecalboxConf::Instance().GetWifiSSID();
        mWindow.pushGui(new GuiArcadeVirtualKeyboard(mWindow, _("WIFI SSID"), mBackupedText, this));
      }
      else
      {
        // Store new SSID
        SetWifiSSID(value, true);

        // Reconnext using new parameters
        if (mWifiOnOff->getState())
          mWindow.pushGui(
            (new GuiWaitLongExecution<NetworkOperation, bool>(mWindow, *this))->Execute(NetworkOperation::NewConnection,
                                                                                        _("Connecting to WIFI...")));
      }
    }
}

void GuiMenuNetwork::TryWPS()
{
  mWpsSSID.clear();
  mWpsPSK.clear();
  mWindow.pushGui((new GuiWaitLongExecution<NetworkOperation, bool>(mWindow, *this))->Execute(NetworkOperation::WPS,""));
}

bool GuiMenuNetwork::ConnectWps(GuiWaitLongExecution<NetworkOperation, bool>& from)
{
  { LOG(LogDebug) << "[WPS] Start WPS connection"; }
  from.SetText(_("Reseting WIFI configuration..."));
  { LOG(LogDebug) << "[WPS] " << "Reseting WIFI configuration..."; }
  if (!RecalboxSystem::enableWifi("", "")) return false;
  from.SetText(_("Waiting for WPS configuration..."));
  { LOG(LogDebug) << "[WPS] " << "Waiting for WPS configuration..."; }
  if (!RecalboxSystem::getWifiWps()) return false;
  Thread::Sleep(2000); // Give time to read the message :)
  String ip = _("Waiting for IP address... (%is)");
  String ipResult;
  for(int i = 30; --i >= 0; )
  {
    String finalText(ip);
    finalText.Replace("%i", String(i));
    from.SetText(finalText);
    { LOG(LogDebug) << "[WPS] " << finalText; }
    if ((RecalboxSystem::getIpV4WirelessAddress(ipResult) ||
       ((i < 5) && RecalboxSystem::getIpV6WirelessAddress(ipResult))) &&
       (RecalboxSystem::hasIpAdress(true)))
      break;
    Thread::Sleep(1000);
  }
  if (!RecalboxSystem::hasIpAdress(true)) return false;
  from.SetText(_("Saving WIFI configuration"));
  { LOG(LogDebug) << "[WPS] " << "Saving WIFI configuration"; }
  if (!RecalboxSystem::saveWifiWps()) return false;
  Thread::Sleep(2000); // Give time to read the message :)
  from.SetText(_("Fetching WIFI parameters"));
  { LOG(LogDebug) << "[WPS] " << "Fetching WIFI parameters"; }
  if (!RecalboxSystem::getWifiConfiguration(mWpsSSID, mWpsPSK)) return false;
  Thread::Sleep(2000); // Give time to read the message :)

  { LOG(LogInfo) << "[WPS] WPS Configuration OK!"; }
  return true;
}

bool GuiMenuNetwork::Execute(GuiWaitLongExecution<NetworkOperation, bool>& from, const NetworkOperation& parameter)
{
  switch(parameter)
  {
    case NetworkOperation::StartWIFI:
    {
      { LOG(LogDebug) << "[Network] Starting WIFI using current configuration"; }
      if (!RecalboxSystem::enableWifi("", "")) return false;
      from.SetText(_("Scanning WIFI networks..."));
      mScannedSSIDList = RecalboxSystem::getAvailableWiFiSSID(mWifiOnOff->getState());
      return true;
    }
    case NetworkOperation::StopWIFI: return RecalboxSystem::disableWifi();
    case NetworkOperation::NewConnection: return RecalboxSystem::enableWifi(RecalboxConf::Instance().GetWifiSSID(), RecalboxConf::Instance().GetWifiKey());
    case NetworkOperation::ScanSSID:
    {
      mScannedSSIDList = RecalboxSystem::getAvailableWiFiSSID(mWifiOnOff->getState());
      return true;
    }
    case NetworkOperation::WPS:
    {
      return ConnectWps(from);
    }
    default: break;
  }

  return false;
}

void GuiMenuNetwork::Completed(const NetworkOperation& parameter, const bool& result)
{
  switch(parameter)
  {
    case NetworkOperation::StartWIFI:
    {
      if (!result)
        mWindow.displayMessage(_("WIFI CONFIGURATION ERROR"));
      Completed(NetworkOperation::ScanSSID, result);
      break;
    }
    case NetworkOperation::StopWIFI: break;
    case NetworkOperation::NewConnection:
    {
      mWindow.displayMessage(result ? _("WIFI ENABLED") : _("WIFI CONFIGURATION ERROR"));
      break;
    }
    case NetworkOperation::ScanSSID:
    {
      // Kill notifier
      mFillingList = true;

      // Fill SSID list
      String currentSSID = RecalboxConf::Instance().GetWifiSSID();
      String currentSSIDView = currentSSID.empty() ? _("EDIT MANUALLY") : currentSSID.Append(" (").Append(_("EDIT MANUALLY")).Append(')');
      bool found = false;
      for(const String& ssid : mScannedSSIDList)
        if (ssid == currentSSID) { found = true; break; }
      mSSIDList->clear();
      mSSIDList->add(currentSSIDView, currentSSID, !found);
      for(const String& ssid : mScannedSSIDList)
        mSSIDList->add(ssid, ssid, currentSSID == ssid);

      // Set back the change notifier
      mFillingList = false;
      break;
    }
    case NetworkOperation::WPS:
    {
      if (mWpsPSK.empty() || mWpsSSID.empty())
      {
        mWindow.displayMessage(_("NO WPS CONFIGURATION FOUND!"));
      }
      else
      {
        // Save new ssid/password
        RecalboxConf::Instance().SetWifiSSID(mWpsSSID).Save();
        RecalboxConf::Instance().SetWifiKey(mWpsPSK).Save();
        // Refresh SSID List
        Completed(NetworkOperation::ScanSSID, true);
        // Refresh psk
        mWifiKey->setText(mWpsPSK);

        mWindow.displayMessage(_("WPS CONFIGURATION SUCCESSFUL!"));
      }
      break;
    }
    default: break;
  }

  // Update status & IP
  mStatus->setText(RecalboxSystem::ping() ? _( "CONNECTED") : _("NOT CONNECTED"));
  mIP->setText(RecalboxSystem::getIpAddress());
  mMenu.onSizeChanged();
}

void GuiMenuNetwork::EditableComponentTextChanged(int id, const String& text)
{
  if ((Components)id == Components::WifiKey)
  {
    // Save
    RecalboxConf::Instance().SetWifiKey(text).Save();
    // Reconnect using new parameters
    if (mWifiOnOff->getState())
      mWindow.pushGui(
        (new GuiWaitLongExecution<NetworkOperation, bool>(mWindow, *this))->Execute(NetworkOperation::NewConnection,
                                                                                    _("Connecting to WIFI...")));
  }
  else if ((Components)id == Components::Hostname)
  {
    RecalboxConf::Instance().SetHostname(text).Save();
    mWindow.displayMessage(_("Hostname changes will not be effective until next reboot"));
  }
}

void GuiMenuNetwork::SwitchComponentChanged(int id, bool& status)
{
  (void)id;
  // Save state
  RecalboxConf::Instance().SetWifiEnabled(status).Save();
  // Connect or disconnect
  NetworkOperation operation = mWifiOnOff->getState() ? NetworkOperation::StartWIFI : NetworkOperation::StopWIFI;
  String text = mWifiOnOff->getState() ? _("Connecting to WIFI...") : _("Disconnecting from WIFI...");
  mWindow.pushGui((new GuiWaitLongExecution<NetworkOperation, bool>(mWindow, *this))->Execute(operation, text));
}

void GuiMenuNetwork::ArcadeVirtualKeyboardValidated(GuiArcadeVirtualKeyboard& vk, const String& text)
{
  (void)vk;
  // Save new SSID
  RecalboxConf::Instance().SetWifiSSID(text).Save();
  // Refresh SSID List
  Completed(NetworkOperation::ScanSSID, true);
  // Reconnext using new parameters
  if (mWifiOnOff->getState())
    mWindow.pushGui((new GuiWaitLongExecution<NetworkOperation, bool>(mWindow, *this))->Execute(NetworkOperation::NewConnection,_("Connecting to WIFI...")));
}


