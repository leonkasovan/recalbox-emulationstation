//
// Created by bkg2k on 30/03/2020.
//

#include <cstring>
#include <utils/Log.h>
#include "Parameters.h"

void Parameters::LogConfig()
{
  LOG(LogInfo) << "Parameters:";
  LOG(LogInfo) << "  Interface    : " << mIP;
  LOG(LogInfo) << "  Port         : " << mPort;
  LOG(LogInfo) << "  Threads      : " << mThreads;
  LOG(LogInfo) << "  Root folder  : " << mWWWRoot;
  LOG(LogInfo) << "  Default file : " << mDefaultFile;
  LOG(LogInfo) << "  Debug logs   : " << mDebug;
}
