#pragma once

#include <utils/String.h>
#include "WindowManager.h"
#include "games/FileData.h"

struct EmulatorDefaults
{
  String core;
  String emulator;
};

/*!
 * @brief Interface between EmulationStation and the Recalbox System
 */
class RecalboxSystem
{
  public:
    static bool MakeBootReadOnly();

    static bool MakeBootReadWrite();

    static long long GetMinimumFreeSpaceOnSharePartition() { return 3LL << 30; } // 3Gb

    static unsigned long long getFreeSpace(const String& mountpoint);

    static unsigned long getFreeSpaceGB(const String& mountpoint);

    static String getFreeSpaceInfo();

    static bool isFreeSpaceLimit();

    static bool isFreeSpaceUnderLimit(long long size);

    static String getRootPassword();

    static std::vector<String> getAvailableWiFiSSID(bool activateWifi);

    static bool getWifiWps();

    static bool saveWifiWps();

    static bool getWifiConfiguration(String& ssid, String& psk);

    static bool setOverscan(bool enable);

    static bool setOverclock(const String& mode);

    static bool ping();

    static bool kodiExists();

    static bool backupRecalboxConf();

    static bool enableWifi(String ssid, String key);

    static bool disableWifi();

    /*!
     * @brief Chech if the interface has a valid IP
     * @param onlyWIFI false = all interface, true = wlan0 only
     * @return True if the interface has a valid IP
     */
    static bool hasIpAdress(bool onlyWIFI);

    static String getIpAddress();

    static bool getIpV4Address(String& result);

    static bool getIpV4WirelessAddress(String& result);

    static bool getIpV6Address(String& result);

    static bool getIpV6WirelessAddress(String& result);

    static std::vector<String> scanBluetooth();

    static bool pairBluetooth(const String& basic_string);

    static std::vector<String> getAvailableStorageDevices();

    static String getCurrentStorage();

    static bool setStorage(const String& basic_string);

    static bool forgetBluetoothControllers();

    static std::pair<String, int> execute(const String& command);

    static std::pair<String, int> getSDLBatteryInfo();

    static bool getSysBatteryInfo(int& charge, int& unicodeIcon);

    static bool IsLiteVersion();

  private:
    //! Share path
    static constexpr const char* sSharePath = "/recalbox/share/";
    static constexpr const char* sConfigScript = "/recalbox/scripts/recalbox-config.sh";
    static constexpr const char* sLiteFlagFile = "/recalbox/recalbox.lite";
    static constexpr const char* sLiteFlagTrackFile = "/overlay/recalbox.lite";

    static String BuildSettingsCommand(const String& arguments);

    static String::List ExecuteSettingsCommand(const String& arguments);
};

