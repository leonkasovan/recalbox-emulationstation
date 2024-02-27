//
// Created by bkg2k on 24/11/18.
//

#include "FolderData.h"
#include "utils/Log.h"
#include "systems/SystemData.h"
#include "GameNameMapManager.h"
#include "GameFilesUtils.h"
#include <utils/Files.h>

#define CastFolder(f) ((FolderData*)(f))

FolderData::~FolderData()
{
  for (FileData* fd : mChildren)
  {
    delete fd;
  }
  mChildren.clear();
}

void FolderData::AddChild(FileData* file, bool lukeImYourFather)
{
  assert(file->Parent() == nullptr || !lukeImYourFather);

  mChildren.push_back(file);
  if (lukeImYourFather)
    file->SetParent(this);
}

void FolderData::RemoveChild(const FileData* file)
{
  for (auto it = mChildren.begin(); it != mChildren.end(); it++)
    if(*it == file)
    {
      mChildren.erase(it);
      return;
    }
}

bool FolderData::RemoveChildRecursively(const FileData* file)
{
  bool result = false;
  // Recurse
  bool found = false;
  for(FileData* item : mChildren)
    if (item->IsGame() && file == item) found = true;
    else if (item->IsFolder())
      if (CastFolder(item)->RemoveChildRecursively(file)) result = true;
  // Erase
  if (found)
  {
    erase(mChildren, file);
    result = true;
  }
  return result;
}

static bool IsMatching(const String& fileWoExt, const String& extension, const String& extensionList)
{
  #define sFilesPrefix "files:"
  if (!extensionList.StartsWith(LEGACY_STRING(sFilesPrefix)))
  {
    // Seek in regular extensions
    int start = 0;
    while(start < (int)extensionList.size())
    {
      int extensionPos = extensionList.Find(extension, start);
      if (extensionPos < 0) return false;
      const char* p = extensionList.data();
      char endChar = p[extensionPos + extension.size()];
      if (endChar == ' ' || endChar == 0) return true;
      start += (int)(extensionPos + extension.size());
    }
    return false;
  }

  // Seek complete files
  constexpr int sFilesPrefixLength = sizeof(sFilesPrefix) - 1;
  String file(fileWoExt); file.Append(extension).LowerCase();
  int filePos = extensionList.Find(file);
  if (filePos < 0) return false;
  const char* p = extensionList.data();
  return ((filePos == sFilesPrefixLength) || (p[filePos - 1] == ' ')) &&
         ((filePos + file.size() == extensionList.size()) || (p[filePos + file.size()] == ' '));
}

void FolderData::PopulateRecursiveFolder(RootFolderData& root, const String& originalFilteredExtensions, const String& ignoreList, FileData::StringMap& doppelgangerWatcher)
{
  const Path folderPath(RomPath());
  if (!folderPath.IsDirectory())
  {
    { LOG(LogWarning) << "[FolderData] Error - folder with path \"" << folderPath.ToString() << "\" is not a directory!"; }
    return;
  }

  // media folder?
  if (folderPath.FilenameWithoutExtension() == "media")
    return;

  //make sure that this isn't a symlink to a thing we already have
  if (folderPath.IsSymLink())
  {
    // if this symlink resolves to somewhere that's at the beginning of our path, it's gonna recurse
    Path canonical = folderPath.ToCanonical();
    if (folderPath.ToString().compare(0, canonical.ToString().size(), canonical.ToChars()) == 0)
    { LOG(LogWarning) << "[FolderData] Skipping infinitely recursive symlink \"" << folderPath.ToString() << "\""; return; }
  }

  // Subsystem override
  String filteredExtensions = originalFilteredExtensions;
  if ((folderPath / ".system.cfg").Exists())
  {
    IniFile subSystem(folderPath / ".system.cfg", false, false);
    filteredExtensions = subSystem.AsString("extensions", originalFilteredExtensions);
  }

  // special system?
  bool hasFiltering = GameNameMapManager::HasFiltering(System());
  // No extension?
  bool noExtensions = filteredExtensions.empty();

  // Keep temporary object outside the loop to avoid construction/destruction and keep memory allocated AMAP
  Path::PathList items = folderPath.GetDirectoryContent();

  HashSet<String> blacklist;
  bool containsMultiDiskFile = GameFilesUtils::ContainsMultiDiskFile(filteredExtensions);
  if (containsMultiDiskFile)
    for(const auto& itemPath : items)
      GameFilesUtils::ExtractUselessFiles(itemPath, blacklist);

  for (Path& filePath : items)
  {
    // Get file
    String stem = filePath.FilenameWithoutExtension();
    if (stem == "gamelist") continue; // Ignore gamelist.zip/xml
    if (stem.empty()) continue;

    // Force to hide ignored files
    // Force to hide ignored files
    const String fileName = filePath.Filename();
    int p = (int)ignoreList.find(fileName);
    if (p > 0 && ignoreList[p-1] == ',')
      if (ignoreList[p + fileName.length()] == ',')
        continue;

    if (containsMultiDiskFile && blacklist.contains(filePath.ToString())) continue;

    // and Extension
    String extension = filePath.Extension().LowerCase();

    //fyi, folders *can* also match the extension and be added as games - this is mostly just to support higan
    //see issue #75: https://github.com/Aloshi/EmulationStation/issues/75
    bool isLaunchableGame = false;
    if (!filePath.IsHidden())
    {
      if ((noExtensions && filePath.IsFile()) ||
          (!extension.empty() && IsMatching(stem, extension, filteredExtensions)))
      {
        if (hasFiltering)
        {
          if (GameNameMapManager::IsFiltered(System(), stem))
            continue; // MAME Bios or Machine
        }
        // Get the key for duplicate detection. MUST MATCH KEYS USED IN Gamelist.findOrCreateFile - Always fullpath
        if (doppelgangerWatcher.find(filePath.ToString()) == doppelgangerWatcher.end())
        {
          FileData* newGame = new FileData(filePath, root);
          newGame->Metadata().SetDirty();
          AddChild(newGame, true);
          doppelgangerWatcher[filePath.ToString()] = newGame;
        }
        isLaunchableGame = true;
      }

      //add directories that also do not match an extension as folders
      if (!isLaunchableGame && filePath.IsDirectory())
      {
        FolderData* newFolder = new FolderData(filePath, root);
        newFolder->PopulateRecursiveFolder(root, filteredExtensions, ignoreList, doppelgangerWatcher);

        //ignore folders that do not contain games
        if (newFolder->HasChildren())
        {
          const String key = newFolder->RomPath().ToString();
          if (doppelgangerWatcher.find(key) == doppelgangerWatcher.end())
          {
            AddChild(newFolder, true);
            doppelgangerWatcher[key] = newFolder;
          }
        }
        else
          delete newFolder;
      }
    }
  }
}

void FolderData::ExtractUselessFiles(const Path::PathList& items, FileSet& blacklist)
{
  for (const Path& filePath : items)
  {
    const String extension = filePath.Extension();
    if(extension == ".cue")
    {
      ExtractUselessFilesFromCue(filePath, blacklist);
      continue;
    }
    if(extension == ".ccd")
    {
      ExtractUselessFilesFromCcd(filePath, blacklist);
      continue;
    }
    if(extension == ".gdi" && filePath.Size() <= sMaxGdiFileSize)
    {
      ExtractUselessFilesFromGdi(filePath, blacklist);
      continue;
    }
    if(extension == ".m3u")
    {
      ExtractUselessFilesFromM3u(filePath, blacklist);
      continue;
    }
  }
}

void FolderData::ExtractUselessFilesFromCue(const Path& path, FileSet& list)
{
  String file = Files::LoadFile(path);
  for(const String& line : file.Split('\n'))
    if (line.Contains("FILE") && line.Contains("BINARY"))
      ExtractFileNameFromLine(line, list);
}

void FolderData::ExtractUselessFilesFromCcd(const Path& path, FileSet& list)
{
  String file = path.FilenameWithoutExtension();
  list.insert(file + ".cue");
  list.insert(file + ".bin");
  list.insert(file + ".sub");
  list.insert(file + ".img");
}

void FolderData::ExtractUselessFilesFromM3u(const Path& path, FileSet& list)
{
  String file = Files::LoadFile(path);
  for(String line : file.Split('\n'))
    list.insert(line.Trim('\r'));
}

void FolderData::ExtractUselessFilesFromGdi(const Path& path, FileSet& list)
{
  String file = Files::LoadFile(path);
  for(const String& line : file.Split('\n'))
    ExtractFileNameFromLine(line, list);
}

void FolderData::ExtractFileNameFromLine(const String& line, FileSet& list)
{
  // 1 check file name between double quotes
  String string;
  if (line.Extract('"', '"', string, true))
    if (string.Contains('.'))
    {
      list.insert(string);
      return;
    }
  
  // 2 check every words separated by space that contains dot
  for(const String& word : line.Split(' ', true))
    if (word.Contains("."))
      list.insert(word);
}

int FolderData::getAllFoldersRecursively(FileData::List& to) const
{
  int gameCount = 0;
  for (FileData* fd : mChildren)
  {
    if (fd->IsFolder())
    {
      to.push_back(fd);
      int position = (int)to.size(); // TOOD: Check if the insert is necessary
      if (CastFolder(fd)->getAllFoldersRecursively(to) > 1)
        to.insert(to.begin() + position, fd); // Include folders iif it contains more than one game.
    }
    else if (fd->IsGame()) gameCount++;
  }
  return gameCount;
}

FileData::List FolderData::GetAllFolders() const
{
  FileData::List result;
  getAllFoldersRecursively(result);
  return result;
}

void FolderData::ClearSubChildList()
{
  for (int i = (int)mChildren.size(); --i >= 0; )
  {
    const FileData* fd = mChildren[i];
    if (fd->IsFolder())
      CastFolder(fd)->ClearSubChildList();
    else
      mChildren[i] = nullptr;
  }
}

void FolderData::BuildDoppelgangerMap(FileData::StringMap& doppelganger, bool includefolder) const
{
  for (FileData* fd : mChildren)
  {
    if (fd->IsFolder())
    {
      CastFolder(fd)->BuildDoppelgangerMap(doppelganger, includefolder);
      if (includefolder)
        doppelganger[fd->RomPath().ToString()] = fd;
    }
    else
      doppelganger[fd->RomPath().ToString()] = fd;
  }
}

bool FolderData::HasMissingHashRecursively()
{
  for (FileData* fd : mChildren)
    if (fd->IsFolder())
    {
      if (CastFolder(fd)->HasMissingHashRecursively())
        return true;
    }
    else if (fd->IsGame())
      if (fd->Metadata().RomCrc32() == 0)
        return true;
  return false;
}

void FolderData::CalculateMissingHashRecursively()
{
  for (FileData* fd : mChildren)
    if (fd->IsFolder())
      CastFolder(fd)->CalculateMissingHashRecursively();
    else if (fd->IsGame())
      if (fd->Metadata().RomCrc32() == 0)
        fd->CalculateHash();
}

int FolderData::getMissingHashRecursively(FileData::List& to) const
{
  int gameCount = 0;
  for (FileData* fd : mChildren)
  {
    if (fd->IsFolder())
      gameCount += CastFolder(fd)->getMissingHashRecursively(to);
    else if (fd->IsGame())
      if (fd->Metadata().RomCrc32() == 0)
      {
        to.push_back(fd);
        gameCount++;
      }
  }
  return gameCount;
}

int FolderData::getItemsRecursively(FileData::List& to, IFilter* filter, bool includefolders) const
{
  int gameCount = 0;
  for (FileData* fd : mChildren)
  {
    if (fd->IsFolder())
    {
      if (CastFolder(fd)->getItemsRecursively(to, filter, includefolders) > 1)
        if (includefolders)
          to.push_back(fd); // Include folders iif it contains more than one game.
    }
    else if (fd->IsGame())
    {
      if (filter->ApplyFilter(*fd))
      {
        to.push_back(fd);
        gameCount++;
      }
    }
  }
  return gameCount;
}

int FolderData::getItemsRecursively(FileData::List& to, Filter includes, Filter excludes, bool includefolders) const
{
  int gameCount = 0;
  for (FileData* fd : mChildren)
  {
    if (fd->IsFolder())
    {
      if (CastFolder(fd)->getItemsRecursively(to, includes, excludes, includefolders) > 1)
        if (includefolders)
          to.push_back(fd); // Include folders iif it contains more than one game.
    }
    else if (fd->IsGame())
    {
      if(IsFiltered(fd, includes, excludes))
      {
        to.push_back(fd);
        gameCount++;
      }
    }
  }
  return gameCount;
}

int FolderData::countItemsRecursively(IFilter* filter, bool includefolders) const
{
  int result = 0;
  for (FileData* fd : mChildren)
  {
    if (fd->IsFolder())
    {
      int subCount = CastFolder(fd)->countItemsRecursively(filter, includefolders);
      result += subCount;
      if (subCount > 1)
        if (includefolders)
          result++; // Include folders iif it contains more than one game.
    }
    else if (fd->IsGame())
    {
      if (filter->ApplyFilter(*fd))
        result++;
    }
  }
  return result;
}

int FolderData::countAllGamesAndFavoritesAndHidden(Filter excludes, int& favorites, int& hidden) const
{
  int result = 0;
  for (FileData* fd : mChildren)
  {
    if (fd->IsFolder())
    {
      int subCount = CastFolder(fd)->countAllGamesAndFavoritesAndHidden(excludes, favorites, hidden);
      result += subCount;
    }
    else if (fd->IsGame())
    {
      if(IsFiltered(fd, Filter::Normal | Filter::Favorite, excludes))
      {
        if (fd->Metadata().Favorite()) favorites++;
        result++;
      }
      else hidden++;
    }
  }
  return result;
}

int FolderData::countItemsRecursively(Filter includes, Filter excludes, bool includefolders) const
{
  int result = 0;
  for (FileData* fd : mChildren)
  {
    if (fd->IsFolder())
    {
      int subCount = CastFolder(fd)->countItemsRecursively(includes, excludes, includefolders);
      result += subCount;
      if (subCount > 1)
        if (includefolders)
          result++; // Include folders iif it contains more than one game.
    }
    else if (fd->IsGame())
    {
      if(IsFiltered(fd, includes, excludes))
        result++;
    }
  }
  return result;
}

int FolderData::countAllRecursively() const
{
  int result = 0;
  for (FileData* fd : mChildren)
  {
    if (fd->IsFolder()) result += CastFolder(fd)->countAllRecursively();
    else if (fd->IsGame()) result++;
  }
  return result;
}

bool FolderData::HasGame() const
{
  for(FileData* fd : mChildren)
  {
    if (fd->IsGame() || (fd->IsFolder() && CastFolder(fd)->HasGame()))
      return true;
  }
  return false;
}

bool FolderData::HasVisibleGame(FileData::TopLevelFilter filter) const
{
  for (FileData* fd : mChildren)
  {
    if ( (fd->IsGame() && fd->IsDisplayable(filter)) || (fd->IsFolder() && CastFolder(fd)->HasVisibleGame(filter)))
      return true;
  }
  return false;
}

bool FolderData::HasSacrapableGame() const
{
  for (FileData* fd : mChildren)
  {
    if ( (fd->IsGame() && !fd->TopAncestor().PreInstalled()) ||
         (fd->IsFolder() && CastFolder(fd)->HasSacrapableGame()))
      return true;
  }
  return false;
}

bool FolderData::HasVisibleGameWithVideo(TopLevelFilter filter) const
{
    for (FileData* fd : mChildren)
    {
        if ( ((fd->IsGame() && !fd->IsDisplayable(filter)) && !fd->Metadata().VideoAsString().empty() &&
        fd->Metadata().Video().Exists()) || (fd->IsFolder() && CastFolder(fd)->HasVisibleGameWithVideo(filter)))
            return true;
    }
    return false;
}

bool FolderData::HasFilteredItemsRecursively(IFilter* filter) const
{
  for (FileData* fd : mChildren)
  {
    if (fd->IsFolder())
    {
      if (CastFolder(fd)->HasFilteredItemsRecursively(filter))
        return true;
    }
    else if (fd->IsGame())
      if (filter->ApplyFilter(*fd))
        return true;
  }
  return false;
}

int FolderData::getItems(FileData::List& to, Filter includes, Filter excludes, bool includefolders) const
{
  int gameCount = 0;
  for (FileData* fd : mChildren)
  {
    if (fd->IsFolder())
    {
      FolderData* folder = CastFolder(fd);
      // Seek for isolated file
      FileData* isolatedFile = nullptr;
      while((folder->mChildren.size() == 1) && folder->mChildren[0]->IsFolder()) folder = CastFolder(folder->mChildren[0]);
      if (folder->mChildren.size() == 1)
      {
        FileData* item = folder->mChildren[0];
        if (item->IsGame())
        {
          if(IsFiltered(fd, includes, excludes))
            isolatedFile = item;
        }
      }
      if (isolatedFile != nullptr) to.push_back(isolatedFile);
      else
      if (includefolders)
        if (folder->countItems(includes, excludes, includefolders ) > 0) // Only add if it contains at leas one game
          to.push_back(fd);
    }
    else
    {
      if(IsFiltered(fd, includes, excludes))
      {
        to.push_back(fd);
        gameCount++;
      }
    }
  }
  return gameCount;
}

int FolderData::countItems(Filter includes, Filter excludes, bool includefolders) const
{
  int result = 0;
  for (FileData* fd : mChildren)
  {
    if (fd->IsFolder())
    {
      FolderData* folder = CastFolder(fd);
      // Seek for isolated file
      FileData* isolatedFile = nullptr;
      while((folder->mChildren.size() == 1) && folder->mChildren[0]->IsFolder()) folder = CastFolder(folder->mChildren[0]);
      if (folder->mChildren.size() == 1)
      {
        FileData* item = folder->mChildren[0];
        if (item->IsGame())
        {
          if(IsFiltered(fd, includes, excludes))
            isolatedFile = item;
        }
      }
      if (isolatedFile != nullptr) result++;
      else
        if (includefolders)
          if (folder->countItems(includes, excludes, includefolders) > 0) // Only add if it contains at leas one game
            result++;
    }
    else if (fd->IsGame())
    {
      if(IsFiltered(fd, includes, excludes))
        result++;
    }
  }
  return result;
}

void FolderData::ParseAllItems(IParser& parser)
{
  for (FileData* fd : mChildren)
  {
    parser.Parse(*fd);
    if (fd->IsFolder())
    {
      FolderData* folder = CastFolder(fd);
      folder->ParseAllItems(parser);
    }
  }
}

bool FolderData::IsFiltered(FileData* fd, FileData::Filter includes, FileData::Filter excludes)
{
  Filter currentIncludes = Filter::None;
  if (fd->Metadata().Favorite())
    currentIncludes |= Filter::Favorite;
  if (currentIncludes == 0)
    currentIncludes = Filter::Normal;

  Filter currentExcludes = Filter::None;
  if (fd->Metadata().Hidden())
    currentExcludes |= Filter::Hidden;
  if (fd->Metadata().NoGame())
    currentExcludes |= Filter::NoGame;
  if (!fd->Metadata().LatestVersion())
    currentExcludes |= Filter::NotLatest;
  if (fd->TopAncestor().PreInstalled())
    currentExcludes |= Filter::PreInstalled;
  if (!fd->System().IncludeAdultGames() && fd->Metadata().Adult())
    currentExcludes |= Filter::Adult;

  return ((currentIncludes & includes) != 0 && (currentExcludes & excludes) == 0);
}

bool FolderData::HasDetailedData() const
{
  for (FileData* fd : mChildren)
  {
    if (fd->IsFolder())
    {
      FolderData* folder = CastFolder(fd);
      if (folder->HasDetailedData())
        return true;
    }
    else
    {
      const MetadataDescriptor& metadata = fd->Metadata();
      if (!metadata.Image().IsEmpty()) return true;
      if (!metadata.Thumbnail().IsEmpty()) return true;
      if (!metadata.Description().empty()) return true;
      if (!metadata.Publisher().empty()) return true;
      if (!metadata.Developer().empty()) return true;
      if (!metadata.Genre().empty()) return true;
    }
  }
  return false;
}

bool FolderData::LookupGame(const FileData* game) const
{
  for (FileData* fd : mChildren)
  {
    bool found = fd == game;
    if (fd->IsFolder()) found |= (CastFolder(fd)->LookupGame(game));
    if (found) return true;
  }
  return false;
}

FileData* FolderData::LookupGame(const String& item, SearchAttributes attributes, const String& path) const
{
  // Recursively look for the game in subfolders too
  for (FileData* fd : mChildren)
  {
    Path filePath(fd->RomPath());
    String filename = path.empty() ? filePath.ToString() : path + '/' + filePath.Filename();

    if (fd->IsFolder())
    {
      FolderData* folder = CastFolder(fd);
      FileData* result = folder->LookupGame(item, attributes, path);
      if (result != nullptr)
        return result;
    }
    else
    {
      if ((attributes & SearchAttributes::ByHash) != 0 && fd->Metadata().RomCrc32AsString() == item)
          return fd;
      if ((attributes & SearchAttributes::ByNameWithExt) != 0 && strcasecmp(filename.c_str(), item.c_str()) == 0)
          return fd;
      if ((attributes & SearchAttributes::ByName) != 0)
        if (strcasecmp((path.empty() ? filePath.FilenameWithoutExtension() : path + '/' + filePath.FilenameWithoutExtension()).c_str(), item.c_str()) == 0)
          return fd;
    }
  }
  return nullptr;
}

FileData* FolderData::LookupGameByFilePath(const String& filepath) const
{
  // Recursively look for the game in subfolders too
  for (FileData* fd : mChildren)
    if (fd->IsFolder())
    {
      FolderData* folder = CastFolder(fd);
      FileData* result = folder->LookupGameByFilePath(filepath);
      if (result != nullptr)
        return result;
    }
    else
      if (filepath == fd->RomPath().ToString())
        return fd;

  return nullptr;
}

FileData* FolderData::LookupGameByCRC32(int crc32) const
{
  if (crc32 == 0) return nullptr;
  // Recursively look for the game in subfolders too
  for (FileData* fd : mChildren)
    if (fd->IsFolder())
    {
      FolderData* folder = CastFolder(fd);
      FileData* result = folder->LookupGameByCRC32(crc32);
      if (result != nullptr)
        return result;
    }
    else
      if (crc32 == fd->Metadata().RomCrc32())
        return fd;

  return nullptr;
}

FileData* FolderData::GetNextFavoriteTo(FileData* reference)
{
  // Look for position index. If not found, start from the begining
  int position = 0;
  for (int i = (int)mChildren.size(); --i >= 0; )
    if (mChildren[i] == reference)
    {
      position = i;
      break;
    }

  // Look forward
  for (int i = position; i < (int)mChildren.size(); i++)
    if (mChildren[i]->Metadata().Favorite())
      return mChildren[i];
  // Look backward
  for (int i = position; --i >= 0; )
    if (mChildren[i]->Metadata().Favorite())
      return mChildren[i];

  return nullptr;
}

void FolderData::Sort(FileData::List& items, FileData::Comparer comparer, bool ascending)
{
  if (items.size() > 1)
  {
    if (ascending)
      QuickSortAscending(items, 0, (int)items.size() - 1, comparer);
    else
      QuickSortDescending(items, 0, (int)items.size() - 1, comparer);
  }
}

void FolderData::QuickSortAscending(FileData::List& items, int low, int high, FileData::Comparer comparer)
{
  int Low = low, High = high;
  const FileData& pivot = *items[(Low + High) >> 1];
  do
  {
    while((*comparer)(*items[Low] , pivot) < 0) Low++;
    while((*comparer)(*items[High], pivot) > 0) High--;
    if (Low <= High)
    {
      FileData* Tmp = items[Low]; items[Low] = items[High]; items[High] = Tmp;
      Low++; High--;
    }
  }while(Low <= High);
  if (High > low) QuickSortAscending(items, low, High, comparer);
  if (Low < high) QuickSortAscending(items, Low, high, comparer);
}

void FolderData::QuickSortDescending(FileData::List& items, int low, int high, FileData::Comparer comparer)
{
  int Low = low, High = high;
  const FileData& pivot = *items[(Low + High) >> 1];
  do
  {
    while((*comparer)(*items[Low] , pivot) > 0) Low++;
    while((*comparer)(*items[High], pivot) < 0) High--;
    if (Low <= High)
    {
      FileData* Tmp = items[Low]; items[Low] = items[High]; items[High] = Tmp;
      Low++; High--;
    }
  }while(Low <= High);
  if (High > low) QuickSortDescending(items, low, High, comparer);
  if (Low < high) QuickSortDescending(items, Low, high, comparer);
}

bool FolderData::Contains(const FileData* item, bool recurse) const
{
  for (FileData* fd : mChildren)
  {
    if ((fd->IsFolder()) && recurse)
    {
      if (Contains(fd, true)) return true;
    }
    if (fd == item) return true;
  }
  return false;
}

FileData::List FolderData::GetFilteredItemsRecursively(IFilter* filter, bool includefolders) const
{
  FileData::List result;
  result.reserve((unsigned long)countItemsRecursively(Filter::All, System().Excludes(), includefolders)); // Allocate once
  getItemsRecursively(result, filter, includefolders);
  result.shrink_to_fit();

  return result;
}

FileData::List FolderData::GetFilteredItemsRecursively(Filter filters, Filter excludes, bool includefolders) const
{
  FileData::List result;
  result.reserve((unsigned long)countItemsRecursively(filters, excludes, includefolders)); // Allocate once
  getItemsRecursively(result, filters, excludes, includefolders);

  return result;
}

FileData::List FolderData::GetAllItemsRecursively(bool includefolders,Filter excludes) const
{
  FileData::List result;
  result.reserve((unsigned long)countItemsRecursively(Filter::All, excludes, includefolders)); // Allocate once
  getItemsRecursively(result, Filter::All, excludes, includefolders);

  return result;
}

FileData::List FolderData::GetAllDisplayableItemsRecursively(bool includefolders, Filter excludes) const
{
  FileData::List result;
  result.reserve((unsigned long)countItemsRecursively(Filter::Normal | Filter::Favorite, excludes, includefolders)); // Allocate once
  getItemsRecursively(result, Filter::Normal | Filter::Favorite, excludes, includefolders);

  return result;
}

FileData::List FolderData::GetAllFavoritesRecursively(bool includefolders, Filter excludes) const
{
  FileData::List result;
  result.reserve((unsigned long)countItemsRecursively(Filter::Favorite, excludes, includefolders)); // Allocate once
  getItemsRecursively(result, Filter::Favorite, excludes, includefolders);

  return result;
}

FileData::List FolderData::GetFilteredItems(Filter filters, Filter excludes, bool includefolders) const
{
  FileData::List result;
  result.reserve((unsigned long)countItems(filters, excludes, includefolders)); // Allocate once
  getItems(result, filters, excludes, includefolders);

  return result;
}

FileData::List FolderData::GetAllItems(bool includefolders, Filter excludes) const
{
  FileData::List result;
  result.reserve((unsigned long)countItems(Filter::All, excludes, includefolders)); // Allocate once
  getItems(result, Filter::All, excludes ,includefolders);

  return result;
}

FileData::List FolderData::GetAllDisplayableItems(bool includefolders, Filter excludes) const
{
  FileData::List result;
  result.reserve((unsigned long)countItems(Filter::Normal | Filter::Favorite, excludes, includefolders)); // Allocate once
  getItems(result, Filter::Normal | Filter::Favorite, excludes, includefolders);

  return result;
}

FileData::List FolderData::GetAllFavorites(bool includefolders, Filter excludes) const
{
  FileData::List result;
  result.reserve((unsigned long)countItems(Filter::Favorite, excludes, includefolders)); // Allocate once
  getItems(result, Filter::Favorite, excludes, includefolders);

  return result;
}

bool FolderData::IsDirty() const
{
  if (TopAncestor().HasDeletedChildren())
    return true;
  for (FileData* fd : mChildren)
  {
    if (fd->IsFolder() && CastFolder(fd)->IsDirty())
      return true;
    if (fd->Metadata().IsDirty())
      return true;
  }
  return false;
}

int FolderData::GetFoldersRecursivelyTo(FileData::List& to) const
{
  if (IsTopMostRoot())
  {
    int count = 0;
    for (const RootFolderData* root : ((RootFolderData*) this)->SubRoots())
      count += root->getAllFoldersRecursively(to);
    return count;
  }

  return getAllFoldersRecursively(to);
}

int FolderData::GetItemsRecursivelyTo(FileData::List& to, FileData::Filter includes, FileData::Filter excludes, bool includefolders) const
{
  if (IsTopMostRoot())
  {
    int count = 0;
    for (const RootFolderData* root : ((RootFolderData*) this)->SubRoots())
      count += root->getItemsRecursively(to, includes, excludes, includefolders);
    return count;
  }

  return getItemsRecursively(to, includes, excludes, includefolders);
}

int FolderData::GetItemsTo(FileData::List& to, FileData::Filter includes, FileData::Filter excludes, bool includefolders) const
{
  if (IsTopMostRoot())
  {
    int count = 0;
    for (const RootFolderData* root : ((RootFolderData*) this)->SubRoots())
      count += root->getItems(to, includes, excludes, includefolders);
    return count;
  }

  return getItems(to, includes, excludes, includefolders);
}

void FolderData::LookupGamesFromPath(const MetadataStringHolder::IndexAndDistance& index, FileData::List& games) const
{
  for(FileData* game : mChildren)
    if (game->IsFolder()) CastFolder(game)->LookupGamesFromPath(index, games);
    else if (game->Metadata().IsMatchingFileIndex(index.Index))
        games.push_back(game);
}

void FolderData::LookupGamesFromName(const MetadataStringHolder::IndexAndDistance& index, FileData::List& games) const
{
  for(FileData* game : mChildren)
    if (game->IsFolder()) CastFolder(game)->LookupGamesFromName(index, games);
    else if (game->Metadata().IsMatchingNameIndex(index.Index))
          games.push_back(game);
}

void FolderData::LookupGamesFromDescription(const MetadataStringHolder::IndexAndDistance& index, FileData::List& games) const
{
  for(FileData* game : mChildren)
    if (game->IsFolder()) CastFolder(game)->LookupGamesFromDescription(index, games);
    else if (game->Metadata().IsMatchingDescriptionIndex(index.Index))
          games.push_back(game);
}

void FolderData::LookupGamesFromDeveloper(const MetadataStringHolder::IndexAndDistance& index, FileData::List& games) const
{
  for(FileData* game : mChildren)
    if (game->IsFolder()) CastFolder(game)->LookupGamesFromDeveloper(index, games);
    else if (game->Metadata().IsMatchingDeveloperIndex(index.Index))
          games.push_back(game);
}

void
FolderData::LookupGamesFromPublisher(const MetadataStringHolder::IndexAndDistance& index, FileData::List& games) const
{
  for(FileData* game : mChildren)
    if (game->IsFolder()) CastFolder(game)->LookupGamesFromPublisher(index, games);
    else if (game->Metadata().IsMatchingPublisherIndex(index.Index))
          games.push_back(game);
}

void FolderData::LookupGamesFromAll(const MetadataStringHolder::IndexAndDistance& index, FileData::List& games) const
{
  for(FileData* game : mChildren)
    if (game->IsFolder()) CastFolder(game)->LookupGamesFromAll(index, games);
    else
      switch ((FastSearchContext) index.Context)
      {
        case FastSearchContext::Path:
          if (game->Metadata().IsMatchingFileIndex(index.Index)) games.push_back(game);
          break;
        case FastSearchContext::Name:
          if (game->Metadata().IsMatchingNameIndex(index.Index)) games.push_back(game);
          break;
        case FastSearchContext::Description:
          if (game->Metadata().IsMatchingDescriptionIndex(index.Index)) games.push_back(game);
          break;
        case FastSearchContext::Developer:
          if (game->Metadata().IsMatchingDeveloperIndex(index.Index)) games.push_back(game);
          break;
        case FastSearchContext::Publisher:
          if (game->Metadata().IsMatchingPublisherIndex(index.Index)) games.push_back(game);
          break;
        case FastSearchContext::All:
        default: break;
      }
}

void FolderData::BuildFastSearchSeriesPath(FolderData::FastSearchItemSerie& into) const
{
  for(const FileData* game : mChildren)
    if (game->IsFolder()) CastFolder(game)->BuildFastSearchSeriesPath(into);
    else into.Set(game, game->Metadata().FileIndex());
}

void FolderData::BuildFastSearchSeriesName(FolderData::FastSearchItemSerie& into) const
{
  for(const FileData* game : mChildren)
    if (game->IsFolder()) CastFolder(game)->BuildFastSearchSeriesName(into);
    else into.Set(game, game->Metadata().NameIndex());
}

void FolderData::BuildFastSearchSeriesDescription(FolderData::FastSearchItemSerie& into) const
{
  for(const FileData* game : mChildren)
    if (game->IsFolder()) CastFolder(game)->BuildFastSearchSeriesDescription(into);
    else into.Set(game, game->Metadata().DescriptionIndex());
}

void FolderData::BuildFastSearchSeriesDeveloper(FolderData::FastSearchItemSerie& into) const
{
  for(const FileData* game : mChildren)
    if (game->IsFolder()) CastFolder(game)->BuildFastSearchSeriesDeveloper(into);
    else into.Set(game, game->Metadata().DeveloperIndex());
}

void FolderData::BuildFastSearchSeriesPublisher(FolderData::FastSearchItemSerie& into) const
{
  for(const FileData* game : mChildren)
    if (game->IsFolder()) CastFolder(game)->BuildFastSearchSeriesPublisher(into);
    else into.Set(game, game->Metadata().PublisherIndex());
}
