//
// Created by bkg2k on 01/04/23.
//
#pragma once

#include <utils/String.h>
#include "games/FileData.h"

class ArcadeGame
{
  public:
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
    ArcadeGame(const FileData* game, const FileData* parent, const String& arcadeName, int rawdriver, Type type, Status status, Rotation rotation)
      : mGame(game)
      , mParent(parent)
      , mNameIndex(sNameHolder.AddString32(arcadeName))
      , mRawDriver(rawdriver)
      , mLimitedDriver(0)
      , mType(type)
      , mStatus(status)
      , mRotation(rotation)
    {
    }

    /*!
     * @brief Remap driver
     * @param destinationDriver new driver
     */
    void SetLimitedDriver(int destinationDriver) { mLimitedDriver = (unsigned char)destinationDriver; }

    /*
     * Getters
     */

    [[nodiscard]] const FileData& Game() const { return *mGame; }
    [[nodiscard]] const FileData* Parent() const { return mParent; }
    [[nodiscard]] String ArcadeName() const { return sNameHolder.GetString(mNameIndex); }
    [[nodiscard]] int RawDriver() const { return (int)mRawDriver; }
    [[nodiscard]] int LimitedDriver() const { return (int)mLimitedDriver; }
    [[nodiscard]] Type Hierarchy() const { return mType; }
    [[nodiscard]] Status EmulationStatus() const { return mStatus; }
    [[nodiscard]] Rotation ScreenRotation() const { return mRotation; }

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
    const FileData* mGame;          //!< Game reference - cannot be null
    const FileData* mParent;        //!< Parent reference or null
    int             mNameIndex;     //!< Name index in name holder
    unsigned short  mRawDriver;     //!< Raw Driver index
    unsigned short  mLimitedDriver; //!< Raw Driver index
    Type            mType;          //!< Game type
    Status          mStatus;        //!< Emulation status
    Rotation        mRotation;      //!< Rotation

    //! Arcade name string holder
    static MetadataStringHolder sNameHolder;
};
