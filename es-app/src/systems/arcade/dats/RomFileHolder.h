//
// Created by bkg2k on 19/06/23.
//
#pragma once

#include <utils/hash/Md5.h>
#include <utils/String.h>

//! Single rom file structure
class RomFileHolder
{
  public:
    enum class Type
    {
      Unknown, //!< Unknown or unset
      Bin,     //!< Binary roms
      Chd,     //!< Chd file
    };

    union HashUnion
    {
      unsigned int mCrc32; //!< Crc32 (4 bytes)
      MD5::DigestMd5 mMd5; //!< Md5 (16 bytes)

      //! CRC32 constructor
      explicit HashUnion(unsigned crc32) : mCrc32(crc32) {}
      //! MD5 constructor
      explicit HashUnion(const MD5::DigestMd5& md5) : mCrc32(0){ memcpy(mMd5, md5, sizeof(MD5::DigestMd5)); }
    };

    //! CRC constructor
    RomFileHolder(const String& romFile, unsigned int crc32)
      : mRomFile(romFile)
      , mHashes(crc32)
      , mType(Type::Bin)
    {
    }

    //! MD5 constructor
    RomFileHolder(const String& romFile, const MD5::DigestMd5& md5)
      : mRomFile(romFile)
      , mHashes(md5)
      , mType(Type::Bin)
    {

    }

    //! Get rom file
    [[nodiscard]] const String& RomFile() const { return mRomFile; }
    //! Get rom type
    [[nodiscard]] Type RomType() const { return mType; }
    //! Get crc32
    [[nodiscard]] unsigned int RomCrc32() const { return mHashes.mCrc32; }
    //! Get Md5
    [[nodiscard]] const MD5::DigestMd5& RomMd5() const { return mHashes.mMd5; }

  private:
    String mRomFile;   //!< Rom name w/ its extension
    HashUnion mHashes; //!< Hashes
    Type mType;        //!< Rom type
};

