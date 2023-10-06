//
// Created by bkg2k on 17/06/23.
//

#pragma once

#include <utils/String.h>
#include <systems/arcade/dats/DatEntry.h>

class ScanResult
{
  public:
    //! Emulation status
    enum Status
    {
      Error,        //!< Something went wrong during scanning...
      NotSupported, //!< Game is unsupported
      Preliminar,   //!< Preliminar support. May have severe bugs
      Imperfect,    //!< Imperfect support. May have bugs
      Good,         //!< Good support
      Unknown,      //!< Game is supported but database has no accurate information
    };

    struct Result
    {
      public:
        enum class TypeFail
        {
          None,        //!< No error - useless except for error catching
          UnknownFile, //!< Unknown file in zip or chd folder
          RomBadHash,  //!< File found, but hash does not match database one
          RomNotFound, //!< File not found - a required file is not available in the zip
          ChdBadHash,  //!< File found, but hash does not match database one
          ChdNotFound, //!< File not found - a required file is not available in tits folder
        };
        struct RomFail
        {
          const String mRom;
          const RomFileHolder::HashUnion mExpectedHash;
          const RomFileHolder::HashUnion mRealHash;
          const TypeFail mFailure;

          //! Null constructor
          RomFail() : mRom(), mExpectedHash(0), mRealHash(0), mFailure(TypeFail::None) {}

          //! Rom not found constructor
          RomFail(const String& romNotFound, unsigned int crc32)
            : mRom(romNotFound)
            , mExpectedHash(crc32)
            , mRealHash(0)
            , mFailure(TypeFail::RomNotFound)
          {
          }

          //! Chd not found constructor
          RomFail(const String& romNotFound, const MD5::DigestMd5& md5)
            : mRom(romNotFound)
            , mExpectedHash(md5)
            , mRealHash(0)
            , mFailure(TypeFail::ChdNotFound)
          {
          }

          //! Bad rom constructor
          RomFail(const String& romBadHash, unsigned int expectedcrc32, unsigned int realcrc32)
            : mRom(romBadHash)
            , mExpectedHash(expectedcrc32)
            , mRealHash(realcrc32)
            , mFailure(TypeFail::RomBadHash)
          {
          }

          //! Chd not found constructor
          RomFail(const String& romBadHash, const MD5::DigestMd5& expectedmd5, const MD5::DigestMd5& realdmd5)
            : mRom(romBadHash)
            , mExpectedHash(expectedmd5)
            , mRealHash(realdmd5)
            , mFailure(TypeFail::ChdBadHash)
          {
          }

          //! Unknown rom file constructor
          RomFail(unsigned int crc32, const String& unknownFile)
            : mRom(unknownFile)
            , mExpectedHash(0)
            , mRealHash(crc32)
            , mFailure(TypeFail::UnknownFile)
          {
          }

          //! Unknown chd file constructor
          RomFail(MD5::DigestMd5& md5, const String& unknownFile)
            : mRom(unknownFile)
            , mExpectedHash(0)
            , mRealHash(md5)
            , mFailure(TypeFail::UnknownFile)
          {
          }
        };

        //! Fail list type
        typedef std::vector<RomFail> RomFailList;

        //! Get emulator short name
        [[nodiscard]] const String& Emulator() const { return mEmulator; }
        //! Get core short name
        [[nodiscard]] const String& Core() const { return mCore; }
        //! Get emulation status
        [[nodiscard]] ScanResult::Status Status() const { return mStatus; }
        //! Get detailled failure count
        [[nodiscard]] int FailCount() const { return (int)mFailures.size(); }
        //! Get detailled failure count
        [[nodiscard]] const RomFail& FailAt(int index) const
        {
          if ((unsigned int)index < mFailures.size()) return mFailures[index];
          static RomFail __nullfail;
          return __nullfail;
        }

        //! Public null constructor
        Result()
          : mEmulator()
          , mCore()
          , mStatus(Status::NotSupported)
        {}

      private:
        friend class DatManager;

        String mEmulator;           //!< Emulator short name
        String mCore;               //!< Core short name
        ScanResult::Status mStatus; //!< Emulation status
        RomFailList mFailures;      //!< Detailed failures

        /*!
         * @brief Private constructor
         * @param emulator Emulator short name
         * @param core Core short name
         * @param status Emulation status
         */
        Result(const String& emulator, const String& core, ScanResult::Status status, RomFailList&& failures)
          : mEmulator(emulator)
          , mCore(core)
          , mStatus(status)
          , mFailures(failures)
        {}
    };

    //! Get result count
    [[nodiscard]] int Count() const { return (int)mResults.size(); }

    [[nodiscard]] const Result& ResultAt(int index) const
    {
      if ((unsigned int)index < mResults.size()) return mResults[index];
      static Result __nullresult;
      return __nullresult;
    }

  private:
    friend class DatManager;

    //! Result array
    std::vector<Result> mResults;
};
