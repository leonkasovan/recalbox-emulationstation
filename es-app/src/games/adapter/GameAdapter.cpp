//
// Created by bkg2k on 22/06/22.
//
// As part of the RECALBOX Project
// http://www.recalbox.com
//

#include "GameAdapter.h"
#include "GameNameMapManager.h"
#include "games/GameFilesUtils.h"
#include <systems/SystemData.h>

const String GameAdapter::sEasyRPGSystemName(LEGACY_STRING("easyrpg"));
const String GameAdapter::sEasyRPGGameNameLower(LEGACY_STRING("rpg_rt.ini"));

String GameAdapter::ScrapingName() const
{
  if (mGame.System().Name() == sEasyRPGSystemName) // TODO: move to String & remove cast
  {
    if (mGame.RomPath().Filename().ToLowerCase() == sEasyRPGGameNameLower)
    {
      IniFile ini(mGame.RomPath(), false, false);
      String gameName = GameFilesUtils::RemoveParenthesis(ini.AsString("GameTitle"));
      if (!gameName.empty()) return gameName;
    }
  }
  if (mGame.System().Name() == "daphne" && mGame.RomPath().Filename().EndsWith(".daphne"))
    return mGame.RomPath().Filename().Remove(".daphne");

  return mGame.RomPath().Filename();
}

String GameAdapter::DisplayName() const
{
  if (!mGame.Name().empty()) return mGame.Name();

  return RawDisplayName(mGame.System(), mGame.RomPath());
}

String GameAdapter::RawDisplayName(SystemData& system, const Path& rompath)
{
  if (system.Name() == sEasyRPGSystemName)
  {
    if (rompath.Filename().ToLowerCase() == sEasyRPGGameNameLower)
    {
      IniFile ini(rompath, false, false);
      String gameName = GameFilesUtils::RemoveParenthesis(ini.AsString("GameTitle"));
      if (!gameName.empty()) return gameName;
    }
  }
  else if (GameNameMapManager::HasRenaming(system))
  {
    String gameName(GameNameMapManager::Rename(system, rompath.FilenameWithoutExtension()));
    if (!gameName.empty()) return gameName;
  }

  return rompath.FilenameWithoutExtension();
}

long long GameAdapter::RomSize()
{
  Path romPath(mGame.RomPath());
  return romPath.IsFile() ? romPath.Size() : 0L;
}
