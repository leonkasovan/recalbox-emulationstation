//
// Created by bkg2k on 15/04/23.
//
#pragma once

#include <utils/storage/HashMap.h>
#include <utils/storage/Array.h>
#include <games/FileData.h>
#include <systems/arcade/ArcadeGame.h>

//! Game database for a given emulator/core
class ArcadeDatabase
{
  public:
    //! Manufacturer index list
    typedef Array<int> IndexList;

    //! Dual manufacturer list
    struct ManufacturerLists
    {
      String::List mLimited; //!< Limited manufacturer list (most important limited list, ordered by game count)
      String::List mRaw;     //!< Raw manufacturer list (all manufacturers, ordered by game count)
      ManufacturerLists(String::List limited, String::List raw) : mLimited(std::move(limited)), mRaw(std::move(raw)) {}
      ManufacturerLists() = default;
    };

    /*!
     * @brief Micro-structure that hold a manufacturer name and an index
     */
    struct Manufacturer
    {
      int Index;
      String Name;
      Manufacturer(int index, const String& name) : Index(index), Name(name) {}
    };

    /*!
     * @brief Constructor
     * @param rawManufacturers Final raw manufacturers
     * @param limitedManufacturers Final limited manufacturers
     * @param games Arcade game spec list
     */
    ArcadeDatabase(String::List&& rawManufacturers, String::List&& limitedManufacturers, Array<ArcadeGame>&& games)
      : mManufacturers(limitedManufacturers, rawManufacturers)
      , mGames(games)
    {
      // Build lookup
      for(int i = games.Count(); --i >= 0;)
        mLookup.insert(&games(i).Game(), i);
    }

    /*!
     * Empty database constructor
     */
    ArcadeDatabase() = default;

    /*!
     * @brief Default destructor
     */
    virtual ~ArcadeDatabase() = default;

    /*!
     * @brief Lookup an arcade game from the source game
     * @param game Source game
     * @return Arcade game or null
     */
    [[nodiscard]] const ArcadeGame* LookupGame(const FileData& game) const
    {
      int* index = mLookup.try_get(&game);
      if (index == nullptr) return nullptr;
      return &mGames.ConstRef(*index);
    }

    /*!
     * @brief Remove any reference to the given game.
     * Delete direct arcade gama associated, and all children orphans
     * @param game FileData reference to remove reference to
     */
    void Remove(const FileData& game)
    {
      for(int i = mGames.Count(); --i >= 0; )
      {
        ArcadeGame& arcade = mGames(i);
        if (&arcade.Game() == &game) mGames.Delete(i);
        else if (arcade.Parent() == &game) arcade.MakeOrphan();
      }
    }

    /*!
     * @brief Check if this database is valid and contains games
     * @return True if there is at least one game
     */
    [[nodiscard]] bool IsValid() const { return !mGames.Empty(); }

    /*!
     * @brief Check if games in this database can be filtered (more than one manufacturer)
     * @return True if there is more than one manufacturer
     */
    [[nodiscard]] bool CanBeFiltered() const { return mManufacturers.mLimited.size() > 1; }

    /*!
     * @brief Get manufacturer name from its index (limited list)
     * @param index manufacturer index
     * @return Manufacturer name or empty string
     */
    [[nodiscard]] const String& LimitedManufacturerNameFromIndex(int index) const { if ((unsigned int)index < mManufacturers.mLimited.size()) return mManufacturers.mLimited[index]; static String nullmanufacturer; return nullmanufacturer; }

    /*!
     * @brief Get manufacturer name from its index (limited list)
     * @param name manufacturer name
     * @return Index or -1
     */
    [[nodiscard]] int LimitedManufacturerIndexFromName(const String& name) const
    {
      for(int i = (int)mManufacturers.mLimited.size(); --i >= 0;)
        if (mManufacturers.mLimited[i] == name)
          return i;
      return -1;
    }

    /*!
     * @brief Get manufacturer name from its index (raw list)
     * @param index manufacturer index
     * @return manufacturer name or empty string
     */
    [[nodiscard]] const String& RawManufacturerNameFromIndex(int index) const { if ((unsigned int)index < mManufacturers.mRaw.size()) return mManufacturers.mRaw[index]; static String nullmanufacturer; return nullmanufacturer; }

    /*!
     * @brief Get manufacturer name from its index (limited list)
     * @param name Manufacturer name
     * @return Index or -1
     */
    [[nodiscard]] int RawManufacturerIndexFromName(const String& name) const
    {
      for(int i = (int)mManufacturers.mRaw.size(); --i >= 0;)
        if (mManufacturers.mRaw[i] == name)
          return i;
      return -1;
    }

    /*!
     * @brief Get manufacturer name from its index (raw list)
     * @param name manufacturer name
     * @return Index or -1
     */
    [[nodiscard]] IndexList RawManufacturerIndexesFromFamilly(const String& name) const
    {
      IndexList result;
      for(int i = (int)mManufacturers.mRaw.size(); --i >= 0;)
      {
        const String& rawManufacturer = mManufacturers.mRaw[i];
        if (rawManufacturer == name) result.Add(i);
        else if (rawManufacturer.Count() > name.Count())
          if (rawManufacturer[name.Count()] == '/')
            if (rawManufacturer.StartsWith(name))
              result.Add(i);
      }
      return result;
    }

    /*!
     * @brief Get manufacturer list (limited list)
     * @return Manufacturer list
     */
    [[nodiscard]] virtual std::vector<Manufacturer> GetLimitedManufacturerList() const
    {
      std::vector<Manufacturer> result;
      for(int i = 0; i < (int)mManufacturers.mLimited.size(); ++i)
        result.push_back(Manufacturer(i, mManufacturers.mLimited[i]));
      return result;
    }

  private:
    ManufacturerLists mManufacturers;      //!< Manufacturer lists
    Array<ArcadeGame> mGames;              //!< Game list
    HashMap<const FileData*, int> mLookup; //!< Reverse lookup FileData* => ArcadeGame
};
