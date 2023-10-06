//
// Created by bkg2k on 03/04/2020.
//

#include <utils/String.h>
#include "GameNameMapManager.h"

void GameNameMapManager::GenerateMameNameHashes()
{
  for(int i = sMameListSize; --i >=0; )
    mMameNameHashes[i] = String::Hash(mMameNameToRealName[i * 2]);
}

void GameNameMapManager::GenerateFlashbackNameHashes()
{
  for(int i = sFlashbackListSize; --i >=0; )
    mFlashbackNameHashes[i] = String::Hash(mFlashbackNameToRealName[i * 2]);
}

const char* GameNameMapManager::GetCleanMameName(const String& from)
{
  static bool HashGenerated = false;
  if (!HashGenerated)
  {
    GenerateMameNameHashes();
    HashGenerated = true;
  }

  int hash = from.Hash();
  for(int i = sMameListSize; --i >= 0; )
    if (mMameNameHashes[i] == hash)
      if (strcmp(from.c_str(), mMameNameToRealName[i << 1]) == 0)
        return mMameNameToRealName[(i << 1) + 1];

  return nullptr;
}


const char* GameNameMapManager::GetCleanFlashbackName(const String& from)
{
  static bool HashGenerated = false;
  if (!HashGenerated)
  {
    GenerateFlashbackNameHashes();
    HashGenerated = true;
  }

  int hash = from.Hash();
  for(int i = sFlashbackListSize; --i >= 0; )
    if (mFlashbackNameHashes[i] == hash)
      if (strcmp(from.c_str(), mFlashbackNameToRealName[i << 1]) == 0)
        return mFlashbackNameToRealName[(i << 1) + 1];

  return nullptr;
}

bool GameNameMapManager::HasRenaming(const SystemData& system)
{
  return system.Descriptor().IsTrueArcade() ||
         system.Descriptor().Name() == "neogeo" ||
         system.Descriptor().Name() == "flashback";
}

bool GameNameMapManager::HasFiltering(const SystemData& system)
{
  return system.Descriptor().IsTrueArcade() ||
         system.Descriptor().Name() == "neogeo";
}

bool GameNameMapManager::IsFiltered(const SystemData& system, const String& filenameWoExt)
{
  if (HasFiltering(system))
  {
    return (mMameBios.contains(filenameWoExt) || mMameDevices.contains(filenameWoExt));
  }
  return false;
}

String GameNameMapManager::Rename(const SystemData& system, const String& filenameWoExt)
{
  if (HasRenaming(system))
  {
    if (system.Descriptor().IsTrueArcade() || system.Descriptor().Name() == "neogeo")
    {
      const char* newName = GetCleanMameName(filenameWoExt);
      return newName != nullptr ? String(newName) : filenameWoExt;
    }
    if (system.Descriptor().Name() == "flashback")
    {
      const char* newName = GetCleanFlashbackName(filenameWoExt.ToLowerCase());
      return newName != nullptr ? String(newName) : filenameWoExt;
    }
  }
  { LOG(LogError) << "[GameName] Renaming called on non-renamed platform game: " << filenameWoExt; }
  return filenameWoExt;
}
