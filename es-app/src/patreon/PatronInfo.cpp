//
// Created by digitalLumberjack on 08/05/2022.
//

#include "PatronInfo.h"
#include "utils/network/HttpClient.h"
#include <rapidjson/document.h>
#include <RecalboxConf.h>
#include <recalbox/RecalboxSystem.h>

PatronInfo::PatronInfo(IPatreonNotification* callback)
  : StaticLifeCycleControler<PatronInfo>("PatronInfo")
  , mEvent(*this)
  , mToken(RecalboxConf::Instance().GetRecalboxPrivateKey().Trim(" \t"))
  , mCallback(callback)
  , mResult(PatronAuthenticationResult::Unknown)
  , mLevel(0)
  , mDone(false)
{
  Thread::Start("PatreonThread");
}

void PatronInfo::Initialize()
{
  if (!mToken.empty())
  {
    unsigned int start = SDL_GetTicks();
    while((SDL_GetTicks() - start) < sNetworkTimeout)
    {
      mHttp.SetBearer(mToken);
      String url = sRootDomainName;
      String body;
      url.Append("/userinfo");
      if (mHttp.Execute(url, body))
      {
        bool success = mHttp.GetLastHttpResponseCode() == 200;
        { LOG(LogInfo) << "[Patreon] Request " << (success ? " successful" : "failed"); }
        if (success)
        {
          rapidjson::Document json;
          json.Parse(body.c_str());
          if (!json.HasParseError())
          {
            if (json.HasMember("full_name") && json.HasMember("patron_status"))
            {
              const rapidjson::Value& response = json["full_name"];
              mName = response.GetString();
              bool active = String(json["patron_status"].GetString()) == "active_patron";
              mResult = active ? PatronAuthenticationResult::Patron : PatronAuthenticationResult::FormerPatron;
              { LOG(LogInfo) << "[Patreon] You're a " << (active ? "Patron!" : "Former patron"); }
              mLevel = json["tier_status"].GetInt();
              break;
            }
            else
            {
              { LOG(LogError) << "[Patreon] API error, missing fields"; }
              mResult = PatronAuthenticationResult::ApiError;
              break;
            }
          }
          else
          {
            { LOG(LogError) << "[Patreon] Json parsing error"; }
            mResult = PatronAuthenticationResult::ApiError;
            break;
          }
        }
        else
        {
          if (mHttp.GetLastHttpResponseCode() == 401)
          {
            { LOG(LogError) << "[Patreon] Invalid key!"; }
            mResult = PatronAuthenticationResult::Invalid;
            break;
          }
          else
          {
            { LOG(LogError) << "[Patreon] Http error: " << mHttp.GetLastHttpResponseCode(); }
            mResult = PatronAuthenticationResult::HttpError;
            if (Wait(10)) return; // Wait 10s & retry
          }
        }
      }
      else
      {
        { LOG(LogError) << "[Patreon] Unknown Http error"; }
        mResult = PatronAuthenticationResult::HttpError;
        if (Wait(1)) return; // Wait 1s & retry
      }
    }

    if (!RecalboxSystem::hasIpAdress(false))
    {
      { LOG(LogError) << "[Patreon] No network"; }
      mResult = PatronAuthenticationResult::NetworkError;
    }
  }

  if (mResult == PatronAuthenticationResult::Unknown)
    mResult = PatronAuthenticationResult::NoPatron;

  mEvent.Send();
}

void PatronInfo::ReceiveSyncMessage()
{
  // No need to check event content, there is only one use case
  if (mCallback != nullptr)
    mCallback->PatreonState(mResult, mLevel, mName);
}

void PatronInfo::WaitForAuthentication(Thread& caller) const
{
  while(!mDone && caller.IsRunning())
    Thread::Sleep(1000);
}

bool PatronInfo::Wait(int second) const
{
  static constexpr int sTimeSliceMs = 200;
  for(second *= 1000 / sTimeSliceMs; --second >= 0; )
    if (!IsRunning())
      return true;
    else
      usleep(sTimeSliceMs * 1000);
  return false;
}
