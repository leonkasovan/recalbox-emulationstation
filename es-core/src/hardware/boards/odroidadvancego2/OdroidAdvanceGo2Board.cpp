//
// Created by bkg2k on 30/10/2020.
//

#include <input/InputCompactEvent.h>
#include "OdroidAdvanceGo2Board.h"
#include <utils/math/Misc.h>
#include <utils/Files.h>
#include <audio/AudioController.h>

bool OdroidAdvanceGo2Board::ProcessSpecialInputs(InputCompactEvent& inputEvent, ISpecialGlobalAction* action)
{
  (void)action;

  // When the wizard is on screen, do not consume special key events
  // so that the wizard can use it
  bool pressResult = !RecalboxConf::Instance().GetFirstTimeUse();

  if (inputEvent.VolumeUpPressed())
  {
    { LOG(LogDebug) << "[OdroidAdvanceGo] Volume + pressed"; }
    int value = RecalboxConf::Instance().GetAudioVolume() + 10;
    value = Math::clampi(value, 0, 100);
    value = (value / 10) * 10;
    AudioController::Instance().SetVolume(value);
    RecalboxConf::Instance().SetAudioVolume(value);
    RecalboxConf::Instance().Save();
    return pressResult;
  }
  if (inputEvent.VolumeDownPressed())
  {
    { LOG(LogDebug) << "[OdroidAdvanceGo] Volume - pressed"; }
    int value = RecalboxConf::Instance().GetAudioVolume() - 10;
    value = Math::clampi(value, 0, 100);
    value = (value / 10) * 10;
    AudioController::Instance().SetVolume(value);
    RecalboxConf::Instance().SetAudioVolume(value);
    RecalboxConf::Instance().Save();
    return pressResult;
  }
  if (inputEvent.BrightnessUpPressed())
  {
    { LOG(LogDebug) << "[OdroidAdvanceGo] Brightness + pressed"; }
    int value = RecalboxConf::Instance().GetBrightness() + 1;
    value = Math::clampi(value, 0, 8);
    SetBrightness(value);
    RecalboxConf::Instance().SetBrightness(value);
    RecalboxConf::Instance().Save();
    return pressResult;
  }
  if (inputEvent.BrightnessDownPressed())
  {
    { LOG(LogDebug) << "[OdroidAdvanceGo] Brightness - pressed"; }
    int value = RecalboxConf::Instance().GetBrightness() - 1;
    value = Math::clampi(value, 0, 8);
    SetBrightness(value);
    RecalboxConf::Instance().SetBrightness(value);
    RecalboxConf::Instance().Save();
    return pressResult;
  }

  return false;
}

void OdroidAdvanceGo2Board::SetLowestBrightness()
{
  Files::SaveFile(Path("/sys/class/backlight/backlight/brightness"), "0");
}

void OdroidAdvanceGo2Board::SetBrightness(int step)
{
  String maxValue = Files::LoadFile(Path("/sys/class/backlight/backlight/max_brightness"));
  int max = 160; // Max GoS value
  (void)maxValue.Trim("\r\n").TryAsInt(max);
  int value = 1 << step; if (value > max) value = max;
  Files::SaveFile(Path("/sys/class/backlight/backlight/brightness"), String(value));
}

void OdroidAdvanceGo2Board::SetCPUGovernance(IBoardInterface::CPUGovernance cpuGovernance)
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

int OdroidAdvanceGo2Board::BatteryChargePercent()
{
  static Path sBatteryCharge(sBatteryCapacityPath);
  int charge = -1;
  (void)Files::LoadFile(sBatteryCharge).Trim('\n').TryAsInt(charge);
  return charge;
}

bool OdroidAdvanceGo2Board::IsBatteryCharging()
{
  static Path sBatteryStatus(sBatteryStatusPath);
  return Files::LoadFile(sBatteryStatus).Trim('\n') == "Charging";
}

void OdroidAdvanceGo2Board::HeadphonePlugged()
{
  if (system("amixer sset 'Playback Path' HP") != 0)
  { LOG(LogError) << "[OdroidAdvanceGo2Board] Error setting headphone on!"; }
}

void OdroidAdvanceGo2Board::HeadphoneUnplugged()
{
  if (system("amixer sset 'Playback Path' SPK") != 0)
  { LOG(LogError) << "[OdroidAdvanceGo2Board] Error setting headphone off!"; }
}

void OdroidAdvanceGo2Board::SetFrontendCPUGovernor()
{
  SetCPUGovernance(CPUGovernance::PowerSave);
}

const RotationCapability OdroidAdvanceGo2Board::GetRotationCapabilities() const {
  if(mModel == BoardType::OdroidAdvanceGoSuper){
    return {.rotationAvailable = true, .systemRotationAvailable = false, .defaultRotationWhenTate = RotationType::Left, .rotateControls = true, .autoRotateGames = true};
  }
  // Odroid GO Advance cannot rotate.
  return {.rotationAvailable = false, .systemRotationAvailable = false, .defaultRotationWhenTate = RotationType::None, .rotateControls = false, .autoRotateGames = false};
}
