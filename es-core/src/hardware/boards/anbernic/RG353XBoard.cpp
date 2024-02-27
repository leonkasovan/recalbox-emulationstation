//
// Created by bkg2k on 30/10/2020.
// Modified by davidb2111 for the RG353x series of boards
//

#include "RG353XBoard.h"
#include <input/InputCompactEvent.h>
#include <input/InputManager.h>
#include <RecalboxConf.h>
#include <utils/math/Misc.h>
#include <utils/Files.h>
#include <audio/AudioController.h>

bool RG353XBoard::ProcessSpecialInputs(InputCompactEvent& inputEvent, ISpecialGlobalAction* action)
{
  (void)action;

  // Trap hotkey event, as it is needed to handle volup/voldown or brightup/brightdown with HK
  if (inputEvent.HotkeyPressed())
    mVolumeReader.HotkeyPressed();
  if (inputEvent.HotkeyReleased())
    mVolumeReader.HotkeyReleased();

  return false;
}

void RG353XBoard::SetLowestBrightness()
{
  Files::SaveFile(Path("/sys/class/backlight/backlight/brightness"), "0");
}

void RG353XBoard::SetBrightness(int step)
{
  String maxValue = Files::LoadFile(Path("/sys/class/backlight/backlight/max_brightness"));
  int max = 255; // Max RG value
  (void)maxValue.Trim("\r\n").TryAsInt(max);
  int value = 1 << step; if (value > max) value = max;
  Files::SaveFile(Path("/sys/class/backlight/backlight/brightness"), String(value));
}

void RG353XBoard::SetCPUGovernance(IBoardInterface::CPUGovernance cpuGovernance)
{
  switch (cpuGovernance)
  {
    case CPUGovernance::PowerSave:
    {
      { LOG(LogInfo) << "[CPU] Set powersaving mode"; }
      Files::SaveFile(Path(sCpuGovernancePath), "powersave");
      Files::SaveFile(Path(sGpuGovernancePath), "powersave");
      Files::SaveFile(Path(sDmcGovernancePath), "powersave");
      break;
    }
    case CPUGovernance::OnDemand:
    {
      { LOG(LogInfo) << "[CPU] Set on-demand mode"; }
      Files::SaveFile(Path(sCpuGovernancePath), "ondemand");
      Files::SaveFile(Path(sGpuGovernancePath), "simple_ondemand");
      Files::SaveFile(Path(sDmcGovernancePath), "dmc_ondemand");
      break;
    }
    case CPUGovernance::FullSpeed:
    {
      { LOG(LogInfo) << "[CPU] Set performance mode"; }
      Files::SaveFile(Path(sCpuGovernancePath), "performance");
      Files::SaveFile(Path(sGpuGovernancePath), "performance");
      Files::SaveFile(Path(sDmcGovernancePath), "performance");
      break;
    }
    default: break;
  }
}

int RG353XBoard::BatteryChargePercent()
{
  static Path sBatteryCharge(sBatteryCapacityPath);
  int charge = -1;
  (void)Files::LoadFile(sBatteryCharge).Trim('\n').TryAsInt(charge);
  return charge;
}

bool RG353XBoard::IsBatteryCharging()
{
  static Path sBatteryStatus(sBatteryStatusPath);
  return Files::LoadFile(sBatteryStatus).Trim('\n') == "Charging";
}

void RG353XBoard::HeadphonePlugged()
{
  if (system("amixer sset 'Playback Path' HP") != 0)
  { LOG(LogError) << "[RG353XBoard] Error setting headphone on!"; }
  AudioController::Instance().SetOutputPort("rk817-headphones");
}

void RG353XBoard::HeadphoneUnplugged()
{
  if (system("amixer sset 'Playback Path' SPK") != 0)
  { LOG(LogError) << "[RG353XBoard] Error setting headphone off!"; }
  AudioController::Instance().SetOutputPort("rk817-speaker");
}

void RG353XBoard::SetFrontendCPUGovernor()
{
  SetCPUGovernance(CPUGovernance::OnDemand);
}

const RotationCapability RG353XBoard::GetRotationCapabilities() const {
  return {.rotationAvailable = true, .systemRotationAvailable = false, .defaultRotationWhenTate = RotationType::Left, .rotateControls = true, .autoRotateGames = true};
}
