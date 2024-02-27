//
// Created by bkg2k on 01/11/2019.
//
#pragma once

#include <utils/Log.h>
#include <utils/String.h>

class EmulatorDescriptor
{
  public:
    //! Compatibility information
    enum class Compatibility : char
    {
      Unknown, //!< Unknown
      High   , //!< Perfect or near perfect
      Good   , //!< Some games don't work at all or have issues
      Average, //!< Expect to run half of the games
      Low    , //!< only a few games run well and w/o issues
    };

    //! Speed information
    enum class Speed : char
    {
      Unknown, //!< Unknown
      High   , //!< Perfect or near perfect
      Good   , //!< Some games will not run at full speed or may have slowdowns
      Average, //!< Expect around half of the games running at a decent framerate
      Low    , //!< Only a few games are playable
    };

    //! Default constructor
    EmulatorDescriptor()
      : EmulatorDescriptor("")
    {
    }

    /*!
     * @brief Constructor
     * @param emulator Emulator name
     */
    explicit EmulatorDescriptor(const String& emulator)
      : mEmulator(emulator)
    {
    }

    //! Get emulator name
    [[nodiscard]] const String& Name() const { return mEmulator; }

    //! Get core count
    [[nodiscard]] int CoreCount() const { return (int)mCores.size(); }

    //! Has at least one core?
    [[nodiscard]] bool HasAny() const { return !mCores.empty(); }

    /*!
     * @brief Check if the emulator has a core matching the given name
     * @param name Core name
     * @return True of the emulator has this core, fals eotherwise
     */
    [[nodiscard]] bool HasCore(const String& name) const
    {
      for(const Core& core : mCores)
        if (name == core.mName)
          return true;
      return false;
    }

    //! Get core name
    [[nodiscard]] const String& CoreNameAt(int index) const { return CoreAt(index).mName; }
    //! Get core priority
    [[nodiscard]] unsigned char CorePriorityAt(int index) const { return CoreAt(index).mPriority; }
    //! Get core Extensions
    [[nodiscard]] const String& CoreExtensions(int index) const { return CoreAt(index).mExtensions; }
    //! Get core Netplay support
    [[nodiscard]] bool CoreNetplay(int index) const { return CoreAt(index).mNetplay; }
    //! Get core Softpatching availability
    [[nodiscard]] bool CoreSoftpatching(int index) const { return CoreAt(index).mSoftpatching; }
    //! Get core Speed
    [[nodiscard]] Speed CoreSpeed(int index) const { return CoreAt(index).mSpeed; }
    //! Get core Compatibility
    [[nodiscard]] Compatibility CoreCompatibility(int index) const { return CoreAt(index).mCompatibility; }
    //! Get core available on CRT
    [[nodiscard]] bool CoreCrtAvailable(int index) const { return CoreAt(index).mCRTAvailable; }
    //! Get core Flat base file
    [[nodiscard]] String CoreFlatDatabase(int index) const { return CoreAt(index).mFlatBaseName; }
    //! Get core Ignored drivers (in arcade database)
    [[nodiscard]] String CoreIgnoreDrivers(int index) const { return CoreAt(index).mIgnoreDrivers; }
    //! Get core Split drivers (in arcade database)
    [[nodiscard]] String CoreSplitDrivers(int index) const { return CoreAt(index).mSplitDrivers; }
    //! Get core driver limit
    [[nodiscard]] int CoreDriverLimit(int index) const { return CoreAt(index).mLimit; }

    /*!
     * @brief Add core
     * @param name Name (file name part)
     * @param priority Priority (lowest value = highest priority)
     * @param extensions File extensions
     * @param netplay Netplay support?
     * @param compatibility Compatibility evaluation
     * @param speed Speed evaluation
     */
    void AddCore(const String& name,
                 int priority,
                 const String& extensions,
                 bool netplay,
                 const String& compatibility,
                 const String& speed,
                 bool softpatching,
                 bool crtAvailable,
                 const String& flatBaseFile,
                 const String& ignoreDrivers,
                 const String& splitDrivers,
                 int limit)
    {
      mCores.push_back(Core());
      Core& core = mCores.back();
      core.mName = name;
      core.mExtensions = extensions;
      core.mPriority = priority;
      core.mCompatibility = ConvertCompatibility(compatibility);
      core.mSpeed = ConvertSpeed(speed);
      core.mNetplay = netplay;
      core.mSoftpatching = softpatching;
      core.mCRTAvailable = crtAvailable;
      core.mFlatBaseName = flatBaseFile;
      core.mIgnoreDrivers = ignoreDrivers;
      core.mSplitDrivers = splitDrivers;
      core.mLimit = limit;
    }

  private:
    //! Core structure
    struct Core
    {
      // Arcade properties
      String mFlatBaseName;         //!< Flat file base name
      String mIgnoreDrivers;        //!< Flat file base name
      String mSplitDrivers;         //!< Flat file base name
      // Core properties
      String mName;                 //!< Core name (file name)
      String mExtensions;           //!< Supported extensions
      int mPriority;                //!< Core priority
      int mLimit;                   //!< Manufacturer/driver limit
      Compatibility mCompatibility; //!< Compatibility rate
      Speed mSpeed;                 //!< Average speed
      bool mNetplay;                //!< Netplay compatible?
      bool mSoftpatching;           //!< Softpathing compatible?
      bool mCRTAvailable;           //!< Available on CRT?

      //! Constructor
      Core()
        : mPriority(0)
        , mLimit(0)
        , mCompatibility(Compatibility::Unknown)
        , mSpeed(Speed::Unknown)
        , mNetplay(false)
        , mSoftpatching(false)
        , mCRTAvailable(false)
      {
      }

      //! Reset default values
      void Reset()
      {
        mName.clear();
        mExtensions.clear();
        mPriority = 255;
        mCompatibility = Compatibility::Unknown;
        mSpeed = Speed::Unknown;
        mNetplay = false;
        mSoftpatching = false;
        mCRTAvailable = false;
        mFlatBaseName.clear();
        mIgnoreDrivers.clear();
        mSplitDrivers.clear();
        mLimit = 0;
      }
    };

    //! Emulator name
    String mEmulator;
    //! Core specifications
    std::vector<Core> mCores;

    //! Give access to private part from the webmanager process class
    friend class RequestHandlerTools;

    [[nodiscard]] const Core& CoreAt(int index) const { return (unsigned int)index < (unsigned int)mCores.size() ? mCores[index] : mCores[0]; }

    /*!
     * @brief Convert compatibility string to compatibility enum
     * @param compatibility compatibility string
     * @return compatibility enum
     */
    static Compatibility ConvertCompatibility(const String& compatibility)
    {
      Compatibility result = Compatibility::Unknown;
      if      (compatibility == "unknown"  ) result = Compatibility::Unknown;
      else if (compatibility == "high"     ) result = Compatibility::High;
      else if (compatibility == "good"     ) result = Compatibility::Good;
      else if (compatibility == "average"  ) result = Compatibility::Average;
      else if (compatibility == "low"      ) result = Compatibility::Low;
      else { LOG(LogError) << "[SystemDescriptor] Unknown compatibility type " << compatibility << " !"; }
      return result;
    }

    /*!
     * @brief Convert speed string to speed enum
     * @param speed speed string
     * @return speed enum
     */
    static Speed ConvertSpeed(const String& speed)
    {
      Speed result = Speed::Unknown;
      if      (speed == "unknown"  ) result = Speed::Unknown;
      else if (speed == "high"     ) result = Speed::High;
      else if (speed == "good"     ) result = Speed::Good;
      else if (speed == "average"  ) result = Speed::Average;
      else if (speed == "low"      ) result = Speed::Low;
      else { LOG(LogError) << "[SystemDescriptor] Unknown Speed type " << speed << " !"; }
      return result;
    }
};

