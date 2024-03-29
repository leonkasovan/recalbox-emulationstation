//
// Created by bkg2k on 01/11/2020.
// Modified by davidb2111 for the RG353x series of boards
//

#include "RG351VPowerEventReader.h"
#include <utils/Log.h>
#include <linux/input.h>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <poll.h>
#include <utils/datetime/DateTime.h>
#include <MainRunner.h>

RG351VPowerEventReader::RG351VPowerEventReader(HardwareMessageSender& messageSender)
  : mSender(messageSender)
  , mFileHandle(0)
  , mWaitFor(WaitFor::Press)
{
}

RG351VPowerEventReader::~RG351VPowerEventReader()
{
  StopReader();
}

void RG351VPowerEventReader::StartReader()
{
  { LOG(LogDebug) << "[RG351V] Power button manager requested to start."; }
  Start("RG351VPower");
}

void RG351VPowerEventReader::StopReader()
{
  { LOG(LogDebug) << "[RG351V] Power button manager requested to stop."; }
  Stop();
}

void RG351VPowerEventReader::Break()
{
  if (mFileHandle >= 0)
  {
    { LOG(LogDebug) << "[RG351V] Breaking power button thread."; }
    mFileHandle = -1;
  }
}

void RG351VPowerEventReader::Run()
{
  { LOG(LogInfo) << "[RG351V] Running background power button manager."; }
  while(IsRunning())
  {
    mFileHandle = open(sInputEventPath, O_RDONLY);
    if (mFileHandle < 0)
    {
      { LOG(LogError) << "[RG351V] Error opening " << sInputEventPath << ". Retry in 5s..."; }
      sleep(5);
      continue;
    }

    input_event pressEvent {};
    while(IsRunning())
    {
      // Poll
      struct pollfd poller { .fd = mFileHandle, .events = POLLIN, .revents = 0 };
      if (poll(&poller, 1, 100) != 1 || (poller.revents & POLLIN) == 0)
      {
        // If the button is pressed at least 2000ms
        // Just send the message and ignore release event.
        if (mWaitFor == WaitFor::Release)
        {
          timeval now {};
          gettimeofday(&now, nullptr);
          long start = (pressEvent.time.tv_usec / 1000) + (pressEvent.time.tv_sec * 1000);
          long stop = (now.tv_usec / 1000) + (now.tv_sec * 1000);
          long elapsed = stop - start;
          if (elapsed >= 2000)
          {
            mSender.Send(BoardType::RG353P, MessageTypes::PowerButtonPressed, (int)elapsed);
            mWaitFor = WaitFor::Ignore; // Ignore releasing the button
          }
        }
        continue;
      }

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
        { LOG(LogInfo) << "[RG351V] Power event reader ordered to stop."; }
        break;
      }

      // Power button pressed?
      if ((event.type == 1) && (event.code == sPowerKeyCode))
        switch(mWaitFor)
        {
          case WaitFor::Press:
          {
            if (event.value == 1) // Really pressed?
            {
              mWaitFor = WaitFor::Release;
              pressEvent = event;
            }
            break;
          }
          case WaitFor::Release:
          {
            if (event.value == 0) // Really released
            {
              long start = (pressEvent.time.tv_usec / 1000) + (pressEvent.time.tv_sec * 1000);
              long stop = (event.time.tv_usec / 1000) + (event.time.tv_sec * 1000);
              long elapsed = stop - start;
              if (elapsed > 20) // Debouncing
                mSender.Send(BoardType::RG353P, MessageTypes::PowerButtonPressed, (int)elapsed);
            }
            break;
          }
          case WaitFor::Ignore: mWaitFor = WaitFor::Press; break;
        }
    }
  }
}

void RG351VPowerEventReader::Suspend()
{
  { LOG(LogInfo) << "[RG351V] SUSPEND!"; }
  mWaitFor = WaitFor::Ignore; // Ignore next event when waking up!

  if (system("/usr/sbin/pm-suspend") != 0) // Suspend mode
  { LOG(LogError) << "[RG351V] Suspend failed!"; }

  { LOG(LogInfo) << "[RG351V] WAKEUP!"; }
  mSender.Send(BoardType::RG353P, MessageTypes::Resume);
}
