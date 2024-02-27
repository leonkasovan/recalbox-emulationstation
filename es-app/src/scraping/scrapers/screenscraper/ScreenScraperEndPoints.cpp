//
// Created by bkg2k on 17/03/2022.
//
// As part of the RECALBOX Project
// http://www.recalbox.com
//

#include "ScreenScraperEndPoints.h"
#include "EmulationStation.h"
#include "systems/SystemData.h"
#include "utils/network/Url.h"
#include <games/adapter/GameAdapter.h>

String ScreenScraperEndPoints::BuildUrlCommon(ScreenScraperEndPoints::Api api, const String& login, const String& password)
{
  // Url
  String result("https://api.screenscraper.fr/api2/");
  // Api
  switch(api)
  {
    case Api::UserInfo: result.Append("ssuserInfos.php?"); break;
    case Api::GameInfo: result.Append("jeuInfos.php?"); break;
  }
  // Format
  result.Append("output=json");
  // Dev
  result.Append("&devid=").Append(XOrTheSpaceSheriff(API_DEV_U, API_DEV_K));
  result.Append("&devpassword=").Append(XOrTheSpaceSheriff(API_DEV_P, API_DEV_K));
  // Software
  result.Append("&softname=").Append(Url::URLEncode("Emulationstation-Recalbox-" + String(PROGRAM_VERSION_STRING).Trim()));
  // Credentials
  result.Append("&ssid=").Append(login);
  result.Append("&sspassword=").Append(password);

  return result;
}

String ScreenScraperEndPoints::GetUserInfoUrl(const String& login, const String& password)
{
  return BuildUrlCommon(Api::UserInfo, login, password);
}

String ScreenScraperEndPoints::GetGameInfoUrl(const String& login, const String& password, const FileData& game,
                                                   const String& crc32, const String& md5, long long int size)
{
  // Build the common part
  String result(BuildUrlCommon(Api::GameInfo, login, password));

  // Add gameinfo properties
  result.Append("&romtype=rom");
  result.Append("&systemeid=").Append(game.System().Descriptor().ScreenScaperID());
  result.Append("&romnom=").Append(Url::URLEncode(GameAdapter(game).ScrapingName()));
  result.Append("&romtaille=").Append(size);
  if (!crc32.empty())
    result.Append("&crc=").Append(crc32);
  if (!md5.empty())
    result.Append("&md5=").Append(md5);

  return result;
}

String ScreenScraperEndPoints::XOrTheSpaceSheriff(const String& _input, const String& key)
{
  String buffer = _input;
  for (int i = (int) _input.size(); --i >= 0;)
    buffer[i] = (char) ((unsigned char)_input[i] ^ (unsigned char)(key[i % key.size()] + (i * 17)));
  return buffer;
}

