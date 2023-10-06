//
// Created by bkg2k on 17/06/23.
//

#include "DatEntry.h"
#include <utils/Log.h>

/*
  Line Format:

  All: 2spicy||317-0491-com.bin||6.0.0009.bin|5ffdfbf8|6.0.0010.bin|ea2bf888|6.0.0010a.bin|10dd9b76|fpr-24370b.ic6|c3b021a4|vid_bios.u504|f78d14d7|dvp-0027a.chd|da1aacee9e32e813844f4d434981e69cc5c80682|mda-c0004a_revb_lindyellow_v2.4.20_mvl31a_boot_2.01.chd|e13da5f827df852e742b594729ee3f933b387410

  0: game name (w/o zip)
  1: parent name (w/o zip)
  2: repeated:
     2.1: rom file w/ extension
     2.2: crc32 or md5 or nothing
*/

bool DatEntry::Deserialize(const String& line, [[out]] DatEntry& to, [[out]] String& game, [[out]] String& parent)
{
  to.mRomList.clear();
  to.mParent = nullptr;

  if (String field, remaining; line.Extract('|', game, remaining, false))
    if (String rom, hash; remaining.Extract('|', parent, remaining, false))
    {
      to.mFileName = game;
      bool romOk = true, hashOk = true;
      while(romOk && hashOk)
      {
        romOk = remaining.Extract('|', rom, remaining, false);
        hashOk = remaining.Extract('|', hash, remaining, false);
        if (romOk && hashOk) to.AddRom(rom, hash);
        else { LOG(LogError) << "[DatEntry] Incomplete rom in line line: " << line; }
      }
      return !romOk && !hashOk;
    }

  { LOG(LogError) << "[DatEntry] Malformatted line: " << line; }
  return false;
}

void DatEntry::AddRom(const String& rom, const String& hash)
{
  // Check type
  RomFileHolder::Type type = RomFileHolder::Type::Bin;
  if (rom.EndsWith(".chd")) type = RomFileHolder::Type::Chd;

  // Get hash
  switch(type)
  {
    case RomFileHolder::Type::Bin:
    {
      mRomList.push_back(RomFileHolder(rom, String('$').Append(hash).AsInt()));
      break;
    }
    case RomFileHolder::Type::Chd:
    {
      MD5::DigestMd5 md5;
      for(int i = 16; --i>= 0; )
        md5[i] = (unsigned char)String('$').Append(hash.SubString(i * 2, 2)).AsInt();
      mRomList.push_back(RomFileHolder(rom, md5));
    }
    case RomFileHolder::Type::Unknown:
    default: break;
  }

  // Store

}
