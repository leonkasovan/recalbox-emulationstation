//
// Created by bkg2k on 02/11/2020.
//

#include "RG353XJackEventReader.h"
#include <utils/Log.h>
#include <linux/input.h>
#include <fcntl.h>
#include <poll.h>
#include <hardware/IBoardInterface.h>

RG353XJackEventReader::RG353XJackEventReader(HardwareMessageSender& messageSender)
  : mSender(messageSender)
  , mFileHandle(0)
{
}

RG353XJackEventReader::~RG353XJackEventReader()
{
  StopReader();
}

void RG353XJackEventReader::StartReader()
{
  { LOG(LogDebug) << "[RG353X] Headphone driver requested to start."; }
  Start("RG353XJack");
}

void RG353XJackEventReader::StopReader()
{
  { LOG(LogDebug) << "[RG353X] Headphone driver requested to stop."; }
  Stop();
}

void RG353XJackEventReader::Break()
{
  if (mFileHandle >= 0)
  {
    { LOG(LogDebug) << "[RG353X] Breaking headphone thread."; }
    mFileHandle = -1;
    }
}

void RG353XJackEventReader::Run()
{
  { LOG(LogInfo) << "[RG353X] Running background headphone driver running."; }
  while(IsRunning())
  {
    mFileHandle = open(sInputEventPath, O_RDONLY);
    if (mFileHandle < 0)
    {
      { LOG(LogError) << "[RG353X] Error opening " << sInputEventPath << ". Retry in 5s..."; }
      sleep(5);
      continue;
    }

    // query device for initial value
    int initialValue;
    if (ioctl(mFileHandle, EVIOCGSW(SW_HEADPHONE_INSERT), &initialValue) != -1) {
      mSender.Send(BoardType::RG353V, (bool)initialValue ? MessageTypes::HeadphonePluggedIn : MessageTypes::HeadphoneUnplugged);
      { LOG(LogDebug) << "[RG353X] Initial headphone status: " << ((bool)initialValue ? "plugged" : "unplugged"); }
    }else
      { LOG(LogDebug) << "[RG353X] Initial headphone status: unknown"; }


    while(IsRunning())
    {
      // Poll
      struct pollfd poller { .fd = mFileHandle, .events = POLLIN, .revents = 0 };
      if (poll(&poller, 1, 100) != 1 || (poller.revents & POLLIN) == 0) continue;

      // Read event
      struct input_event event {};
      if (read(mFileHandle, &event, sizeof(event)) != sizeof(event))
      {
        close(mFileHandle);
        // Error while the file handle is ok means a true read error
        if (mFileHandle >= 0)
        {
          { LOG(LogError) << "[RG353X] Error reading " << sInputEventPath << ". Retrying"; }
          continue;
        }
        // If file handle NOK, we're instructed to quit
        { LOG(LogInfo) << "[RG353X] Headphone driver ordered to stop."; }
        break;
      }

      // Headphone plugged in or unplugged
      if ((event.type == sHeadphoneInsertType) && (event.code == sHeadphoneInsertCode))
        mSender.Send(BoardType::RG353V, (event.value == 0) ? MessageTypes::HeadphoneUnplugged : MessageTypes::HeadphonePluggedIn);
    }
  }
}
