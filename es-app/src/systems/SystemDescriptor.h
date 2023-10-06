//
// Created by bkg2k on 01/11/2019.
//
#pragma once

#include <utils/os/fs/Path.h>
#include <emulators/EmulatorList.h>

class SystemDescriptor
{
  public:

    enum class SystemType
    {
      Unknown , //!< ?!
      Arcade  , //!< Arcade system
      Console , //!< Home console
      Handheld, //!< Handheld console
      Computer, //!< Computer
      Fantasy , //!< Fantasy console (non physical system
      Engine  , //!< Game Engine
      Port    , //!< Port
      Virtual , //!< Internal virtual system
      VArcade , //!< Internal virtual Arcade system
    };

    //! Device requirement
    enum class DeviceRequirement
    {
      Unknown    , //!< ?!
      Required   , //!< Can't play without it!
      Recommended, //!< Most games require it
      Optional   , //!< Some games may require it
      None       , //!< Useless
    };

    /*!
     * @brief Default constructor
     */
    SystemDescriptor()
      : mIcon(0)
      , mScreenScraperID(0)
      , mReleaseDate(0)
      , mManufacturer()
      , mType(SystemType::Unknown)
      , mPad(DeviceRequirement::Unknown)
      , mKeyboard(DeviceRequirement::Unknown)
      , mMouse(DeviceRequirement::Unknown)
      , mLightgun(false)
      , mCrtInterlaced(false)
      , mCrtMultiRegion(false)
      , mPort(false)
      , mReadOnly(false)
      , mHasDownloader(false)
      , mIgnoredFiles()
    {
    }

    /*!
     * @brief Set system informations
     * @param guid GUID
     * @param name Short name ("snes")
     * @param fullname Full name ("Super Nintendo Entertainment System")
     * @param readonly Read only system
     * @return This
     */
    SystemDescriptor& SetSystemInformation(const String& guid,
                                           const String& name,
                                           const String& fullname)
    {
      mGUID = guid;
      mName = name;
      mFullName = fullname;

      return *this;
    }

    /*!
     * @brief Set descriptor info
     * @param path Rom path
     * @param extensions supported extensions
     * @param themefolder Theme sub folder
     * @param command Command (may be empty to take the default global command)
     * @return This
     */
    SystemDescriptor& SetDescriptorInformation(const String& path,
                                               const String& extensions,
                                               const String& themefolder,
                                               const String& command,
                                               const String& icon,
                                               bool port,
                                               bool readonly,
                                               bool hasdownloader)
    {
      mPath = Path(path);
      mExtensions = extensions;
      mThemeFolder = themefolder;
      mCommand = command;
      mPort = port;
      mReadOnly = readonly;
      mHasDownloader = hasdownloader;
      mIcon = 0;
      if (int tmp = 0; String(icon).TryAsInt(tmp)) mIcon = tmp;

      return *this;
    }

    /*!
     * @brief Set scraper information
     * @param screenscraperId ScreenScraper system identifier
     * @return This
     */
    SystemDescriptor& SetScraperInformation(int screenscraperId)
    {
      mScreenScraperID = screenscraperId;
      return *this;
    }

    SystemDescriptor& SetPropertiesInformation(const String& systemtype,
                                               const String& pad,
                                               const String& keyboard,
                                               const String& mouse,
                                               const String& releasedate,
                                               const String& manufacturer,
                                               bool lightgun,
                                               bool multiresolution,
                                               bool multiregion,
                                               const String& ignoredfiles)
    {
      mType = ConvertSystemType(systemtype);
      mPad = ConvertDeviceRequirement(pad);
      mKeyboard = ConvertDeviceRequirement(keyboard);
      mMouse = ConvertDeviceRequirement(mouse);
      if (!String(releasedate).Remove('-').TryAsInt(mReleaseDate)) mReleaseDate = 0;
      mManufacturer = manufacturer;
      mLightgun = lightgun;
      mCrtInterlaced = multiresolution;
      mCrtMultiRegion = multiregion;
      mIgnoredFiles = ignoredfiles;
      return *this;
    }

    //! Clear all emulator entries
    SystemDescriptor& ClearEmulators() { mEmulators.Clear(); return *this; }

    /*!
     * @brief Set the whole emulator tree
     * @param emulatorList Emulator/core tree
     */
    void SetEmulatorList(const EmulatorList& emulatorList)
    {
      mEmulators = emulatorList;
    }

    static void SetDefaultCommand(const String& command) { mDefaultCommand = command; }

    /*!
     * @brief Check of the current descriptor is valid and contains all required informations
     * @return True if the descriptor is valid
     */
    bool IsValid()
    {
      return !mGUID.empty() && !mName.empty() && !mPath.IsEmpty() && !mExtensions.empty() && !Command().empty();
    }

    /*
     * Accessors
     */

    [[nodiscard]] const String& GUID() const { return mGUID; }
    [[nodiscard]] const String& Name() const { return mName; }
    [[nodiscard]] const String& FullName() const { return mFullName; }

    [[nodiscard]] const Path& RomPath() const { return mPath; }
    [[nodiscard]] const String& Extension() const { return mExtensions; }
    [[nodiscard]] const String& ThemeFolder() const { return mThemeFolder; }
    [[nodiscard]] const String& Command() const { return mCommand.empty() ? mDefaultCommand : mCommand; }
    [[nodiscard]] unsigned int Icon() const { return (unsigned int)mIcon; }
    [[nodiscard]] String IconPrefix() const;

    [[nodiscard]] int ScreenScaperID() const { return mScreenScraperID; }

    [[nodiscard]] int ReleaseDate() const { return mReleaseDate; }
    [[nodiscard]] const String& Manufacturer() const { return mManufacturer; }

    [[nodiscard]] SystemType Type() const { return mType; }
    [[nodiscard]] DeviceRequirement PadRequirement() const { return mPad; }
    [[nodiscard]] DeviceRequirement KeyboardRequirement() const { return mKeyboard; }
    [[nodiscard]] DeviceRequirement MouseRequirement() const { return mMouse; }
    [[nodiscard]] bool LightGun() const { return mLightgun; }
    [[nodiscard]] bool CrtHighResolution() const { return mCrtInterlaced; }
    [[nodiscard]] bool CrtMultiRegion() const { return mCrtMultiRegion; }
    [[nodiscard]] const String& IgnoredFiles() const { return mIgnoredFiles; }

    [[nodiscard]] bool HasNetPlayCores() const
    {
      for(int i = mEmulators.Count(); --i >= 0; )
        for(int j = mEmulators.EmulatorAt(i).CoreCount(); --j >= 0; )
          if (mEmulators.EmulatorAt(i).CoreNetplay(j))
            return true;
      return false;
    }

    [[nodiscard]] bool IsSoftpatching(const String& emulatorName, const String& coreName) const
    {
      for(int i = mEmulators.Count(); --i >= 0; )
      {
        if(mEmulators.EmulatorAt(i).Name() != emulatorName) continue;
        for (int j = mEmulators.EmulatorAt(i).CoreCount(); --j >= 0;)
          if (mEmulators.EmulatorAt(i).CoreNameAt(j) == coreName && mEmulators.EmulatorAt(i).CoreSoftpatching(j))
            return true;
      }
      return false;
    }

    [[nodiscard]] bool IsPort() const { return mPort; }
    [[nodiscard]] bool IsReadOnly() const { return mReadOnly; }
    [[nodiscard]] bool HasDownloader() const { return mHasDownloader; }

    [[nodiscard]] const EmulatorList& EmulatorTree() const { return mEmulators; }

    //! Is this system an arcade system?
    [[nodiscard]] bool IsArcade() const { return mType == SystemType::Arcade || mType == SystemType::VArcade; };

    //! Is this system an arcade system?
    [[nodiscard]] bool IsTrueArcade() const { return mType == SystemType::Arcade; };

    //! Is this system an arcade system?
    [[nodiscard]] bool IsVirtualArcade() const { return mType == SystemType::VArcade; };

  private:
    static String      mDefaultCommand;  //!< Default command

    // System
    String             mGUID;            //!< System GUID
    String             mName;            //!< Short name ("snes")
    String             mFullName;        //!< Full name ("Super Nintendo Entertainment System")
    // Descriptor
    Path               mPath;            //!< Rom path
    String             mThemeFolder;     //!< Theme sub-folder
    String             mExtensions;      //!< Supported extensions, space separated
    String             mCommand;         //!< Emulator command
    String::Unicode    mIcon;            //!< Icon unicode char
    // Scraper
    int                mScreenScraperID; //!< ScreenScraper ID
    // Properties
    int                mReleaseDate;     //!< Release date in numeric format yyyymm
    String             mManufacturer;    //!< Manufacturer ("Nintendo")
    SystemType         mType;            //!< System type
    DeviceRequirement  mPad;             //!< Pad state
    DeviceRequirement  mKeyboard;        //!< Pad state
    DeviceRequirement  mMouse;           //!< Pad state
    bool               mLightgun;        //!< Support lightgun ?
    bool               mCrtInterlaced;   //!< Support 480i/p ?
    bool               mCrtMultiRegion;  //!< PAL/NTSC ?

    EmulatorList       mEmulators;       //!< Emulator/core tree

    bool               mPort;            //!< This system is a port
    bool               mReadOnly;        //!< This system is a port and is readonly
    bool               mHasDownloader;   //!< System has a downloder
    String             mIgnoredFiles;    //!< Ignored files list

    static SystemType ConvertSystemType(const String& systemtype)
    {
      SystemType result = SystemType::Unknown;
      if      (systemtype == "arcade"  ) result = SystemType::Arcade;
      else if (systemtype == "varcade" ) result = SystemType::VArcade;
      else if (systemtype == "console" ) result = SystemType::Console;
      else if (systemtype == "handheld") result = SystemType::Handheld;
      else if (systemtype == "computer") result = SystemType::Computer;
      else if (systemtype == "virtual" ) result = SystemType::Virtual;
      else if (systemtype == "engine"  ) result = SystemType::Engine;
      else if (systemtype == "port"    ) result = SystemType::Port;
      else if (systemtype == "fantasy" ) result = SystemType::Fantasy;
      else { LOG(LogError) << "[SystemDescriptor] Unknown system type " << systemtype << " !"; }
      return result;
    }

    static DeviceRequirement ConvertDeviceRequirement(const String& requirement)
    {
      DeviceRequirement result = DeviceRequirement::Unknown;
      if      (requirement == "no"         ) result = DeviceRequirement::None;
      else if (requirement == "optional"   ) result = DeviceRequirement::Optional;
      else if (requirement == "recommended") result = DeviceRequirement::Recommended;
      else if (requirement == "mandatory"  ) result = DeviceRequirement::Required;
      else { LOG(LogError) << "[SystemDescriptor] Unknown device requirement " << requirement << " !"; }
      return result;
    }
};
