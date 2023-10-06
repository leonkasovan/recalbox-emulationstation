//
// Created by bkg2k on 17/06/23.
//

#include <unistd.h>
#include <fcntl.h>
#include "DatContent.h"
#include "utils/Files.h"
#include <systems/SystemData.h>

DatContent::DatContent(const Path& flatDatabasdePath)
{
  Load(flatDatabasdePath);
}

void DatContent::Load(const Path& flatDatabasdePath)
{
  // Build main entry list
  String::List gameList;
  String::List parentList;
  for(const String& line : Files::LoadAllFileLines(flatDatabasdePath))
  {
    DatEntry entry;
    String game, parent;
    if (DatEntry::Deserialize(line, entry, game, parent))
    {
      mEntryList.push_back(entry); // Record rom list
      gameList.push_back(game);    // Keep track of game name (zip)
      gameList.push_back(parent);  // Keep track of parent name (may be empty)
    }
  }

  // Once the list is complete, we can build the map using reference
  for(int i = (int)mEntryList.size(); --i >= 0; )
    mEntryMap[gameList[i]] = &mEntryList[i];

  // And once the map is built, we can set parent references
  for(int i = (int)mEntryList.size(); --i >= 0; )
    if (!parentList[i].empty())
      if (DatEntry** entry = mEntryMap.try_get(parentList[i]); entry != nullptr)
        mEntryList[i].SetParent(*entry);
}

const DatEntry* DatContent::Lookup(const FileData& game)
{
  DatEntry** entry = mEntryMap.try_get(game.Metadata().RomFileOnly().FilenameWithoutExtension());
  if (entry == nullptr) return nullptr;
  return *entry;
}

bool DatContent::Scan(const FileData& game, ScanResult::Result::RomFailList& failures)
{
  // Be optimistic :)
  bool result = true;

  // Processed zip/chd files
  HashSet<String> processedFiles;

  // Lookup DatEntry
  const DatEntry* entry = Lookup(game);
  if (entry == nullptr) return false; // Game not found in dat = not supported at all

  // Now we have information, scan files
  Path gamePath = game.RomPath();
  Zip zippedGame(gamePath, false);
  Zip zippedParent(entry->HasParent() ? gamePath.Directory() / (entry->mParent->Name() + ".zip") : Path::Empty, false);
  for(int i = entry->RomCount(); --i >= 0;)
  {
    const RomFileHolder& rom = entry->Rom(i);
    switch(rom.RomType())
    {
      case RomFileHolder::Type::Bin:
      {
        bool filefound = false;
        unsigned int realcrc32 = 0;
        // Lookup in zipped game
        bool match = LookForRom(zippedGame, rom, filefound, realcrc32);
        if (filefound) processedFiles.insert(rom.RomFile());
        // Not found? look in parent game
        if (!match)
          match = LookForRom(zippedParent, rom, filefound, realcrc32);
        if (!match)
        {
          // Record failure
          if (filefound) failures.push_back(ScanResult::Result::RomFail(rom.RomFile(), rom.RomCrc32(), realcrc32)); // CRC does not match
          else failures.push_back(ScanResult::Result::RomFail(rom.RomFile(), rom.RomCrc32())); // File not found
          result = false; // No match in zip file...
        }
        break;
      }
      case RomFileHolder::Type::Chd:
      {
        Path romPath = game.RomPath();
        Path chdPath = romPath.Directory() / romPath.FilenameWithoutExtension() / rom.RomFile();
        MD5::DigestMd5 md5;
        if (!Md5File(chdPath, md5))  // No CHD file (or error occurred while reading data)
        {
          failures.push_back(ScanResult::Result::RomFail(rom.RomFile(), rom.RomMd5()));
          result = false;
        }
        else
        {
          processedFiles.insert(rom.RomFile());
          if (memcmp(md5, rom.RomMd5(), sizeof(MD5::DigestMd5)) != 0) // MD5 not matching
          {
            failures.push_back(ScanResult::Result::RomFail(rom.RomFile(), rom.RomMd5(), md5));
            result = false;
          }
        }
        break;
      }
      case RomFileHolder::Type::Unknown:
      default:
      {
        { LOG(LogError) << "[DatContent] Unknown DatEntry type!"; }
        return false;
      }
    }
  }

  // Add unknown files for future romset rebuilding
  AddUnknownFile(game.RomPath(), zippedGame, processedFiles, failures);

  return result;
}

bool DatContent::Md5File(const Path& file, MD5::DigestMd5& hash)
{
  MD5 md5;
  unsigned char buffer[256 << 10]; // 256ko

  int chdFile = open(file.ToChars(), O_RDONLY);
  if (chdFile < 0) return false; // CHD missing
  for(unsigned int readbytes = 0; (readbytes = read(chdFile, buffer, sizeof(buffer))) > 0; )
    md5.update(buffer, readbytes);
  close(chdFile);

  md5.finalize();
  memcpy(hash, md5.Output(), sizeof(MD5::DigestMd5));
  return true;
}

bool DatContent::LookForRom(const Zip& zip, const RomFileHolder& rom, [[out]] bool& filefound, [[out]] unsigned int& crc32)
{
  bool result = false;
  for(int rc = zip.Count(); --rc >= 0 && !result; )
    if (String file = zip.FileName(rc).Filename(); file  == rom.RomFile())
    {
      if (filefound = true; rom.RomCrc32() == (unsigned int) zip.Crc32(rc)) result = true;
      else crc32 = (unsigned int) zip.Crc32(rc);
    }
  return result;
}

void DatContent::AddUnknownFile(const Path& romPath, const Zip& zippedGame, const HashSet<String>& processedFiles,
                                ScanResult::Result::RomFailList& failures)
{
  (void)romPath;
  (void)zippedGame;
  (void)processedFiles;
  (void)failures;
}
