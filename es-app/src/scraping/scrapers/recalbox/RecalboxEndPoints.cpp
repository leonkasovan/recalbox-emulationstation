//
// Created by bkg2k on 17/03/2022.
//
// As part of the RECALBOX Project
// http://www.recalbox.com
//

#include "RecalboxEndPoints.h"
#include "scraping/ScraperSeamless.h"
#include "Upgrade.h"
#include "hardware/Board.h"
#include "systems/SystemData.h"
#include "utils/network/Url.h"
#include <games/adapter/GameAdapter.h>

String RecalboxEndPoints::GetUserInfoUrl(const String& login, const String& password)
{
  (void)login;
  (void)password;

  String url = GetUrlBase();
  url.Append("/api/authenticated")
     .Append(BuildQueryString(nullptr, 0));

  return url;
}

String
RecalboxEndPoints::GetGameInfoUrlByMD5(const String& login, const String& password, const FileData& game,
                                       const String& md5, long long int size)
{
  (void)login;
  (void)password;
  (void)size;
  (void)game;

  String url = GetUrlBase();
  url.Append("/api/game/bymd5/")
     .Append(md5)
     .Append(BuildQueryString(&game, size));

  return url;
}

String
RecalboxEndPoints::GetGameInfoUrlByName(const String& login, const String& password, const FileData& game,
                                        const String& md5, long long int size)
{
  (void)login;
  (void)password;

  String url = GetUrlBase();

  url.Append("/api/game/bysystem/")
     .Append(game.System().Descriptor().ScreenScaperID())
     .Append("/andname/")
     .Append(Url::URLEncode(GameAdapter(game).ScrapingName()))
     .Append("/andsize/")
     .Append(size)
     .Append(BuildQueryString(&game, size));
  if (!md5.empty())
    url.Append("&md5=")
       .Append(md5);

  return url;
}

String RecalboxEndPoints::BuildQueryString(const FileData* game, long long size)
{
  String result("?board=");
  result.Append(Url::URLEncode(mBoard))
        .Append(LEGACY_STRING("&uuid="))
        .Append(Url::URLEncode(mUUID))
        .Append(LEGACY_STRING("&version="))
        .Append(Url::URLEncode(mVersion));
  if (game != nullptr)
    result.Append(LEGACY_STRING("&system="))
          .Append(game->System().Descriptor().ScreenScaperID())
          .Append(LEGACY_STRING("&systemname="))
          .Append(Url::URLEncode(game->System().Name()))
          .Append(LEGACY_STRING("&gamename="))
          .Append(Url::URLEncode(GameAdapter(*game).ScrapingName()))
          .Append(LEGACY_STRING("&size="))
          .Append(size);

  return result;
}

const ScreenScraperUser* RecalboxEndPoints::GetDirectUserObject() const
{
  static ScreenScraperUser sUser(ScraperSeamless::sScrapingEngineCount);

  return &sUser;
}

void RecalboxEndPoints::AddQueryParametersToMediaRequest(const FileData* game, long long size, String& url)
{
  url.Append(BuildQueryString(game, size));
}

RecalboxEndPoints::RecalboxEndPoints()
  : mUUID(Files::LoadFile(Path(Upgrade::sLocalUUID)).Trim())
  , mBoard()
  , mVersion(Files::LoadFile(Path(Upgrade::sLocalVersionFile)).Trim())
  , mServerIndex(0)
  , mErrors(0)
{
  switch(Board::Instance().GetBoardType())
  {
    case BoardType::UndetectedYet:        mBoard = "undetected"; break;
    case BoardType::Unknown:              mBoard = "unknown"; break;
    case BoardType::Pi0:                  mBoard = "RPi zero"; break;
    case BoardType::Pi02:                 mBoard = "RPi zero 2"; break;
    case BoardType::Pi1:                  mBoard = "RPi 1"; break;
    case BoardType::Pi2:                  mBoard = "RPi 2"; break;
    case BoardType::Pi3:                  mBoard = "RPi 3"; break;
    case BoardType::Pi3plus:              mBoard = "RPi 3B+"; break;
    case BoardType::Pi4:                  mBoard = "RPi 4"; break;
    case BoardType::Pi400:                mBoard = "RPi 400"; break;
    case BoardType::Pi5:                  mBoard = "RPi 5"; break;
    case BoardType::UnknownPi:            mBoard = "unknown RPi"; break;
    case BoardType::OdroidAdvanceGo:      mBoard = "Odroid GO Advance"; break;
    case BoardType::OdroidAdvanceGoSuper: mBoard = "Odroid GO Super"; break;
    case BoardType::PCx86:                mBoard = "x86"; break;
    case BoardType::PCx64:                mBoard = "x64"; break;
    case BoardType::RG351P:               mBoard = "RG351P/M"; break;
    case BoardType::RG353P:               mBoard = "RG353P"; break;
    case BoardType::RG353V:               mBoard = "RG353V"; break;
    case BoardType::RG353M:               mBoard = "RG353M"; break;
    case BoardType::RG503:                mBoard = "RG503"; break;
    case BoardType::RG351V:               mBoard = "RG351V"; break;
  }

  String servers = mDns.GetTxtRecord(sRootDomainName);
  if (!servers.empty())
  {
    mServers = servers.Split('|', false);
    srand(time(nullptr));
    mServerIndex = rand() % mServers.size();
    { LOG(LogDebug) << "[RecalboxEndpoints] Selecting server " << mServerIndex+1 << "/" << mServers.size() << " : " << mServers[mServerIndex]; }
  }
}

String RecalboxEndPoints::GetUrlBase()
{
  if (mServers.empty()) return sRootDomainName;
  return mServers[mServerIndex];
}

void RecalboxEndPoints::NotifyError()
{
  Mutex::AutoLock lock(mErrorLocker);
  if (++mErrors >= 5)
    if (++mServerIndex >= (int)mServers.size())
      mServerIndex = 0;
}
