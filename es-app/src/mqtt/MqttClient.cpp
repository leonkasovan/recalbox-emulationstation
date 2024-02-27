//
// Created by Bkg2k on 11/03/2020.
//

#include "MqttClient.h"
#include <utils/Log.h>
#include <mqtt/paho/cpp/connect_options.h>

MqttClient::MqttClient(const char* clientId, IMQTTMessageReceived* callback)
  : mSender(*this)
  , mMqtt("tcp://127.0.0.1:1883", clientId, 0, nullptr)
  , mCallbackInterface(callback)
{
  #ifdef FREEZE_MQTT
  return;
  #endif

  // Set options
  mqtt::connect_options connectOptions;
  connectOptions.set_automatic_reconnect(true);  // Auto-reconnect
  connectOptions.set_clean_session(true); // Clean session: do not receive old messages

  // Connect
  try
  {
    if (mOriginalTocken = mMqtt.connect(connectOptions, nullptr, *this); mOriginalTocken != nullptr)
      { mMqtt.start_consuming(); LOG(LogError) << "[MQTT] Connexion to " << mMqtt.get_server_uri() << " from " << mMqtt.get_client_id() << " successful !"; }
    else
      { LOG(LogError) << "[MQTT] Connexion to " << mMqtt.get_server_uri() << " from " << mMqtt.get_client_id() << " failed (init) !"; }
  }
  catch(std::exception& e)
  {
    { LOG(LogError) << "[MQTT] Connection to " << mMqtt.get_server_uri() << " from " << mMqtt.get_client_id() << " failed (catch) ! reason: " << e.what(); }
  }
}

MqttClient::~MqttClient()
{
  mMqtt.stop_consuming();
}

bool MqttClient::Send(const String& topic, const String& message, int qos)
{
  #ifdef FREEZE_MQTT
  return true;
  #endif

  try
  {
    mMqtt.publish(topic, message.data(), message.size(), qos, false, nullptr, *this);
    return true;
  }
  catch(std::exception& e)
  {
    { LOG(LogError) << "[MQTT] Sending messageConnection to " << mMqtt.get_server_uri() << " from " << mMqtt.get_client_id() << " failed (send) ! reason: " << e.what(); }
  }
  return false;
}

void MqttClient::on_failure(const mqtt::token& asyncActionToken)
{
  switch(asyncActionToken.get_type())
  {
    case mqtt::token::CONNECT:
    {
      { LOG(LogError) << "[MQTT] Connection to " << mMqtt.get_server_uri() << " from " << mMqtt.get_client_id() << " failed code " << asyncActionToken.get_reason_code(); }
      break;
    }
    case mqtt::token::SUBSCRIBE:
    {
      { LOG(LogError) << "[MQTT] Subscribing to " << mMqtt.get_server_uri() << " from " << mMqtt.get_client_id() << " failed code " << asyncActionToken.get_reason_code(); }
      break;
    }
    case mqtt::token::PUBLISH:
    {
      { LOG(LogTrace) << "[MQTT] Publishing to " << mMqtt.get_server_uri() << " from " << mMqtt.get_client_id() << " failed code " << asyncActionToken.get_reason_code(); }
      break;
    }
    case mqtt::token::UNSUBSCRIBE:
    case mqtt::token::DISCONNECT:
    default:
    {
      { LOG(LogError) << "[MQTT] Unknown failure in action " << asyncActionToken.get_type() << " to " << mMqtt.get_server_uri() << " from " << mMqtt.get_client_id(); }
      break;
    }
  }
}

void MqttClient::on_success(const mqtt::token& asyncActionToken)
{
  switch(asyncActionToken.get_type())
  {
    case mqtt::token::CONNECT:
    {
      { LOG(LogDebug) << "[MQTT] Connexion to " << mMqtt.get_server_uri() << " from " << mMqtt.get_client_id() << " OK!"; }
      mSender.Send();
      break;
    }
    case mqtt::token::SUBSCRIBE:
    {
      { LOG(LogInfo) << "[MQTT] Subscribing to " << mMqtt.get_server_uri() << " from " << mMqtt.get_client_id() << " OK!"; }
      break;
    }
    case mqtt::token::PUBLISH:
    {
      if (mMqtt.get_client_id() == "recalbox-api-server-broadcaster")
        { LOG(LogTrace) << "[MQTT] Publishing to " << mMqtt.get_server_uri() << " from " << mMqtt.get_client_id() << " OK!"; }
      else
        { LOG(LogInfo) << "[MQTT] Publishing to " << mMqtt.get_server_uri() << " from " << mMqtt.get_client_id() << " OK!"; }
      break;
    }
    case mqtt::token::UNSUBSCRIBE:
    case mqtt::token::DISCONNECT:
    default:
    {
      { LOG(LogInfo) << "[MQTT] Unknown success in action " << asyncActionToken.get_type() << " to " << mMqtt.get_server_uri() << " from " << mMqtt.get_client_id(); }
      break;
    }
  }
}

void MqttClient::Subscribe(const char* topic)
{
  #ifdef FREEZE_MQTT
  return;
  #endif

  if (!mMqtt.is_connected())
  {
    if (std::find(mPendingSubs.begin(), mPendingSubs.end(), String(topic)) == mPendingSubs.end())
      mPendingSubs.push_back(topic);
    { LOG(LogInfo) << "[MQTT] Pending subscribing to: " << topic; }
    return;
  }

  if (mMqtt.subscribe(topic, 0, this, *this) == nullptr)
  {
    LOG(LogError) << "[MQTT] Error subscribing to: " << topic;
    return;
  }
  { LOG(LogInfo) << "[MQTT] Subscribed to: " << topic; }
  mMqtt.set_message_callback([this](mqtt::const_message_ptr msg)
                             {
                               MessageReceived(msg);
                             });
}

void MqttClient::MessageReceived(mqtt::const_message_ptr& msg)
{
  if (mCallbackInterface != nullptr)
    mCallbackInterface->MqttMessageReceived(String(msg->get_topic().data(), (int)msg->get_topic().length()),
                                            String(msg->get_payload_str().data(), (int)msg->get_payload_str().length()));
}

void MqttClient::ReceiveSyncMessage()
{
  String::List pendings = mPendingSubs;
  mPendingSubs.clear();
  for(const String& pending : pendings)
    Subscribe(pending.data());
}

void MqttClient::Wait()
{
  #ifndef FREEZE_MQTT
  if (mOriginalTocken)
    mOriginalTocken->wait();
  #endif
}

void MqttClient::WaitFor(int t)
{
  (void)t;
  #ifndef FREEZE_MQTT
  mOriginalTocken->wait_for(t);
  #endif
}

void MqttClient::Disconnect()
{
  mMqtt.disconnect();
}
