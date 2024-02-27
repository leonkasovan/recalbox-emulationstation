//
// Created by bkg2k on 13/02/2021.
// Modified by davidb2111 for the RG351V board
//
// As part of the RECALBOX Project
// http://www.recalbox.com
//

#include "RG351VVolumeReader.h"
#include <utils/Log.h>
#include <linux/input.h>
#include <fcntl.h>
#include <poll.h>
#include <hardware/IBoardInterface.h>

RG351VVolumeReader::RG351VVolumeReader(HardwareMessageSender& messageSender)
  : mSender(messageSender)
  , mFileHandle(0)
{
}

RG351VVolumeReader::~RG351VVolumeReader()
{
  StopReader();
}

void RG351VVolumeReader::StartReader()
{
  { LOG(LogDebug) << "[RG351V] Volume button driver requested to start."; }
  Start("RG351VVol");
}

void RG351VVolumeReader::StopReader()
{
  { LOG(LogDebug) << "[RG351V] Volume button driver requested to stop."; }
  Stop();
}

void RG351VVolumeReader::Break()
{
  if (mFileHandle >= 0)
  {
    { LOG(LogDebug) << "[RG351V] Breaking Volume button thread."; }
    mFileHandle = -1;
  }
}

void RG351VVolumeReader::Run()
{
  { LOG(LogInfo) << "[RG351V] Running background Volume button driver running."; }
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
        { LOG(LogInfo) << "[RG351V] Volume button driver ordered to stop."; }
        break;
      }
      // Volume down and hotkey buttons pressed?
      if (mHotkeyPressed)
      {
        if ((event.type == EV_KEY) && (event.code == sVolumeDown) && (event.value >= 1))
          mSender.Send(BoardType::RG351V, MessageTypes::BrightnessDownPressed);
        // Volume up button pressed?
        if ((event.type == EV_KEY) && (event.code == sVolumeUp) && (event.value >= 1))
          mSender.Send(BoardType::RG351V, MessageTypes::BrightnessUpPressed);
      }else // only volume button pressed
      {
        if ((event.type == EV_KEY) && (event.code == sVolumeDown) && (event.value >= 1))
          mSender.Send(BoardType::RG351V, MessageTypes::VolumeDownPressed);
        // Volume up button pressed?
        if ((event.type == EV_KEY) && (event.code == sVolumeUp) && (event.value >= 1))
          mSender.Send(BoardType::RG351V, MessageTypes::VolumeUpPressed);
      }
    }
  }
}
