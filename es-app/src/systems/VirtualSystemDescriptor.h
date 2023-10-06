//
// Created by bkg2k on 25/06/23.
//
#pragma once

#include "VirtualSystemType.h"
#include "utils/String.h"
#include "games/classifications/Genres.h"

struct VirtualSystemDescriptor
{
  public:
    //! Simple type descriptor
    explicit VirtualSystemDescriptor(VirtualSystemType type, int index)
      : mType(type)
      , mArcadeDriver()
      , mGenreData(GameGenres::None)
      , mIndex(index)
    {}

    //! Arcade manufacturer descriptor
    explicit VirtualSystemDescriptor(const String& manufacturer, int index)
      : mType(VirtualSystemType::ArcadeManufacturers)
      , mArcadeDriver(manufacturer)
      , mGenreData(GameGenres::None)
      , mIndex(index)
    {}

    //! Genre descriptor
    explicit VirtualSystemDescriptor(GameGenres genre, int index)
      : mType(VirtualSystemType::Genre)
      , mArcadeDriver()
      , mGenreData(genre)
      , mIndex(index)
    {}

    // Default constructor for thread pool
    VirtualSystemDescriptor()
      : mType(VirtualSystemType::Favorites)
      , mArcadeDriver()
      , mGenreData(GameGenres::None)
      , mIndex(0)
    {}

    //! Default copy constructor
    VirtualSystemDescriptor(const VirtualSystemDescriptor& source) = default;
    //! Default move constructor
    VirtualSystemDescriptor(VirtualSystemDescriptor&& source) = default;
    //! Default copy operator
    VirtualSystemDescriptor& operator =(const VirtualSystemDescriptor& source) = default;
    //! Default move operator
    VirtualSystemDescriptor& operator =(VirtualSystemDescriptor&& source) = default;

    //! Get type
    [[nodiscard]] VirtualSystemType Type() const { return mType; }
    //! Get arcade driver
    [[nodiscard]] const String& ArcadeDriver() const { return mArcadeDriver; }
    //! Get genre
    [[nodiscard]] GameGenres Genre() const { return mGenreData; }
    //! Get genre
    [[nodiscard]] int Index() const { return mIndex; }

  private:
    VirtualSystemType mType; //!< Type
    String mArcadeDriver;    //!< Arcade manufacturers
    GameGenres mGenreData;   //!< Genre for ganre systems
    int mIndex;              //!< Virtual system index
};
