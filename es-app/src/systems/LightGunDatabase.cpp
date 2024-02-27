//
// Created by bkg2k on 08/03/2021.
//
// As part of the RECALBOX Project
// http://www.recalbox.com
//

#include "LightGunDatabase.h"

LightGunDatabase::LightGunDatabase()
  : mCurrentSystem(nullptr)
  , mCurrentList(nullptr)
{
  LoadDatabase();
}

void LightGunDatabase::SetCurrentSystem(const SystemData& system)
{
  if (mCurrentSystem == &system) return;
  mCurrentSystem = &system;
  mCurrentList = mSystemLists.try_get(system.Name());
  { LOG(LogDebug) << "[LightGun] System " << system.Name() << " selected."; }
}

bool LightGunDatabase::ApplyFilter(const FileData& file)
{
  String name = GetSimplifiedName(file.Name().empty() ? file.RomPath().FilenameWithoutExtension() : file.Name());
  SetCurrentSystem(file.System());
  if (mCurrentList != nullptr)
  {
    for (const String& gamename: *mCurrentList)
      if (name.Contains(gamename))
      {
        { LOG(LogTrace) << "[LightGun] Game " << file.Name() << " match database name " << gamename; }
        return true;
      }
  }
  return false;
}

String LightGunDatabase::GetSimplifiedName(const String& name)
{
  String result(name);

  // Kill decorations
  int pos1 = result.Find('(');
  int pos2 = result.Find('[');
  if ((pos2 >= 0) && (pos2 < pos1)) pos1 = pos2;
  if (pos1 >= 0) result = result.SubString(0, pos1);

  // Crunch non a-z,0-9,! characters
  int writeIndex = 0;
  for(int i = -1; ++i < (int)result.size(); )
  {
    unsigned int c = result[i] | 0x20; // upper => lowercase letters, no effect on digits
    if (((unsigned int)(c - 0x30) <= 9) || ((unsigned int)(c - 0x61) <= 25) || (c == ('!' | 0x20)))
      result[writeIndex++] = (char)c;
  }
  result.resize(writeIndex);

  //{ LOG(LogDebug) << "[LightGun] " << name << " = " << result; }

  return result;
}

void LightGunDatabase::LoadDatabase()
{
  XmlDocument document;
  XmlResult result = document.load_file(sXmlPath);
  if (!result)
  {
    { LOG(LogError) << "[LightGun] Could not parse " << sXmlPath << " file!"; }
    return;
  }

  XmlNode root = document.child("root");
  if (root != nullptr)
    for (const XmlNode& system : root.children("system"))
    {
      String systemNames = Xml::AttributeAsString(system, "name", "<missing name>");
      for(const String& systemName : systemNames.Split('|', true))
      {
        { LOG(LogDebug) << "[LightGun] Load system: " << systemName; }

        // Create system list
        String::List& gameList = mSystemLists[systemName];

        // Run through games
        String gameName;
        for (const XmlNode& games: system.children("gameList"))
          for (const XmlNode& game: games.children("game"))
          {
            gameName = Xml::AttributeAsString(game, "name", "");
            if (!gameName.empty() && Xml::AttributeAsBool(game, "tested", false))
              gameList.push_back(gameName);
          }
        { LOG(LogDebug) << "[LightGun] " << gameList.size() << " games found in system " << systemName; }
      }
    }
}
