/*
 * File:   RecalboxSystem.cpp
 * Author: digitallumberjack
 *
 * Created on 29 novembre 2014, 03:1
 */

#include "RecalboxSystem.h"
#include <sys/statvfs.h>
#include "audio/AudioManager.h"

#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <MainRunner.h>
#include <Upgrade.h>
#include <input/InputMapper.h>

String RecalboxSystem::BuildSettingsCommand(const String& arguments)
{
  String result = sConfigScript;
  result.Append(' ');
  result.Append(arguments);
  return result;
}

String::List RecalboxSystem::ExecuteSettingsCommand(const String& arguments)
{
  String output;
  char buffer[1024];
  FILE* pipe = popen(BuildSettingsCommand(arguments).c_str(), "r");
  if (pipe != nullptr)
  {
    while (feof(pipe) == 0)
      if (fgets(buffer, sizeof(buffer), pipe) != nullptr)
        output.Append(buffer);
    pclose(pipe);
  }
  return output.Split('\n');
}

unsigned long long RecalboxSystem::getFreeSpace(const String& mountpoint)
{
  struct statvfs fiData {};
  const char* fnPath = mountpoint.c_str();
  unsigned long long free = 0;
  if ((statvfs(fnPath, &fiData)) >= 0)
  {
    free = (((unsigned long long)fiData.f_bfree * (unsigned long long)fiData.f_bsize));
  }
  return free;
}

unsigned long RecalboxSystem::getFreeSpaceGB(const String& mountpoint)
{
  return (unsigned long)(getFreeSpace(mountpoint) >> 30);
}

String RecalboxSystem::getFreeSpaceInfo()
{
  String sharePart = sSharePath;
  String result = "N/A";
  if (!sharePart.empty())
  {
    const char* fnPath = sharePart.c_str();
    struct statvfs fiData {};
    if ((statvfs(fnPath, &fiData)) < 0)
    {
      result += " (SYSTEM ERROR)";
    }
    else
    {
      long long total = ((long long)fiData.f_blocks * (long long)fiData.f_bsize);
      long long free = ((long long)fiData.f_bfree * (long long)fiData.f_bsize);
      if (total != 0)
      {
        long long used = total - free;
        int percent = (int)(used * 100 / total);
        result = Sizes(used).ToHumanSize().Append('/').Append(Sizes(total).ToHumanSize()).Append(" (").Append(percent).Append("%)");
      }
    }
  }
  else result += " (NO PARTITION)";

  return result;
}

bool RecalboxSystem::isFreeSpaceUnderLimit(long long size)
{
  return size < GetMinimumFreeSpaceOnSharePartition();
}

bool RecalboxSystem::isFreeSpaceLimit()
{
  String sharePart = sSharePath;
  if (!sharePart.empty())
  {
    return (long long)getFreeSpace(sharePart) < GetMinimumFreeSpaceOnSharePartition();
  }
  else
  {
    return false; //"ERROR";
  }

}

std::vector<String> RecalboxSystem::getAvailableWiFiSSID(bool activatedWifi)
{
  if (!activatedWifi) enableWifi("", "");
  std::vector<String> result = ExecuteSettingsCommand("wifi list");
  if (!activatedWifi) disableWifi();

  return result;
}

bool RecalboxSystem::setOverscan(bool enable)
{
  String cmd(sConfigScript);
  cmd += " overscan";
  cmd += enable ? " enable" : " disable";
  { LOG(LogInfo) << "[System] Launching " << cmd; }
  if (system(cmd.c_str()) != 0)
  {
    { LOG(LogWarning) << "[System] Error executing " << cmd; }
    return false;
  }
  else
  {
    { LOG(LogInfo) << "[System] Overscan set to : " << enable; }
    return true;
  }

}

bool RecalboxSystem::setOverclock(const String& mode)
{
  if (!mode.empty())
  {
    String cmd(sConfigScript);
    cmd += " overclock";
    cmd += ' ';
    cmd += mode;
    { LOG(LogInfo) << "[System] Launching " << cmd; }
    if (system(cmd.c_str()) != 0)
    {
      { LOG(LogWarning) << "[System] Error executing " << cmd; }
      return false;
    }
    else
    {
      { LOG(LogInfo) << "[System] Overclocking set to " << mode; }
      return true;
    }
  }

  return false;
}

bool RecalboxSystem::backupRecalboxConf()
{
  String cmd(sConfigScript);
  cmd += " configbackup";
  { LOG(LogInfo) << "[System] Launching " << cmd; }
  if (system(cmd.c_str()) == 0)
  {
    { LOG(LogInfo) << "[System] recalbox.conf backup'ed successfully"; }
    return true;
  }
  else
  {
    { LOG(LogInfo) << "[System] recalbox.conf backup failed"; }
    return false;
  }
}

bool RecalboxSystem::enableWifi(String ssid, String key)
{
  ssid.Replace('"', "\\\"");
  key.Replace('"', "\\\"");
  String cmd(sConfigScript);
  cmd += " wifi enable \"" + ssid + "\" \"" + key + "\"";
  { LOG(LogInfo) << "[System] Launching " << cmd; }
  if (system(cmd.c_str()) == 0)
  {
    { LOG(LogInfo) << "[System] Wifi enabled "; }
    return true;
  }
  else
  {
    { LOG(LogInfo) << "[System] Cannot enable wifi "; }
    return false;
  }
}

bool RecalboxSystem::disableWifi()
{
  String cmd(sConfigScript);
  cmd += " wifi disable";
  { LOG(LogInfo) << "[System] Launching " << cmd; }
  if (system(cmd.c_str()) == 0)
  {
    { LOG(LogInfo) << "[System] Wifi disabled "; }
    return true;
  }
  else
  {
    { LOG(LogInfo) << "[System] Cannot disable wifi "; }
    return false;
  }
}

bool RecalboxSystem::getWifiWps()
{
  bool result = false;
  String::List lines = ExecuteSettingsCommand("wifi wps");
  for(const String& line : lines)
    if (line.StartsWith(LEGACY_STRING("OK")))
    {
      result = true;
      break;
    }
  return result;
}

bool RecalboxSystem::saveWifiWps()
{
  bool result = false;
  String::List lines = ExecuteSettingsCommand("wifi save");
  for(const String& line : lines)
    if (line.StartsWith(STRING_AND_LENGTH("OK")))
    {
      result = true;
      break;
    }
  return result;
}

bool RecalboxSystem::getWifiConfiguration(String& ssid, String& psk)
{
  IniFile wpaConfiguration(Path("/overlay/.configs/wpa_supplicant.conf"), false, false);
  ssid = wpaConfiguration.AsString("ssid").Trim(" \"");
  psk = wpaConfiguration.AsString("psk").Trim(" \"");
  return !ssid.empty() && !psk.empty();
}

bool RecalboxSystem::getIpV4Address(String& result)
{
  struct ifaddrs* ifAddrStruct = nullptr;
  getifaddrs(&ifAddrStruct);

  for (struct ifaddrs* ifa = ifAddrStruct; ifa != nullptr; ifa = ifa->ifa_next)
    if (ifa->ifa_addr != nullptr)
      if (ifa->ifa_addr->sa_family == AF_INET)
      {
        void* tmpAddrPtr = &((struct sockaddr_in*) ifa->ifa_addr)->sin_addr;
        char addressBuffer[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
        if (strcmp(ifa->ifa_name, "lo") != 0)
        {
          { LOG(LogDebug) << "[Network] IPv4 found for interface " << ifa->ifa_name << " : " << addressBuffer; }
          result = String(addressBuffer);
          freeifaddrs(ifAddrStruct);
          return true;
        }
      }

  if (ifAddrStruct != nullptr) freeifaddrs(ifAddrStruct);
  return false;
}

bool RecalboxSystem::getIpV4WirelessAddress(String& result)
{
  struct ifaddrs* ifAddrStruct = nullptr;
  getifaddrs(&ifAddrStruct);

  for (struct ifaddrs* ifa = ifAddrStruct; ifa != nullptr; ifa = ifa->ifa_next)
    if (ifa->ifa_addr != nullptr)
      if (ifa->ifa_addr->sa_family == AF_INET)
      {
        void* tmpAddrPtr = &((struct sockaddr_in*) ifa->ifa_addr)->sin_addr;
        char addressBuffer[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
        if (strcmp(ifa->ifa_name, "lo") != 0 && strncmp(ifa->ifa_name, STRING_AND_LENGTH("eth")) != 0)
        {
          { LOG(LogDebug) << "[Network] IPv4 found for interface " << ifa->ifa_name << " : " << addressBuffer; }
          result = String(addressBuffer);
          freeifaddrs(ifAddrStruct);
          return true;
        }else
          { LOG(LogDebug) << "[Network] Skip IPv4 found for interface " << ifa->ifa_name << " : " << addressBuffer; }
      }

  if (ifAddrStruct != nullptr) freeifaddrs(ifAddrStruct);
  return false;
}

bool RecalboxSystem::getIpV6Address(String& result)
{
  struct ifaddrs* ifAddrStruct = nullptr;
  getifaddrs(&ifAddrStruct);

  for (struct ifaddrs* ifa = ifAddrStruct; ifa != nullptr; ifa = ifa->ifa_next)
    if (ifa->ifa_addr != nullptr)
      if (ifa->ifa_addr->sa_family == AF_INET6)
      {
        void* tmpAddrPtr = &((struct sockaddr_in6*) ifa->ifa_addr)->sin6_addr;
        char addressBuffer[INET6_ADDRSTRLEN];
        inet_ntop(AF_INET6, tmpAddrPtr, addressBuffer, INET6_ADDRSTRLEN);
        if (strcmp(ifa->ifa_name, "lo") != 0)
        {
          { LOG(LogDebug) << "[Network] IPv6 found for interface " << ifa->ifa_name << " : " << addressBuffer; }
          result = String(addressBuffer);
          freeifaddrs(ifAddrStruct);
          return true;
        }
      }

  if (ifAddrStruct != nullptr) freeifaddrs(ifAddrStruct);
  return false;
}

bool RecalboxSystem::getIpV6WirelessAddress(String& result)
{
  struct ifaddrs* ifAddrStruct = nullptr;
  getifaddrs(&ifAddrStruct);

  for (struct ifaddrs* ifa = ifAddrStruct; ifa != nullptr; ifa = ifa->ifa_next)
    if (ifa->ifa_addr != nullptr)
      if (ifa->ifa_addr->sa_family == AF_INET6)
      {
        void* tmpAddrPtr = &((struct sockaddr_in6*) ifa->ifa_addr)->sin6_addr;
        char addressBuffer[INET6_ADDRSTRLEN];
        inet_ntop(AF_INET6, tmpAddrPtr, addressBuffer, INET6_ADDRSTRLEN);
        if (strcmp(ifa->ifa_name, "lo") != 0 && strncmp(ifa->ifa_name, STRING_AND_LENGTH("eth")) != 0)
        {
          { LOG(LogDebug) << "[Network] IPv6 found for interface " << ifa->ifa_name << " : " << addressBuffer; }
          result = String(addressBuffer);
          freeifaddrs(ifAddrStruct);
          return true;
        }else
          { LOG(LogDebug) << "[Network] Skip IPv6 found for interface " << ifa->ifa_name << " : " << addressBuffer; }
      }

  if (ifAddrStruct != nullptr) freeifaddrs(ifAddrStruct);
  return false;
}

String RecalboxSystem::getIpAddress()
{
  String result = "NOT CONNECTED";

  if (!getIpV4Address(result))
    getIpV6Address(result);

  return result;
}

bool RecalboxSystem::hasIpAdress(bool onlyWIFI)
{
  bool result = false;
  struct ifaddrs* ifAddrStruct = nullptr;
  getifaddrs(&ifAddrStruct);

  for (struct ifaddrs* ifa = ifAddrStruct; ifa != nullptr; ifa = ifa->ifa_next)
  {
    if (ifa->ifa_addr == nullptr) continue;
    if (ifa->ifa_addr->sa_family == AF_INET)
    {
      void* tmpAddrPtr = &((struct sockaddr_in*) ifa->ifa_addr)->sin_addr;
      char addressBuffer[INET_ADDRSTRLEN];
      inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
      if ((strcmp(ifa->ifa_name, "lo") != 0 && !onlyWIFI) ||
          (strncmp(ifa->ifa_name, STRING_AND_LENGTH("wlan")) == 0 &&
           strncmp(addressBuffer, STRING_AND_LENGTH("169.254")) != 0 &&
           onlyWIFI))
      {
        { LOG(LogDebug) << "[Network] Interface " << ifa->ifa_name << " has IPv4 address " << addressBuffer; }
        result = true;
        break;
      }
    }
  }
  // Seeking for ipv6 if no IPV4
  if (!result)
    for (struct ifaddrs* ifa = ifAddrStruct; ifa != nullptr; ifa = ifa->ifa_next)
    {
      if (ifa->ifa_addr == nullptr) continue;
      if (ifa->ifa_addr->sa_family == AF_INET6)
      {
        void* tmpAddrPtr = &((struct sockaddr_in6*) ifa->ifa_addr)->sin6_addr;
        char addressBuffer[INET6_ADDRSTRLEN];
        inet_ntop(AF_INET6, tmpAddrPtr, addressBuffer, INET6_ADDRSTRLEN);
        if ((strcmp(ifa->ifa_name, "lo") != 0 && !onlyWIFI) ||
            (strncmp(ifa->ifa_name, STRING_AND_LENGTH("wlan")) == 0 &&
             strncmp(addressBuffer, STRING_AND_LENGTH("fe80")) != 0 &&
             strncmp(addressBuffer, STRING_AND_LENGTH("fd")) != 0 &&
             onlyWIFI))
        {
          { LOG(LogDebug) << "[Network] Interface " << ifa->ifa_name << " has IPv6 address " << addressBuffer; }
          result = true;
          break;
        }
      }
    }

  if (ifAddrStruct != nullptr) freeifaddrs(ifAddrStruct);
  return result;
}

String::List RecalboxSystem::scanBluetooth()
{
  return ExecuteSettingsCommand("hcitoolscan");
}

bool RecalboxSystem::pairBluetooth(const String& controller)
{
  String escapedController(controller);
  escapedController.Replace('(', "\\(")
                   .Replace(')', "\\)")
                   .Replace('*', "\\*")
                   .Replace('\'', "\\'")
                   .Replace('"', "\\\"");
  String cmd(sConfigScript);
  cmd += " hiddpair " + escapedController;
  int exitcode = system(cmd.c_str());
  return exitcode == 0;
}

std::vector<String> RecalboxSystem::getAvailableStorageDevices()
{
  return ExecuteSettingsCommand("storage list");
}

String RecalboxSystem::getCurrentStorage()
{
  String::List lines = ExecuteSettingsCommand("storage current");
  return lines.empty() ? "INTERNAL" : lines[0];
}

bool RecalboxSystem::setStorage(const String& selected)
{
  String cmd(sConfigScript);
  cmd += " storage " + selected;
  int exitcode = system(cmd.c_str());
  return exitcode == 0;
}

bool RecalboxSystem::forgetBluetoothControllers()
{
  String cmd(sConfigScript);
  cmd += " forgetBT";
  int exitcode = system(cmd.c_str());
  return exitcode == 0;
}

String RecalboxSystem::getRootPassword()
{
  String::List lines = ExecuteSettingsCommand("getRootPassword");
  return lines.empty() ? "" : lines[0];
}

std::pair<String, int> RecalboxSystem::execute(const String& command)
{
  FILE* pipe = popen(command.c_str(), "r");
  char line[1024];

  if (pipe == nullptr)
  {
    return std::make_pair("", -1);
  }
  String output;
  while (fgets(line, 1024, pipe) != nullptr)
  {
    output += line;
  }
  int exitCode = pclose(pipe);
  return std::make_pair(output, WEXITSTATUS(exitCode));
}

bool RecalboxSystem::ping()
{
  return Upgrade::NetworkReady();
}

std::pair<String, int> RecalboxSystem::getSDLBatteryInfo()
{
  std::pair<String, int> result;
  int percent = -1;
  auto powerInfo = SDL_GetPowerInfo(nullptr, &percent);
  switch (powerInfo)
  {
    case SDL_POWERSTATE_UNKNOWN:
    case SDL_POWERSTATE_NO_BATTERY:
    {
      percent = -1;
      break;
    }
    case SDL_POWERSTATE_ON_BATTERY:
    {
      if (percent > 66)
        result.first = "\uF1ba";
      else if (percent > 33)
        result.first = "\uF1b8";
      else if (percent > 15)
        result.first = "\uF1b1";
      else
        result.first = "\uF1b5";
      break;
    }
    case SDL_POWERSTATE_CHARGING:
    case SDL_POWERSTATE_CHARGED:
    {
      result.first = "\uf1b4";
      break;
    }
  }
  result.second = percent;

  return result;
}

bool RecalboxSystem::getSysBatteryInfo(int& charge, int& unicodeIcon)
{
  if (!Board::Instance().HasBattery()) return false;

  charge = Board::Instance().BatteryChargePercent();

  unicodeIcon = 0xf1b4;
  if (!Board::Instance().IsBatteryCharging())
  {
    if (charge > 66)      unicodeIcon = 0xF1ba;
    else if (charge > 33) unicodeIcon = 0xF1b8;
    else if (charge > 15) unicodeIcon = 0xF1b1;
    else                  unicodeIcon = 0xF1b5;
  }

  return true;
}

bool RecalboxSystem::kodiExists()
{
  return Path("/usr/bin/kodi").IsFile();
}

bool RecalboxSystem::MakeBootReadOnly()
{
  return system("mount -o remount,ro /boot") == 0;
}

bool RecalboxSystem::MakeBootReadWrite()
{
  return system("mount -o remount,rw /boot") == 0;
}

bool RecalboxSystem::IsLiteVersion()
{
  // Original file existe?
  if (!Path(sLiteFlagFile).Exists()) return false;

  // Keep track of lite versions
  Path track(sLiteFlagTrackFile);
  if (track.Exists())
    Files::SaveFile(track, String::Empty);

  return true;
}
