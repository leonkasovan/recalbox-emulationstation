//
// Created by bkg2k on 17/06/23.
//

#include "DatManager.h"

DatManager::DatManager(const SystemData& system)
{
  LoadAllDats(system);
}

DatManager::~DatManager()
{
  for(auto& kv : mDatPerEmulatorCore)
    delete kv.second;
}

void DatManager::LoadAllDats(const SystemData& system)
{
  // Valid arcade system?
  if (!system.Descriptor().IsTrueArcade()) return;
  if (!system.HasGame()) return;

  // Load all possible database
  for (int e = system.Descriptor().EmulatorTree().Count(); --e >= 0;)
  {
    const EmulatorDescriptor& emulator = system.Descriptor().EmulatorTree().EmulatorAt(e);
    for (int c = emulator.CoreCount(); --c >= 0;)
      LoadDat(emulator.Name(), emulator.CoreNameAt(c), emulator.CoreFlatDatabase(c));
  }
}

void DatManager::LoadDat(const String& emulator, const String& core, const String& databaseFilename)
{
  Path flatDatabasePath = Path("/recalbox/system/arcade/flats/" + databaseFilename).ChangeExtension(".fdt");
  mDatPerEmulatorCore[Key(emulator, core)] = new DatContent(flatDatabasePath);
}

ScanResult DatManager::Scan(const FileData& game)
{
  ScanResult result;

  // Run through DatContents
  for(const auto& kv : mDatPerEmulatorCore)
    if (String emulator, core; kv.first.Extract('|', emulator, core, false))
    {
      ScanResult::Result::RomFailList failures;
      bool ok = kv.second->Scan(game, failures);
      ScanResult::Status status = ok ? ScanResult::Status::Unknown : ScanResult::Status::NotSupported;
      if (ok) // Game match database,; adjust emulation status
        if (const ArcadeDatabase* database = game.System().ArcadeDatabases().LookupDatabaseFor(emulator, core); database != nullptr)
          if (const ArcadeGame* arcade = database->LookupGame(game))
            switch(arcade->EmulationStatus())
            {
              case ArcadeGame::Status::Good: status = ScanResult::Status::Good; break;
              case ArcadeGame::Status::Imperfect: status = ScanResult::Status::Imperfect; break;
              case ArcadeGame::Status::Preliminary: status = ScanResult::Status::Preliminar; break;
              case ArcadeGame::Status::Unknown:
              default: break;
            }
      result.mResults.push_back(ScanResult::Result(emulator, core, status, std::move(failures)));
    }

  return result;
}

