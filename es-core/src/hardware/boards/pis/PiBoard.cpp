//
// Created by digitalLumberjack on 20/06/2022.
//

#include <recalbox/RecalboxSystem.h>
#include "PiBoard.h"
#include "hardware/ISpecialGlobalActions.h"
#include <utils/Files.h>

bool PiBoard::OnRebootOrShutdown() {
  if (Path("/sys/class/leds/retroflagled/trigger").Exists())
  {
    RecalboxSystem::execute("echo 'heartbeat' > /sys/class/leds/retroflagled/trigger");
    return true;
  }
  return false;
}

bool PiBoard::ProcessSpecialInputs(InputCompactEvent& inputEvent, ISpecialGlobalAction* action)
{
  // show the OSD on the PiBoy XRS
  if (Case::CurrentCase().Model() == Case::CaseModel::PiBoyXRS &&
      inputEvent.RawEvent().Type() == InputEvent::EventType::Button && inputEvent.RawEvent().Id() == 10 &&
      InputManager::Instance().GetDeviceNameFromId(inputEvent.RawEvent().Device()) == String("PiBoy XRS Controller"))
  /*if (inputEvent.Hotkey())*/
    if (action != nullptr)
    {
      if (inputEvent.RawEvent().Value() != 0) action->EnableOSDImage(Path(":/help/piboy_xrs_menu.png"), 0.f, 0.f, 1.f, 1.f, 0.9f, true);
      else action->DisableOSDImage();
      return true;
    }
  return false;
}

bool PiBoard::HasBattery()
{
  switch(Case::CurrentCase().Model())
  {
    case Case::CaseModel::PiBoyXRS:
    case Case::CaseModel::PiBoyDMG:
      return true;
    case Case::CaseModel::GPiV1:
    case Case::CaseModel::GPiV2:
    case Case::CaseModel::GPiV3:
    case Case::CaseModel::GPi2:
    case Case::CaseModel::GPi2W:
    case Case::CaseModel::Nuxii:
    case Case::CaseModel::Nespi4Case:
    case Case::CaseModel::Nespi4CaseManual:
    case Case::CaseModel::SuperPi4Case:
    case Case::CaseModel::NespiCasePlus:
    case Case::CaseModel::PiStation:
    case Case::CaseModel::SuperPiCase:
    case Case::CaseModel::MegaPiCase:
    case Case::CaseModel::ArgonOne:
    case Case::CaseModel::RaspberryPiTouchDisplay:
    case Case::CaseModel::RecalboxRGBDualOrRGBHat:
    case Case::CaseModel::None:
      return false;
      break;
  }
  return false;
}

int PiBoard::BatteryChargePercent()
{
  if (Case::CurrentCase().Model() == Case::CaseModel::PiBoyXRS ||
      Case::CurrentCase().Model() == Case::CaseModel::PiBoyDMG)
  {
    static Path sBatteryCharge(Case::sPiboyBatteryCapacityPath);
    int charge = -1;
    (void)Files::LoadFile(sBatteryCharge).TryAsInt(charge);
    return charge;
  }
  return -1;
}

bool PiBoard::IsBatteryCharging()
{
  if (Case::CurrentCase().Model() == Case::CaseModel::PiBoyXRS ||
      Case::CurrentCase().Model() == Case::CaseModel::PiBoyDMG)
  {
    static Path sBatteryCharge(Case::sPiboyAmpsPath);
    int charge = -1;
    (void)Files::LoadFile(sBatteryCharge).TryAsInt(charge);
    return charge >= 0 ? true : false;
  }
  return false;
}
