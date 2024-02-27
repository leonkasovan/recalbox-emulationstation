//
// Created by digitalLumberjack on 12/3/23.
//
#pragma once

#include "utils/os/system/Thread.h"
#include "WindowManager.h"

class PiPowerAndTempThread : public Thread
{
  public:
    explicit PiPowerAndTempThread(HardwareMessageSender& messageSender, BoardType boardType)
      : messageSender(messageSender), boardType(boardType) {};
    void Run() override;
  private:
    HardwareMessageSender& messageSender;
    BoardType boardType;
};


