//
// Created by bkg2k on 02/11/2020.
//

#include "RG351VJackEventReader.h"
#include <utils/Log.h>
#include <linux/input.h>
#include <fcntl.h>
#include <poll.h>
#include <hardware/IBoardInterface.h>

RG351VJackEventReader::RG351VJackEventReader(HardwareMessageSender& messageSender)
  : mSender(messageSender)
  , mFileHandle(0)
{
}

RG351VJackEventReader::~RG351VJackEventReader()
{
  StopReader();
}

void RG351VJackEventReader::StartReader()
{
  { LOG(LogDebug) << "[RG351V] Headphone driver requested to start."; }
  Start("RG351VJack");
}

void RG351VJackEventReader::StopReader()
{
  { LOG(LogDebug) << "[RG351V] Headphone driver requested to stop."; }
  Stop();
}

void RG351VJackEventReader::Break()
{
  if (mFileHandle >= 0)
  {
    { LOG(LogDebug) << "[RG351V] Breaking headphone thread."; }
    mFileHandle = -1;
    }
}

void RG351VJackEventReader::Run()
{
  { LOG(LogInfo) << "[RG351V] Running background headphone driver running."; }
  while(IsRunning())
  {
    mFileHandle = open(sInputEventPath, O_RDONLY);
    if (mFileHandle < 0)
    {
      { LOG(LogError) << "[RG351V] Error opening " << sInputEventPath << ". Retry in 5s..."; }
      sleep(5);
      continue;
    }

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
          { LOG(LogError) << "[RG351V] Error reading " << sInputEventPath << ". Retrying"; }
          continue;
        }
        // If file handle NOK, we're instructed to quit
        { LOG(LogInfo) << "[RG351V] Headphone driver ordered to stop."; }
        break;
      }

      // Headphone plugged in or unplugged
      if ((event.type == sHeadphoneInsertType) && (event.code == sHeadphoneInsertCode))
        mSender.Send(BoardType::RG353V, (event.value == 0) ? MessageTypes::HeadphoneUnplugged : MessageTypes::HeadphonePluggedIn);
    }
  }
}
