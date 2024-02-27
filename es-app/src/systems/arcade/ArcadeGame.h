//
// Created by bkg2k on 01/04/23.
//
#pragma once

#include <utils/String.h>
#include "games/FileData.h"

class ArcadeGame
{
  public:
    //! 4 manufacturer storage
    template<typename rawType> struct ManufacturerHolder
    {
      public:
        //! Constructor
        ManufacturerHolder() : mManufacturers(0) {}

        /*!
         * @brief Add new manufacturer to the list
         * @param manufacturer
         */
        void Add(int manufacturer)
        {
          if (manufacturer > sMask) { LOG(LogError) << "[ArcadeGame] Storage erreor! Manufacturer index greater than storage space!"; return; }
          int pos = Count();
          if (pos == 4) { LOG(LogError) << "[ArcadeGame] Storage error! No space left in 4 manufacturer storage!"; return; }
          mManufacturers &= ~((rawType)sMask << (sBits * pos + sCountBits));
          mManufacturers |= ((rawType)manufacturer << (sBits * pos + sCountBits));
          mManufacturers++; // Increment counter only
        }

        void SetAt(int manufacturer, int at)
        {
          at &= 3;
          mManufacturers &= ~((rawType)sMask << (sBits * at + sCountBits));
          mManufacturers |= ((rawType)manufacturer << (sBits * at + sCountBits));
        }

        /*!
         * @brief Manufacturer count (0-4)
         * @return Number of stored manufacturers
         */
        [[nodiscard]] int Count() const
        {
          return (int)mManufacturers & sCountMask;
        }

        /*!
         * @brief Check if this holder contains only one zero'ed entry
         * @return True if this holder contain sonly one zero'ed entry, false otherwise
         */
        [[nodiscard]] bool HasOnlyZero() const
        {
          return mManufacturers == 1; // Counter == 1 && all data == 0
        }

        /*!
         * @brief Check if the holder contains the given manufacturer index
         * @param manufacturer Manufacturer index
         * @return True if the holder contains the given manufacturer index, false otherwise
         */
        [[nodiscard]] bool Contains(int manufacturer) const
        {
          rawType tmp = mManufacturers >> sCountBits;
          for(int i = Count(); --i >= 0; tmp >>= sBits)
            if ((int)(tmp & sMask) == manufacturer)
              return true;
          return false;
        }

        /*!
         * @brief Get manufacturer at the given index
         * @param index Index (0-3)
         * @return Manufacturer
         */
        [[nodiscard]] int Manufacturer(int index) const
        {
          index &= 3;
          return (int)((mManufacturers >> (sBits * index + sCountBits)) & sMask);
        }

      private:
        static constexpr int sCountBits = 2;                                      //! Keep 2 bits to store item count
        static constexpr int sCountMask = (1 << sCountBits) - 1;                  //! Counter mask
        static constexpr int sBits = ((int)sizeof(rawType) * 8 - sCountBits) / 4; //! Number of storage bits for a single subdata
        static constexpr int sMask = (1 << sBits) - 1;                            //! Single subdata mask
        rawType mManufacturers;                                                   //!< Raw data holder
    } __attribute__((packed));

    //! Convenient raw manufacturer typedef
    typedef ManufacturerHolder<long long int> RawManufacturerHolder;
    //! Convenient raw manufacturer typedef
    typedef ManufacturerHolder<int> LimitedManufacturerHolder;

    //! Game type
    enum class Type : char
    {
      Parent,   //!< Parent game
      Clone,    //!< Clone game (must link to its parent)
      Orphaned, //!< Clone game but parent unavailable
      Bios,     //!< Bios
    };
    //! Status
    enum class Status: char
    {
      Unknown,
      Good,
      Imperfect,
      Preliminary,
    };
    //! Status
    enum class Rotation: char
    {
      Noon,       //!< No rotation
      ThreeHours, //!< 90° clockwise
      SixHours,   //!< 190° clockwise
      NineHours,  //!< 270° clockwise
    };

    //! Constructor
    ArcadeGame(const FileData* game, const FileData* parent, const String& arcadeName, const ManufacturerHolder<long long int>& rawManufacturers, Type type, Status status, Rotation rotation, unsigned short width, unsigned short height)
      : mGame(game)
      , mParent(parent)
      , mRawManufacturer(rawManufacturers)
      , mNameIndex(sNameHolder.AddString32(arcadeName))
      , mType(type)
      , mStatus(status)
      , mRotation(rotation)
      , mWidth(width)
      , mHeight(height)
    {
    }

    /*!
     * @brief Remap manufacturer
     * @param destinationManufacturer new manufacturer
     * @param at storage index
     */
    void AddLimitedManufacturer(int destinationManufacturer) { mLimitedManufacturer.Add(destinationManufacturer); }

    /*!
     * @brief Decrease manufacturer index at the given position
     * @param index storage index
     */
    void DecLimitedManufacturerAt(int index) { mLimitedManufacturer.SetAt(mLimitedManufacturer.Manufacturer(index) - 1, index); }

    /*
     * Getters
     */

    [[nodiscard]] const FileData& Game() const { return *mGame; }
    [[nodiscard]] const FileData* Parent() const { return mParent; }
    [[nodiscard]] String ArcadeName() const { return sNameHolder.GetString(mNameIndex); }
    [[nodiscard]] const RawManufacturerHolder& RawManufacturer() const { return mRawManufacturer; }
    [[nodiscard]] const LimitedManufacturerHolder& LimitedManufacturer() const { return mLimitedManufacturer; }
    [[nodiscard]] Type Hierarchy() const { return mType; }
    [[nodiscard]] Status EmulationStatus() const { return mStatus; }
    [[nodiscard]] Rotation ScreenRotation() const { return mRotation; }
    [[nodiscard]] unsigned short Width() const { return mWidth; }
    [[nodiscard]] unsigned short Height() const { return mHeight; }

    /*
     * Tools
     */

    /*!
     * @brief If the current game is a clone, dete its parent and make it orphaned
     */
    void MakeOrphan()
    {
      if (mType == Type::Clone && mParent != nullptr)
      {
        mParent = nullptr;
        mType = Type::Orphaned;
      }
    }

    /*!
     * @brief Convert hierarchical type from string tp type
     * @param type Stringized type
     * @param hasParent True return clone type. False return orphaned
     * @return Type
     */
    static Type TypeFromString(const String& type, bool hasParent)
    {
      if (type == "bios") return Type::Bios;
      if (type == "parent") return Type::Parent;
      if (type == "clone" && hasParent) return Type::Clone;
      return Type::Orphaned;
    }

    /*!
     * @brief Get status from the given string
     * @param status Status
     * @return Status
     */
    static Status StatusFromString(const String& status)
    {
      if (status == "good") return Status::Good;
      if (status == "imperfect") return Status::Imperfect;
      if (status == "preliminary") return Status::Preliminary;
      return Status::Unknown;
    }

    /*!
     * @brief Get rotation from string
     * @param rotation Rotation
     * @return Rotation
     */
    static Rotation RotationFromString(const String& rotation)
    {
      switch(rotation.AsInt())
      {
        case 90: return Rotation::ThreeHours;
        case 180: return Rotation::SixHours;
        case 270: return Rotation::NineHours;
        default: break;
      }
      return Rotation::Noon;
    }

    //! Finalize the Name Holder once all database have been loaded
    static void Finalize() { sNameHolder.Finalize(); }

  private:
    const FileData*           mGame;          //!< Game reference - cannot be null
    const FileData*           mParent;        //!< Parent reference or null
    RawManufacturerHolder     mRawManufacturer;     //!< Raw Manufacturer index
    LimitedManufacturerHolder mLimitedManufacturer; //!< Limited Manufacturer index
    int                       mNameIndex;     //!< Name index in name holder
    Type                      mType;          //!< Game type
    Status                    mStatus;        //!< Emulation status
    Rotation                  mRotation;      //!< Rotation
    unsigned short            mWidth;
    unsigned short            mHeight;

    //! Arcade name string holder
    static MetadataStringHolder sNameHolder;
} __attribute__((packed));
