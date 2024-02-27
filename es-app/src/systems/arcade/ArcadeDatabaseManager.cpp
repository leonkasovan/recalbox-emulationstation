//
// Created by bkg2k on 12/03/23.
//

#include "ArcadeDatabaseManager.h"
#include "emulators/EmulatorManager.h"
#include "ArcadeVirtualSystems.h"
#include <utils/Files.h>
#include <systems/SystemManager.h>

ArcadeDatabaseManager::ArcadeDatabaseManager(SystemData& parentSystem)
  : mSystem(parentSystem)
  , mReady(false)
{
}

ArcadeDatabaseManager::~ArcadeDatabaseManager()
{
  for(const auto& kv : mDatabases)
    delete kv.second;
}

void ArcadeDatabaseManager::LoadDatabases()
{
  if (mReady) return;

  // Valid arcade system?
  if (!mSystem.Descriptor().IsTrueArcade()) return;
  if (!mSystem.HasGame()) return;

  // Load all possible database
  for(int e = mSystem.Descriptor().EmulatorTree().Count(); --e >= 0;)
  {
    const EmulatorDescriptor& emulator = mSystem.Descriptor().EmulatorTree().EmulatorAt(e);
    for (int c = emulator.CoreCount(); --c >= 0;)
      if (!emulator.CoreFlatDatabase(c).empty())
        mDatabases[String(emulator.Name()).Append('|').Append(emulator.CoreNameAt(c))] =
          LoadFlatDatabase(emulator.Name(), emulator.CoreNameAt(c), emulator.CoreFlatDatabase(c), emulator.CoreIgnoreDrivers(c));
  }

  AssignNames();

  mReady = true;
}

ArcadeDatabase* ArcadeDatabaseManager::LoadFlatDatabase([[maybe_unused]] const String& emulator, [[maybe_unused]] const String& core, const String& databaseFilename, const String& ignoredManufacturerString)
{
  // Get database
  Path database("/recalbox/system/arcade/flats/" + databaseFilename);

  // Build special sets
  HashSet<String> ignoredManufacturers;
  for(const String& ignored : ignoredManufacturerString.Split(','))
    ignoredManufacturers.insert_unique(ignored);

  // Load all lines
  String::List lines = Files::LoadAllFileLines(database);
  if (!lines.empty()) lines.erase(lines.begin()); // Remove header
  if (lines.empty())
  {
    LOG(LogError) << "[Arcade] Invalid database: " << database.ToString();
    return new ArcadeDatabase();
  }

  // Setup romname => FileData map for fast lookup
  HashMap<String, FileData*> map;
  class MapBuilder : public IParser
  {
    private:
      HashMap<String, FileData*>* mMap;

    public:
      explicit MapBuilder(HashMap<String, FileData*>& map) : mMap(&map) {}

      void Parse(FileData& game) override
      {
        if (game.IsGame())
          mMap->insert_unique(game.Metadata().RomFileOnly().FilenameWithoutExtension(), &game);
      }
  } Mapper(map);
  mSystem.MasterRoot().ParseAllItems(Mapper);

  // Prepare raw manufacturers
  ManufacturerMap rawManufacturers;
  rawManufacturers.insert_unique("", { "", 0, 0 }); // all "others"
  int nextIndex = 1;

  // Build game array
  Array<ArcadeGame> games((int)lines.size(), 1, false); // Allocate all
  for(int i = (int)lines.size(); --i >= 0; )
    if (const String& line = lines[i]; !line.empty())
      DeserializeTo(games, line, map, rawManufacturers, ignoredManufacturers, nextIndex);

  // Build final manufacturers
  ArcadeDatabase::ManufacturerLists finalDrivers = BuildAndRemapManufacturers(rawManufacturers, games, nextIndex);

  #if DEBUG
  { printf("%s\n", (String("First most populated arcade systems for ") + emulator + "-" + core).c_str()); }
  for(int i = 1; i < (int)finalDrivers.mLimited.size(); ++i)
  { printf("%s\n", (String("  #") + i + " - " + finalDrivers.mLimited[i]).c_str()); }
  #endif

  return new ArcadeDatabase(std::move(finalDrivers.mRaw), std::move(finalDrivers.mLimited), std::move(games));
}

void ArcadeDatabaseManager::DeserializeTo(Array<ArcadeGame>& games, const String& line, const HashMap<String, FileData*>& map,
                                          ManufacturerMap& manufacturerMap, const HashSet<String>& ignoreDrivers, int& nextDriverIndex)
{
  // Field positions
  enum
  {
    fZip = 0,
    fName,
    fType,
    fDriver,
    fRotation,
    fWidth,
    fHeight,
    fFrequency,
    fBios,
    fParent,
    fStatus,
    fManufacturer,
    fFieldCount,
  };

  // Get fields
  String::List fields = line.Split('|');
  if (fields.size() < fFieldCount)
  { LOG(LogError) << "[Arcade] Invalid line: " << line; return; }

  // Try to lookup game
  FileData** fileData = map.try_get(fields[fZip]);
  if (fileData == nullptr) return; // This game is not in the rom folders
  FileData* realGame = *fileData;

  // Parent?
  FileData** parentNull = map.try_get(fields[fParent]);
  FileData* parent = parentNull != nullptr ? *parentNull : nullptr;

  // Raw manufacturer
  ArcadeGame::RawManufacturerHolder rawManufacturers;
  String manufacturers = fields[fManufacturer];
  String manufacturer;
  manufacturers.LowerCase();
  for(;;)
  {
    bool again = true;
    if (!manufacturers.Extract('/', manufacturer, manufacturers, true))
    {
      manufacturer = manufacturers;
      again = false;
    }

    if (ignoreDrivers.contains(manufacturer)) manufacturer.clear(); // Sent to "others" manufacturer 0
    RawManufacturer* rawManufacturer = manufacturerMap.try_get(manufacturer);
    if (rawManufacturer == nullptr)
    {
      manufacturerMap.insert(manufacturer, {manufacturer, nextDriverIndex++, 0 });
      rawManufacturer = manufacturerMap.try_get(manufacturer);
    }
    rawManufacturer->mGameCount++;
    rawManufacturers.Add(rawManufacturer->mIndex);
    // Has sub system?
    if (int pos = manufacturer.Find('\\'); pos >= 0)
    {
      manufacturer.Delete(pos, INT32_MAX);
      rawManufacturer = manufacturerMap.try_get(manufacturer);
      if (rawManufacturer == nullptr)
      {
        manufacturerMap.insert(manufacturer, {manufacturer, nextDriverIndex++, 0 });
        rawManufacturer = manufacturerMap.try_get(manufacturer);
      }
      rawManufacturer->mGameCount++;
      rawManufacturers.Add(rawManufacturer->mIndex);
    }
    if (!again) break;
  }

  // Data
  ArcadeGame::Rotation rotation = ArcadeGame::RotationFromString(fields[fRotation]);
  ArcadeGame::Type type = ArcadeGame::TypeFromString(fields[fType], parent != nullptr);
  ArcadeGame::Status status = ArcadeGame::StatusFromString(fields[fStatus]);
  unsigned short width = fields[fWidth].AsInt();
  unsigned short height = fields[fHeight].AsInt();

  games.Add(ArcadeGame(realGame, parent, fields[fName], rawManufacturers, type, status, rotation, width, height));
  //lookups.insert(realGame, &games(games.Count() - 1));
}

ArcadeDatabase::ManufacturerLists ArcadeDatabaseManager::BuildAndRemapManufacturers(const ManufacturerMap& rawManufacturers, Array<ArcadeGame>& games, int rawDriverCount)
{
  // Build and sort a list of all manufacturers
  std::vector<RawManufacturer> sortedDrivers;
  for(const auto& kv : rawManufacturers)
    sortedDrivers.push_back(kv.second);
  std::sort(sortedDrivers.begin(), sortedDrivers.end(), [](const RawManufacturer& a, const RawManufacturer& b) { return a.mGameCount > b.mGameCount; });

  // Build final list by selecting the top most used manufacturers
  // Also build a remap list
  String::List limitedDriverList;
  Array<int> indexRemapper(rawDriverCount);
  indexRemapper.Insert(0, 0, rawDriverCount);
  limitedDriverList.push_back(String::Empty); // Driver 0 = all others
  for(int i = 0; i < sManufacturerLimits; ++i)
  {
    if (i >= (int)sortedDrivers.size()) break;
    const RawManufacturer& manufacturer = sortedDrivers[i];
    if (manufacturer.mIndex == 0) continue; // Driver 0 has already been added, ignore it
    indexRemapper(manufacturer.mIndex) = (int)limitedDriverList.size();
    limitedDriverList.push_back(manufacturer.mName);
  }
  // Force manufacturers used in virtual system to appear, even if the limit is already reached
  for(const RawManufacturer& manufacturer : sortedDrivers)
  {
    if (indexRemapper[manufacturer.mIndex] != 0) continue; // Already kept
    bool found = false;
    for(const String& virtualDriver : ArcadeVirtualSystems::GetVirtualArcadeSystemList())
      if (virtualDriver == manufacturer.mName) { found = true; break; }
    if (found)
    {
      indexRemapper(manufacturer.mIndex) = (int)limitedDriverList.size();
      limitedDriverList.push_back(manufacturer.mName);
    }
  }

  // Log
  if (Log::ReportingLevel() >= LogLevel::LogDebug)
  {
    String log("[Arcade] Most used manufacturers: ");
    for (int i = 0; i < (int) limitedDriverList.size(); ++i)
      log.Append(i == 0 ? "" : ",").Append(limitedDriverList[i]);
    LOG(LogDebug) << log;
  }

  // Remap indexes using the remapper
  for(int i = games.Count(); --i >= 0;)
    for(int m = games[i].RawManufacturer().Count(); --i >= 0; )
      games(i).AddLimitedManufacturer(indexRemapper[games[i].RawManufacturer().Manufacturer(m)]);

  // Count "all others". If noone in this manufacturer, remove it and decrease all manufacturers
  int zeroCount = 0;
  for(int i = games.Count(); --i >= 0;)
    if (games(i).LimitedManufacturer().HasOnlyZero())
      zeroCount++;
  if (zeroCount == 0)
  {
    limitedDriverList.erase(limitedDriverList.begin());
    for(int i = games.Count(); --i >= 0;)
      for(int m = games[i].RawManufacturer().Count(); --i >= 0; )
        games(i).DecLimitedManufacturerAt(m);
  }

  // Build raw list
  sortedDrivers.clear();
  for(const auto& kv : rawManufacturers)
    sortedDrivers.push_back(kv.second);
  // resort raw manufacturer using manufacturer index
  std::sort(sortedDrivers.begin(), sortedDrivers.end(), [](const RawManufacturer& a, const RawManufacturer& b) { return a.mIndex < b.mIndex; });
  // build final list
  String::List rawDriverList;
  for(const RawManufacturer& manufacturer : sortedDrivers)
    rawDriverList.push_back(manufacturer.mName);

  return ArcadeDatabase::ManufacturerLists(limitedDriverList, rawDriverList);
}

void ArcadeDatabaseManager::AssignNames()
{
  class Renaming : public IParser
  {
    private:
      Databases& mDatabase;

    public:
      explicit Renaming(Databases& databases) : mDatabase(databases) {}

      void Parse(FileData& game) override
      {
        String emulator;
        String core;
        if (game.IsGame())
          // Game is unamed?
          if (String name = game.Metadata().Name(); name.empty() || name == game.Metadata().RomFileOnly().FilenameWithoutExtension())
            // Get emulator & core for this game, regarding all default and override configurations
            if (EmulatorManager::GetGameEmulator(game, emulator, core))
              // Lookup emulator hashmap
              if (ArcadeDatabase** gameDatabase = mDatabase.try_get(emulator.Append('|').Append(core)); gameDatabase != nullptr)
                // Has Game a matching ArcadeGame?
                if (const ArcadeGame* arcadeGame = (*gameDatabase)->LookupGame(game); arcadeGame != nullptr)
                  game.Metadata().SetName(arcadeGame->ArcadeName());
      }
  } Rename(mDatabases);
  if (mSystem.Name() == "atomiswave")
    mSystem.MasterRoot().ParseAllItems(Rename);
}

const ArcadeDatabase* ArcadeDatabaseManager::LookupDatabase() const
{
  String emulator;
  String core;
  if (EmulatorManager::GetDefaultEmulator(mSystem, emulator, core))
  {
    ArcadeDatabase** database = mDatabases.try_get(emulator.Append('|').Append(core));
    if (database != nullptr) return *database;
  }
  return nullptr;
}

const ArcadeDatabase* ArcadeDatabaseManager::LookupDatabase(const FolderData& folder, String& emulatorName, String& coreName) const
{
  emulatorName.clear();
  coreName.clear();
  if (EmulatorManager::GetGameEmulator(folder, emulatorName, coreName))
  {
    ArcadeDatabase** database = mDatabases.try_get(emulatorName.Append('|').Append(coreName));
    if (database != nullptr) return *database;
  }
  return nullptr;
}

const ArcadeDatabase* ArcadeDatabaseManager::LookupDatabase(const FileData& game, String& emulatorName, String& coreName) const
{
  emulatorName.clear();
  coreName.clear();
  if (EmulatorManager::GetGameEmulator(game, emulatorName, coreName))
  {
    ArcadeDatabase** database = mDatabases.try_get(emulatorName.Append('|').Append(coreName));
    if (database != nullptr) return *database;
  }
  return nullptr;
}

const ArcadeDatabase* ArcadeDatabaseManager::LookupDatabase(const FolderData& folder) const
{
  String emulatorName;
  String coreName;
  if (EmulatorManager::GetGameEmulator(folder, emulatorName, coreName))
  {
    ArcadeDatabase** database = mDatabases.try_get(emulatorName.Append('|').Append(coreName));
    if (database != nullptr) return *database;
  }
  return nullptr;
}

void ArcadeDatabaseManager::RemoveGame(const FileData& game)
{
  for(const auto& database : mDatabases)
    database.second->Remove(game);
}

const ArcadeDatabase* ArcadeDatabaseManager::LookupDatabaseFor(const String& emulatorName, const String& coreName) const
{
  ArcadeDatabase** database = mDatabases.try_get(String(emulatorName).Append('|').Append(coreName));
  if (database != nullptr) return *database;
  return nullptr;
}



