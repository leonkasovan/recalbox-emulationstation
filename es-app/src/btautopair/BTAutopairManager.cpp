//
// Created by Bkg2k on 10/03/2020.
//

#include "BTAutopairManager.h"

/*
 * Members
 */
BTAutopairManager::BTAutopairManager()
  : StaticLifeCycleControler<BTAutopairManager>("BTAutopairManager"),
    mMQTTClient("recalbox-emulationstation-bt", nullptr)
{
  mMQTTClient.WaitFor(10);
}

const char* BTAutopairManager::ActionToString(BTCommand action)
{
  switch(action)
  {
    case BTCommand::StartDiscovery:  return R"({"command": "start_discovery"})";
    case BTCommand::StopDiscovery:   return R"({"command": "stop_discovery"})";
    default: break;
  }
  return "error";
}

void BTAutopairManager::StartDiscovery()
{
  mMQTTClient.Send(sBtTopic, ActionToString(BTCommand::StartDiscovery));
}

void BTAutopairManager::StopDiscovery()
{
  mMQTTClient.Send(sBtTopic, ActionToString(BTCommand::StopDiscovery));
}


