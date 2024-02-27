//
// Created by digi on 12/3/23.
//

#include "PiPowerAndTempThread.h"
#include "recalbox/RecalboxSystem.h"
#include "guis/GuiInfoPopup.h"
#include "utils/locale/LocaleHelper.h"

/*
 *  11110000000000001010
    ||||            ||||_ under-voltage
    ||||            |||_ currently throttled
    ||||            ||_ arm frequency capped
    ||||            |_ soft temperature reached
    ||||_ under-voltage has occurred since last reboot
    |||_ throttling has occurred since last reboot
    ||_ arm frequency capped has occurred since last reboot
    |_ soft temperature reached since last reboot
 */
enum ThrotthledBits
{
  UnderVoltageCurrently = 0x00001,
  ThrotthledCurrently = 0x00002,
  ArmCappedCurrently = 0x00004,
  SoftTempReachedCurrently = 0x00008,
  UnderVoltageSinceStart = 0x10000,
  ThrotthledSinceStart = 0x20000,
  ArmCappedSinceStart = 0x40000,
  SoftTempReachedSinceStart = 0x80000,
};

void PiPowerAndTempThread::Run()
{
  bool throttledSent = false;
  bool tempSent = false;

  while (this->IsRunning())
  {
    static String prefix = "=0x";
    std::pair<String, int> res = RecalboxSystem::execute("vcgencmd get_throttled");
    if (res.second == 0 && res.first.Contains(prefix))
    {
      String value = String(res.first.ToTrim(), res.first.Find(prefix) + 1);
      int throtthled = 0;
      if (value.TryAsInt(throtthled))
      {
        if ((throtthled & UnderVoltageSinceStart) && !throttledSent)
        {
          messageSender.Send(boardType, MessageTypes::UnderVoltage);
          throttledSent = true;
        }
        else if ((throtthled & SoftTempReachedSinceStart) && !tempSent)
        {
          messageSender.Send(boardType, MessageTypes::TemperatureLimit);
          tempSent = true;
        }
      }
    }
    sleep(10);
  }
}
