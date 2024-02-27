//
// Created by Bkg2k on 18/02/2020.
//

#include <utils/locale/LocaleHelper.h>
#include "EmulatorManager.h"

bool EmulatorManager::GetDefaultEmulator(const SystemData& system, String& emulator, String& core)
{
  { LOG(LogTrace) << "[Emulator] Get default system's emulator for " << system.FullName(); }
  bool Ok = GetSystemDefaultEmulator(system, emulator, core);
  if (!Ok) { LOG(LogTrace) << "[Emulator] Cannot get default emulator!"; }
  return Ok;
}

bool EmulatorManager::GetSystemEmulator(const SystemData& system, String& emulator, String& core)
{
  { LOG(LogTrace) << "[Emulator] Get system's emulator for " << system.FullName(); }
  bool Ok = GetSystemDefaultEmulator(system, emulator, core);
  if (Ok) GetEmulatorFromConfigFile(system, emulator, core);
  if (!Ok) { LOG(LogTrace) << "[Emulator] Cannot get system emulator!"; }
  return Ok;
}

bool EmulatorManager::GetGameEmulatorOverriden(const FileData& game, String& emulator, String& core, bool& byOverride)
{
  { LOG(LogTrace) << "[Emulator] Get game's emulator for " << game.RomPath().ToString(); }

  byOverride = false;

  // Get default emulator first
  bool Ok = GetSystemDefaultEmulator(game.System(), emulator, core);
  if (Ok)
  {
    // Then from the general config file
    GetEmulatorFromConfigFile(game.System(), emulator, core);

    // Then automatic
    GetEmulatorFromSubfolder(game, emulator, core);

    // Then from the gamelist.xml file
    GetEmulatorFromGamelist(game, emulator, core);

    // Then from file overrides
    if (GetEmulatorFromOverride(game, emulator, core))
      byOverride = true;
  }
  else { LOG(LogError) << "[Emulator] Cannot get default emulator!"; }

  { LOG(LogTrace) << "[Emulator] Final game's emulator for " << game.RomPath().ToString() << " : " << emulator << "-" << core; }

  return Ok;
}

EmulatorData EmulatorManager::GetGameEmulator(const FileData& game)
{
  String emulator;
  String core;

  if (GetGameEmulator(game, emulator, core))
    return EmulatorData(emulator, core);

  return EmulatorData("", "");
}

bool EmulatorManager::GetSystemDefaultEmulator(const SystemData& system, String& emulator,
                                                     String& core)
{
  emulator.clear();
  core.clear();
  const bool isCRT = Board::Instance().CrtBoard().IsCrtAdapterAttached();
  const EmulatorList& list = system.Emulators();
  if (list.HasAny())
  {
    int priority = 255;
    // Search for lowest priority core/emulator
    for(int emulatorIndex = list.Count(); --emulatorIndex >= 0; )
      for(int coreIndex = list.EmulatorAt(emulatorIndex).CoreCount(); --coreIndex >= 0; )
        if (list.EmulatorAt(emulatorIndex).CorePriorityAt(coreIndex) < priority && ((isCRT && list.EmulatorAt(emulatorIndex).CoreCrtAvailable(coreIndex)) || !isCRT))
        {
          priority = list.EmulatorAt(emulatorIndex).CorePriorityAt(coreIndex);
          core = list.EmulatorAt(emulatorIndex).CoreNameAt(coreIndex);
          emulator = list.EmulatorAt(emulatorIndex).Name();
        }
    // Should not, but can happen if a system has no core having crt.available=1, so we use the first core to avoid empty values
    if(core.empty() && emulator.empty())
    {
      core = list.EmulatorAt(0).CoreNameAt(0);
      emulator = list.EmulatorAt(0).Name();
      { LOG(LogWarning) << "[Emulator]  You added a system without a core configured for CRT " << emulator; }
    }
    { LOG(LogTrace) << "[Emulator]   From SystemList: " << emulator << '/' << core; }
    return true;
  }
  return false;
}

void EmulatorManager::GetEmulatorFromSubfolder(const FileData& game, String& emulator, String& core)
{
  if (game.System().Name() == "mame")
  {
    // Make rom path relative to the system folder
    bool ok;
    Path relative = game.RomPath().MakeRelative(game.TopAncestor().RomPath(), ok).Directory();
    // Run through foldert and try to detect a known one
    if (ok && !relative.IsEmpty())
      for(int i = relative.ItemCount(); --i >= 0;)
      {
        static HashMap<String, String> folderToEmulatorCore
        {
          { "mame", "libretro|mame" },
          { "mame2000", "libretro|mame2000" },
          { "mame2003", "libretro|mame2003" },
          { "mame2003+", "libretro|mame2003_plus" },
          { "mame2003plus", "libretro|mame2003_plus" },
          { "mame2003-plus", "libretro|mame2003_plus" },
          { "mame2010", "libretro|mame2010" },
          { "mame2015", "libretro|mame2015" },
          { "mame2016", "libretro|mame2016" },
          { "advancemame", "advancemame|advancemame" },
        };
        if (String* result = folderToEmulatorCore.try_get(relative.Item(i)); result != nullptr)
          if (String newEmulator, newCore; result->Extract('|', newEmulator, newCore, false))
            if (CheckEmulatorAndCore(game.System(), newEmulator, newCore))
            {
              emulator = newEmulator;
              core = newCore;
              break;
            }
      }
  }
}

void EmulatorManager::GetEmulatorFromConfigFile(const SystemData& system, String& emulator, String& core)
{
  String rawemulator = RecalboxConf::Instance().AsString(system.Name() + ".emulator");
  String rawcore = RecalboxConf::Instance().AsString(system.Name() + ".core");
  PatchNames(rawemulator, rawcore);

  // At least one not empty?
  if (!rawemulator.empty() || !rawcore.empty())
  {
    if (CheckEmulatorAndCore(system, rawemulator, rawcore))
    {
      emulator = rawemulator;
      core = rawcore;
      { LOG(LogTrace) << "[Emulator]   From recalbox.conf " << emulator << '/' << core; }
    }
    else
    {
      if (GuessEmulatorAndCore(system, rawemulator, rawcore))
      {
        emulator = rawemulator;
        core = rawcore;
        { LOG(LogTrace) << "[Emulator]   Guessed from recalbox.conf " << emulator << '/' << core; }
      }
    }
  }
}

void EmulatorManager::GetEmulatorFromGamelist(const FileData& game, String& emulator, String& core)
{
  // Get configured emulator/core iif they are both not empty
  if (!game.Metadata().Core().empty() && !game.Metadata().Emulator().empty())
  {
    String rawemulator = game.Metadata().Emulator();
    String rawcore = game.Metadata().Core();
    PatchNames(rawemulator, rawcore);

    if (CheckEmulatorAndCore(game.System(), rawemulator, rawcore))
    {
      emulator = rawemulator;
      core = rawcore;
      { LOG(LogTrace) << "[Emulator]   From Gamelist.xml " << emulator << '/' << core; }
    }
    else
    {
      if (GuessEmulatorAndCore(game.System(), rawemulator, rawcore))
      {
        emulator = rawemulator;
        core = rawcore;
        { LOG(LogTrace) << "[Emulator]   Guessed from Gamelist.xml " << emulator << '/' << core; }
      }
    }
  }
}

bool EmulatorManager::GetEmulatorFromOverride(const FileData& game, String& emulator, String& core)
{
  String rawGlobalEmulator;
  String rawSystemEmulator;
  String rawGlobalCore;
  String rawSystemCore;

  String keyEmulator = game.System().Name() + ".emulator";
  String keyCore = game.System().Name() + ".core";

  // Get game directory
  Path romPath(game.RomPath());
  Path path = game.IsGame() ? romPath.Directory() : romPath;
  // Run through all folder starting from root
  int count = path.ItemCount();
  int start = game.IsRoot() ? count : game.TopAncestor().RomPath().ItemCount();
  for (int i = start; i <= count; ++i)
  {
    IniFile configuration(Path(path.UptoItem(i)) / ".recalbox.conf", false, false);
    if (configuration.IsValid())
    {
      // Get values
      String globalEmulator = configuration.AsString("global.emulator");
      String systemEmulator = configuration.AsString(keyEmulator);
      String globalCore = configuration.AsString("global.core");
      String systemCore = configuration.AsString(keyCore);

      // Record non empty values
      if (!globalEmulator.empty()) rawGlobalEmulator = globalEmulator;
      if (!globalCore.empty()    ) rawGlobalCore     = globalCore;
      if (!systemEmulator.empty()) rawSystemEmulator = systemEmulator;
      if (!systemCore.empty()    ) rawSystemCore     = systemCore;
    }
  }

  // Get file config
  IniFile configuration(romPath.ChangeExtension(romPath.Extension() + ".recalbox.conf"), false, false);
  if (configuration.IsValid())
  {
    // Get values
    String globalEmulator = configuration.AsString("global.emulator");
    String systemEmulator = configuration.AsString(keyEmulator);
    String globalCore = configuration.AsString("global.core");
    String systemCore = configuration.AsString(keyCore);

    // Record non empty values
    if (!globalEmulator.empty()) rawGlobalEmulator = globalEmulator;
    if (!globalCore.empty()    ) rawGlobalCore     = globalCore;
    if (!systemEmulator.empty()) rawSystemEmulator = systemEmulator;
    if (!systemCore.empty()    ) rawSystemCore     = systemCore;
  }

  // Get final tupple
  String finalEmulator = rawSystemEmulator.empty() ? rawGlobalEmulator : rawSystemEmulator;
  String finalCore     = rawSystemCore.empty()     ? rawGlobalCore     : rawSystemCore;
  PatchNames(finalEmulator, finalCore);

  if (!finalEmulator.empty() || !finalCore.empty())
  {
    if (CheckEmulatorAndCore(game.System(), finalEmulator, finalCore))
    {
      emulator = finalEmulator;
      core = finalCore;
      { LOG(LogTrace) << "[Emulator]   From override files" << emulator << '/' << core; }
      return true;
    }
    if (GuessEmulatorAndCore(game.System(), finalEmulator, finalCore))
    {
      emulator = finalEmulator;
      core = finalCore;
      { LOG(LogTrace) << "[Emulator]   Guessed from override files" << emulator << '/' << core; }
      return true;
    }
  }

  return false;
}

bool EmulatorManager::ConfigOverloaded(const FileData& game)
{
  String defaultEmulator;
  String defaultCore;
  String gameEmulator;
  String gameCore;
  GetSystemDefaultEmulator(game.System(), defaultEmulator, defaultCore);
  GetGameEmulator(game, gameEmulator, gameCore);
  return !(defaultEmulator == gameEmulator && defaultCore == gameCore);
}

bool EmulatorManager::CheckEmulatorAndCore(const SystemData& system, const String& emulator, const String& core)
{
  const EmulatorList& list = system.Emulators();
  if (list.HasNamed(emulator))
    if (list.Named(emulator).HasCore(core))
      return true;
  return false;
}

bool EmulatorManager::GuessEmulatorAndCore(const SystemData& system, String& emulator, String& core)
{
  const EmulatorList& list = system.Emulators();
  // Emulator without core
  if (!emulator.empty() && core.empty())
    if (list.HasNamed(emulator))
      if (list.Named(emulator).CoreCount() == 1)
      {
        core = list.Named(emulator).CoreNameAt(0);
        { LOG(LogTrace) << "[Emulator]   Core " << core << " guessed from emulator " << emulator << " which has only one core"; }
        return true;
      }
  // Core w/o emulator
  if (emulator.empty() && !core.empty())
    for(int i = list.Count(); --i >= 0; )
      if (list.EmulatorAt(i).HasCore(core))
      {
        emulator = list.EmulatorAt(i).Name();
        { LOG(LogTrace) << "[Emulator]   Emulator " << emulator << " guessed from core " << core; }
        return true;
      }

  { LOG(LogDebug) << "[Emulator]   Cannot guess core/emulator couple!"; }
  return false;
}

String::List EmulatorManager::GetEmulators(const SystemData& system)
{
  const EmulatorList& list = system.Emulators();
  EmulatorList effectiveList;

  // When on CRT select only emulators that have one core available
  const bool isCRT = Board::Instance().CrtBoard().IsCrtAdapterAttached();
  for(int emulatorIndex = list.Count(); --emulatorIndex >= 0; ){
    bool hasCrtCore = false;
    for(int coreIndex = list.EmulatorAt(emulatorIndex).CoreCount(); --coreIndex >= 0; )
    {
      if(list.EmulatorAt(emulatorIndex).CoreCrtAvailable(coreIndex))
        hasCrtCore = true;
    }
    if(!isCRT || hasCrtCore)
      effectiveList.AddEmulator(list.EmulatorAt(emulatorIndex));
  }

  // Get priorities
  unsigned char emulatorPriorities[EmulatorList::sMaximumEmulators];
  for(int i = EmulatorList::sMaximumEmulators; --i >= 0; ) emulatorPriorities[i] = 255;
  for(int i = effectiveList.Count(); --i >= 0; )
    for(int j = effectiveList.EmulatorAt(i).CoreCount(); --j >= 0; )
      if (effectiveList.EmulatorAt(i).CorePriorityAt(j) < emulatorPriorities[i])
        emulatorPriorities[i] = effectiveList.EmulatorAt(i).CorePriorityAt(j);

  // Build a sorted output list
  String::List result;
  for(int round = effectiveList.Count(); --round >= 0; )
  {
    int lowestPriority = emulatorPriorities[0];
    int index = 0;
    for(int i = 0; i < effectiveList.Count(); ++i)
      if (emulatorPriorities[i] < lowestPriority)
      {
        lowestPriority = emulatorPriorities[i];
        index = i;
      }
    result.push_back(effectiveList.EmulatorAt(index).Name());
    emulatorPriorities[index] = 255;
  }

  return result;
}

String::List EmulatorManager::GetCores(const SystemData& system, const String& emulator)
{
  const bool isCRT = Board::Instance().CrtBoard().IsCrtAdapterAttached();
  const EmulatorList& list = system.Emulators();
  if (list.HasNamed(emulator))
  {
    const EmulatorDescriptor& descriptor = list.Named(emulator);
    // Get priorities
    Array<unsigned char> corePriorities(descriptor.CoreCount());
    for(int i = descriptor.CoreCount(); --i >= 0; ) corePriorities(i) = descriptor.CorePriorityAt(i);

    // Build a sorted output list
    String::List result;
    for(int round = descriptor.CoreCount(); --round >= 0; )
    {
      int lowestPriority = corePriorities[0];
      int index = 0;
      for(int i = 0; i < descriptor.CoreCount(); ++i)
        if (corePriorities[i] < lowestPriority)
        {
          lowestPriority = corePriorities[i];
          index = i;
        }
      corePriorities(index) = 255;
      if(isCRT && !descriptor.CoreCrtAvailable(index))
        continue;
      result.push_back(descriptor.CoreNameAt(index));
    }

    return result;
  }

  return String::List();
}

String EmulatorManager::KeyFrom(const SystemData& system)
{
  return system.Descriptor().GUID();
}

void EmulatorManager::PatchNames(String& emulator, String& core)
{
  if (emulator == "libretro")
    if (core == "duckstation") core = "swanstation";
}
