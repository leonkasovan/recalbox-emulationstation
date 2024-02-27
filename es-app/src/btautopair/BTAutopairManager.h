//
// Created by davidb2111 on 03/11/2023.
//
#pragma once

#include <utils/cplusplus/StaticLifeCycleControler.h>
#include <mqtt/MqttClient.h>

enum class BTCommand
{
  StartDiscovery = 1,
  StopDiscovery  = 2,
};

class BTAutopairManager : public StaticLifeCycleControler<BTAutopairManager>
{
  private:
    // MQTT client
    MqttClient mMQTTClient;

    // Topic
    String sBtTopic = "bluetooth/operation";

    //! Convert action to string
    const char* ActionToString(BTCommand action);

  public:
    //! constructor
    explicit BTAutopairManager();

    //! ask for bluetooth discovery
    void StartDiscovery();

    //! ask to stop bluetooth discovery
    void StopDiscovery();
};
