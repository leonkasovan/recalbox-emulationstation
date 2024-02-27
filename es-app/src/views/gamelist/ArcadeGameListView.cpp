//
// Created by bkg2k on 31/05/23.
//

#include "ArcadeGameListView.h"
#include <systems/arcade/ArcadeVirtualSystems.h>
#include "utils/locale/LocaleHelper.h"

ArcadeGameListView::ArcadeGameListView(WindowManager& window, SystemManager& systemManager, SystemData& system)
  : DetailedGameListView(window, systemManager, system)
  , mDatabase(nullptr)
{
}

void ArcadeGameListView::populateList(const FolderData& folder)
{
  mPopulatedFolder = &folder;

  // Default filter
  FileData::Filter includesFilter = FileData::Filter::Normal | FileData::Filter::Favorite;
  // Favorites only?
  if (RecalboxConf::Instance().GetFavoritesOnly()) includesFilter = FileData::Filter::Favorite;

  // Get items
  bool flatfolders = mSystem.IsAlwaysFlat() || (RecalboxConf::Instance().GetSystemFlatFolders(mSystem));
  FileData::List items;
  if (flatfolders) folder.GetItemsRecursivelyTo(items, includesFilter, mSystem.Excludes(), false);
  else folder.GetItemsTo(items, includesFilter, mSystem.Excludes(), true);

  // Check emptyness
  if (items.empty()) items.push_back(&mEmptyListItem); // Insert "EMPTY SYSTEM" item

  // Sort
  FileSorts::SortSets set = mSystem.IsVirtual() ? FileSorts::SortSets::MultiSystem :
                            mSystem.Descriptor().IsArcade() ? FileSorts::SortSets::Arcade :
                            FileSorts::SortSets::SingleSystem;
  FileSorts::Sorts sort = FileSorts::Clamp(RecalboxConf::Instance().GetSystemSort(mSystem), set);
  BuildAndSortArcadeGames(items, FileSorts::ComparerArcadeFromSort(sort), FileSorts::IsAscending(sort));
  BuildList();
}

void ArcadeGameListView::BuildList()
{
  mList.clear();
  mHeaderText.setText(mSystem.FullName());

  // Region filtering?
  Regions::GameRegions currentRegion = Regions::Clamp((Regions::GameRegions)RecalboxConf::Instance().GetSystemRegionFilter(mSystem));
  bool activeRegionFiltering = false;
  if (currentRegion != Regions::GameRegions::Unknown)
  {
    Regions::List availableRegion = AvailableRegionsInGames(mGameList);
    // Check if our region is in the available ones
    for(Regions::GameRegions region : availableRegion)
    {
      activeRegionFiltering = (region == currentRegion);
      if (activeRegionFiltering) break;
    }
  }

  // Prepare manufacturer filtering
  std::vector<ArcadeDatabase::Manufacturer> manufacturerList = GetManufacturerList();
  HashSet<int> hiddenManufacturers;
  for(const ArcadeDatabase::Manufacturer& manufacturer : manufacturerList)
    if (RecalboxConf::Instance().IsInArcadeSystemHiddenManufacturers(mSystem, manufacturer.Name.empty() ? ArcadeVirtualSystems::sAllOtherManufacturers : manufacturer.Name))
      hiddenManufacturers.insert(manufacturer.Index);
  bool mustHideManufacturers = !hiddenManufacturers.empty();

  // Add to list
  bool filterOutBios = RecalboxConf::Instance().GetArcadeViewHideBios();
  bool filterOutUnknown = RecalboxConf::Instance().GetArcadeViewHideNonWorking();
  bool onlyTate = RecalboxConf::Instance().GetTateOnly();
  for (const ParentTupple& parent : mGameList)
  {
    if (parent.mArcade == nullptr && !parent.mGame->IsFolder() && filterOutUnknown) continue;
    if (parent.mArcade != nullptr )
    {
      if (filterOutBios)
        if (parent.mArcade->Hierarchy() == ArcadeGame::Type::Bios) continue;
      if (mustHideManufacturers)
        if (HasMatchingManufacturer(hiddenManufacturers, parent.mArcade->LimitedManufacturer())) continue;
    }
    // Region filtering?
    int colorIndexOffset = 0;
    if (activeRegionFiltering)
      if (!Regions::IsIn4Regions(parent.mGame->Metadata().Region().Pack, currentRegion))
        colorIndexOffset = 2;
    // Tate filtering only on parent - clones have presumably the same rotation :)
    if (onlyTate && parent.mGame->Metadata().Rotation() == RotationType::None) continue;
    // Store
    mList.add(GetIconifiedDisplayName(parent), parent.mGame, colorIndexOffset + (parent.mGame->IsFolder() ? 1 : 0), false);

    // Children?
    if (/*parent.mArcade != nullptr && */parent.mCloneList != nullptr)
      /*if (parent.mArcade->Hierarchy() == ArcadeGame::Type::Parent)*/
        if (!parent.mFolded)
        {
          for (const ArcadeTupple& clone : *parent.mCloneList)
          {
            if (mustHideManufacturers)
              if (HasMatchingManufacturer(hiddenManufacturers, clone.mArcade->LimitedManufacturer())) continue;
            // Region filtering?
            colorIndexOffset = 0;
            if (activeRegionFiltering)
              if (!Regions::IsIn4Regions(clone.mGame->Metadata().Region().Pack, currentRegion))
                colorIndexOffset = 2;
            // Store
            mList.add(GetIconifiedDisplayName(clone), clone.mGame, colorIndexOffset, false);
          }
        }
  }

  // Check emptyness
  if (mList.IsEmpty())
    mList.add(_S(mEmptyListItem.Name()), &mEmptyListItem, 0, true);
}


bool ArcadeGameListView::HasMatchingManufacturer(const HashSet<int>& manufacturerSet, const ArcadeGame::LimitedManufacturerHolder& manufacturers)
{
  for(int i = manufacturers.Count(); --i >= 0; )
    if (manufacturerSet.contains(manufacturers.Manufacturer(i)))
      return true;
  return false;
}

String ArcadeGameListView::getArcadeItemIcon(const ArcadeTupple& game)
{
  String result;

  // Open folder for folders
  if (game.mGame->IsFolder())
  {
    result.Append("\uF07C");

    // Crossed out eye for hidden things
    if (game.mGame->Metadata().Hidden()) result.Append("\uF070");
  }
  if (game.mGame->IsGame())
  {
    // Hierarchy
    if (game.mArcade == nullptr) result.Append("\uF1C2");
    else
      switch(game.mArcade->Hierarchy()) // ◀ ▶ ▲ ▼
      {
        case ArcadeGame::Type::Parent:
        {
          const ParentTupple& parent = *((const ParentTupple*)&game);
          result.Append("\uF1F0").Append(parent.mCloneList == nullptr ? "•" : (parent.mFolded ? "▶" : "▼")); break;
        }
        case ArcadeGame::Type::Clone: result.Append("    \uF1F1•"); break;
        case ArcadeGame::Type::Orphaned: result.Append("\uF1F2•"); break;
        case ArcadeGame::Type::Bios: result.Append("\uF1F3"); break;
      }
    // Crossed out eye for hidden things
    if (game.mGame->Metadata().Hidden()) result.Append("\uF070");
    // System icon, for Favorite games
    if (mSystem.IsVirtual() || game.mGame->Metadata().Favorite()) result.Append(game.mGame->System().Descriptor().IconPrefix());
  }

  return result.Append(' ');
}

String ArcadeGameListView::GetDisplayName(const ArcadeTupple& game)
{
  if (RecalboxConf::Instance().GetArcadeUseDatabaseNames() && game.mArcade != nullptr)
    return game.mArcade->ArcadeName();
  return RecalboxConf::Instance().GetDisplayByFileName() ? game.mGame->Metadata().RomFileOnly().ToString() : game.mGame->Name(); // TODO: Use gugue new displayable name ASAP
}

String ArcadeGameListView::GetIconifiedDisplayName(const ArcadeTupple& game)
{
  return getArcadeItemIcon(game).Append(GetDisplayName(game));
}

void ArcadeGameListView::BuildAndSortArcadeGames(FileData::List& items, FileSorts::ComparerArcade comparer, bool ascending)
{
  // Split lists
  ParentTuppleList parents;
  HashMap<const FileData*, ArcadeTuppleList*> clones;
  ParentTuppleList orphaned;
  ParentTuppleList bios;
  ParentTuppleList notWorking;
  ParentTuppleList folders;
  // Virtual arcade only - Avoid too much lookups by keeping already found database localy
  // and by comparing parent pointers so that database changes only when parent changes
  HashMap<const FolderData*, const ArcadeDatabase*> mDatabaseLookup;
  FolderData* previousParent = nullptr;

  // Initialize database early for True arcade systems
  if (mSystem.IsTrueArcade())
  {
    mDatabase = mSystem.ArcadeDatabases().LookupDatabase(*mPopulatedFolder, mDefaultEmulator, mDefaultCore);
    if (mDatabase == nullptr || !mDatabase->IsValid())
      return DetailedGameListView::populateList(*mPopulatedFolder);
  }

  // Get initial fold status
  bool folded = RecalboxConf::Instance().GetArcadeViewFoldClones();

  mGameList.clear();
  for(FileData* item : items)
  {
    // Lookup database for virtual arcade systems
    if (mSystem.IsVirtualArcade())
      if (item->Parent() !=  previousParent)
      {
        previousParent = item->Parent();
        const ArcadeDatabase** db = mDatabaseLookup.try_get(previousParent);
        if (db != nullptr) mDatabase = *db;
        else
        {
          mDatabase = previousParent->System().ArcadeDatabases().LookupDatabase(*previousParent);
          mDatabaseLookup[previousParent] = mDatabase;
        }
      }
    // Safety check
    if (mDatabase == nullptr)
    {
      LOG(LogError) << "[ArcadeGameListView] No database for game " << item->RomPath().ToString() << " in system " << mSystem.FullName();
      continue;
    }
    // Lookup game from the current database
    const ArcadeGame* arcade = mDatabase->LookupGame(*item);
    // Distribute tuples in lists according to their type
    // Clones are stored in parent's lists
    if (arcade == nullptr)
    {
      if (item->IsFolder()) folders.push_back(ParentTupple(nullptr, item, false));
      else notWorking.push_back(ParentTupple(nullptr, item, false));
    }
    else
      switch(arcade->Hierarchy())
      {
        case ArcadeGame::Type::Parent: parents.push_back(ParentTupple(arcade, item, folded)); break;
        case ArcadeGame::Type::Clone:
        {
          if (clones[arcade->Parent()] == nullptr) clones[arcade->Parent()] = new ArcadeTuppleList();
          clones[arcade->Parent()]->push_back(ArcadeTupple(arcade, item)); break;
        }
        case ArcadeGame::Type::Orphaned: orphaned.push_back(ParentTupple(arcade, item, false)); break;
        case ArcadeGame::Type::Bios: bios.push_back(ParentTupple(arcade, item, false)); break;
      }
  }

  // Folders first
  AddSortedCategories({ &folders }, comparer, ascending);

  // Sorts parents / orphaned
  AddSortedCategories({ &parents, &orphaned }, comparer, ascending);

  // Insert clones as children
  for(ParentTupple& parent : mGameList)
  {
    if (parent.mGame->IsFolder()) continue; // Ignore folders
    if (parent.mArcade->Hierarchy() != ArcadeGame::Type::Parent) continue;
    ArcadeTuppleList** children = clones.try_get(parent.mGame);
    if (children != nullptr)
    {
      ArcadeTupplePointerList sortedList;
      for(ArcadeTupple& clone : **children) sortedList.push_back(&clone);
      FileSorts::SortArcade(sortedList, comparer, ascending); // Sort clones for a single parent
      ArcadeTuppleList* dynamicCloneList = new ArcadeTuppleList();
      for(ArcadeTupple* item : sortedList) dynamicCloneList->push_back(*item);
      parent.AddClones(dynamicCloneList);
      delete *children;
      clones.erase(parent.mGame);
    }
  }

  // Sort & add bios & unknowns
  AddSortedCategories({ &bios }, comparer, ascending);
  AddSortedCategories({ &notWorking }, comparer, ascending);

  // For virtual arcade systems, cleanup database
  if (mSystem.IsVirtualArcade()) mDatabase = nullptr;
}

void ArcadeGameListView::AddSortedCategories(const std::vector<ParentTuppleList*>& categoryLists, FileSorts::ComparerArcade comparer, bool ascending)
{
  ArcadeTupplePointerList sortedList;
  for(ParentTuppleList* categoryList : categoryLists)
    for(ParentTupple& item : *categoryList) sortedList.push_back(&item);
  FileSorts::SortArcade(sortedList, comparer, ascending); // Sort bios
  for(ArcadeTupple* item : sortedList) mGameList.push_back(*((ParentTupple*)item));
}

Regions::List ArcadeGameListView::AvailableRegionsInGames(ArcadeGameListView::ParentTuppleList& games)
{
  bool regionIndexes[256];
  memset(regionIndexes, 0, sizeof(regionIndexes));
  // Run through all games
  for(const ParentTupple& tupple : games)
  {
    unsigned int fourRegions = tupple.mGame->Metadata().Region().Pack;
    // Set the 4 indexes corresponding to all 4 regions (Unknown regions will all point to index 0)
    regionIndexes[(fourRegions >>  0) & 0xFF] = true;
    regionIndexes[(fourRegions >>  8) & 0xFF] = true;
    regionIndexes[(fourRegions >> 16) & 0xFF] = true;
    regionIndexes[(fourRegions >> 24) & 0xFF] = true;
  }
  // Rebuild final list
  Regions::List list;
  for(int i = 0; i < (int)sizeof(regionIndexes); ++i )
    if (regionIndexes[i])
      list.push_back((Regions::GameRegions)i);
  // Only unknown region?
  if (list.size() == 1 && regionIndexes[0])
    list.clear();
  return list;
}

bool ArcadeGameListView::ProcessInput(const InputCompactEvent& event)
{
  if (event.AnyHotkeyCombination())
  {
    bool vertical = event.HotkeyDownReleased() || event.HotkeyUpReleased();
    if (event.HotkeyLeftReleased()  && !vertical) { Fold(); return true; }
    if (event.HotkeyRightReleased() && !vertical) { Unfold(); return true; }
    if (event.HotkeyUpReleased())                 { FoldAll(); return true; }
    if (event.HotkeyDownReleased())               { UnfoldAll(); return true; }
  }

  return DetailedGameListView::ProcessInput(event);
}

void ArcadeGameListView::FoldAll()
{
  // Get cursor position - Get ancestor if its a clone
  FileData* item = getCursor();
  const ArcadeDatabase* database = item->System().ArcadeDatabases().LookupDatabase(*item->Parent());
  if (database == nullptr) return;

  const ArcadeGame* arcade = database->LookupGame(*item);
  if (arcade != nullptr)
    if (arcade->Hierarchy() == ArcadeGame::Type::Clone)
      item = (FileData*)arcade->Parent();

  // Fold all
  for(ParentTupple& parent : mGameList)
    if (parent.mCloneList != nullptr)
      if (parent.mArcade->Hierarchy() == ArcadeGame::Type::Parent)
        parent.mFolded = true;

  // Rebuild the UI list
  BuildList();

  // Set cursor
  setCursor(item);
}

void ArcadeGameListView::UnfoldAll()
{
  // Get cursor position
  FileData* item = getCursor();

  // Fold all
  for(ParentTupple& parent : mGameList)
    if (parent.mCloneList != nullptr)
      if (parent.mArcade->Hierarchy() == ArcadeGame::Type::Parent)
        parent.mFolded = false;

  // Rebuild the UI list
  BuildList();

  // Set cursor
  setCursor(item);
}

void ArcadeGameListView::Fold()
{
  // Get cursor position - Get ancestor if its a clone
  FileData* item = getCursor();
  const ArcadeDatabase* database = item->System().ArcadeDatabases().LookupDatabase(*item->Parent());
  if (database == nullptr) return;

  const ArcadeGame* arcade = database->LookupGame(*item);
  if (arcade != nullptr)
    if (arcade->Hierarchy() == ArcadeGame::Type::Clone)
      item = (FileData*)arcade->Parent();

  // Fold all
  bool rebuild =false;
  for(ParentTupple& parent : mGameList)
    if (parent.mCloneList != nullptr)
      if (parent.mArcade->Hierarchy() == ArcadeGame::Type::Parent)
        if (parent.mGame == item)
        {
          parent.mFolded = true;
          rebuild = true;
        }

  // Rebuild the UI list
  if (rebuild) BuildList();

  // Set cursor
  setCursor(item);
}

void ArcadeGameListView::Unfold()
{
  // Get cursor position
  FileData* item = getCursor();

  // Fold all
  bool rebuild =false;
  for(ParentTupple& parent : mGameList)
    if (parent.mGame == item)
      if (parent.mCloneList != nullptr)
        if (parent.mArcade->Hierarchy() == ArcadeGame::Type::Parent)
        {
          parent.mFolded = false;
          rebuild = true;
        }

  // Rebuild the UI list
  if (rebuild) BuildList();

  // Set cursor
  setCursor(item);
}

void ArcadeGameListView::jumpToLetter(unsigned int unicode)
{
  for(int c = 0; c < (int)getCursorIndexMax(); ++c)
    if (getDataAt(c)->IsGame())
      if (String::UpperUnicode(LookupDisplayName(*getDataAt(c)).ReadFirstUTF8()) == unicode)
      {
        setCursor(getDataAt(c));
        break;
      }
}

void ArcadeGameListView::jumpToNextLetter(bool forward)
{
  const ArcadeTupple& baseArcade = Lookup(*getCursor());
  const FileData* baseGame = baseArcade.mArcade != nullptr && baseArcade.mArcade->Hierarchy() == ArcadeGame::Type::Clone ? baseArcade.mArcade->Parent() : getCursor();
  UnicodeChar baseChar = String::UpperUnicode(LookupDisplayName(*baseGame).ReadFirstUTF8());
  int max = getCursorIndexMax() + 1;
  int step = max + (forward ? 1 : -1);

  int cursorIndex = getCursorIndex();
  for(int i = cursorIndex; (i = (i + step) % max) != cursorIndex; )
  {
    const ArcadeTupple& currentArcade = Lookup(*getDataAt(i));
    if (currentArcade.mArcade != nullptr && currentArcade.mArcade->Hierarchy() == ArcadeGame::Type::Clone) continue; // Skip clones
    if (String::UpperUnicode(LookupDisplayName(*getDataAt(i)).ReadFirstUTF8()) != baseChar)
    {
      setCursorIndex(i);
      break;
    }
  }
}

const ArcadeTupple& ArcadeGameListView::Lookup(const FileData& item)
{
  for(const ParentTupple& parent : mGameList)
    if (parent.mGame == &item) return parent;
    else if (parent.mCloneList != nullptr)
      for(const ArcadeTupple& clone : *parent.mCloneList)
        if (clone.mGame == &item)
          return clone;

  { LOG(LogError) << "[ArcadeGameListView] Lookup FileData failed for game " << item.Name(); }
  static ArcadeTupple nullTupple(nullptr, nullptr);
  return nullTupple;
}

String ArcadeGameListView::LookupDisplayName(const FileData& item)
{
  return GetDisplayName(Lookup(item));
}

std::vector<ArcadeDatabase::Manufacturer> ArcadeGameListView::GetManufacturerList() const
{
  if (mDatabase == nullptr) return std::vector<ArcadeDatabase::Manufacturer>();
  std::vector<ArcadeDatabase::Manufacturer> result = mDatabase->GetLimitedManufacturerList();
  return result;
}

int ArcadeGameListView::GetGameCountForManufacturer(int driverIndex) const
{
  int count = 0;
  for(const ParentTupple& parent : mGameList)
  {
    if (parent.mArcade == nullptr) continue;
    if (parent.mArcade->LimitedManufacturer().Contains(driverIndex)) count++;
    if (parent.mCloneList != nullptr)
      for (const ArcadeTupple& clone: *parent.mCloneList)
        if (clone.mArcade->LimitedManufacturer().Contains(driverIndex)) count++;
  }
  return count;
}

String ArcadeGameListView::GetDisplayName(FileData& game)
{
  for(const ParentTupple& parent : mGameList)
    if (parent.mGame == &game) return GetIconifiedDisplayName(parent);
    else if (parent.mCloneList != nullptr)
      for(const ArcadeTupple& clone : *parent.mCloneList)
        if (clone.mGame == &game) return GetIconifiedDisplayName(clone);

  // Fallback
  return GetIconifiedDisplayName(ArcadeTupple(nullptr, &game));
}

String ArcadeGameListView::GetDescription(FileData& game)
{
  String emulator;
  String core;
  if (const ArcadeDatabase* database = game.System().ArcadeDatabases().LookupDatabase(game, emulator, core); database != nullptr)
    if (const ArcadeGame* arcade = database->LookupGame(game); arcade != nullptr)
    {
      if (arcade->EmulationStatus() == ArcadeGame::Status::Imperfect)
        return (_F(_("{0} reports the emulation status of this game is 'imperfect'.")) / core).ToString().Append(String::LF, 2).Append(game.Metadata().Description());
      if (arcade->EmulationStatus() == ArcadeGame::Status::Preliminary)
        return (_F(_("{0} reports the emulation status of this game is 'preliminary'. You should expect issues such as bugs or even crashes!")) / core).ToString().Append(String::LF, 2).Append(game.Metadata().Description());
    }
  return game.Metadata().Description();
}

void ArcadeGameListView::RefreshItem(FileData* game)
{
  if (game == nullptr || !game->IsGame()) { LOG(LogError) << "[DetailedGameListView] Trying to refresh null or empty item"; return; }

  int index = mList.Lookup(game);
  if (index < 0) { LOG(LogError) << "[DetailedGameListView] Trying to refresh a not found item"; return; }
  mList.changeTextAt(index, GetDisplayName(*game));
}
