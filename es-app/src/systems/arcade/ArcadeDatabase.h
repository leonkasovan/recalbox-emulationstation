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
    //! Driver index list
    typedef Array<int> IndexList;

    //! Dual driver list
    struct DriverLists
    {
      String::List mLimited; //!< Limited driver list (most important limited list, ordered by game count)
      String::List mRaw;     //!< Raw driver list (all driver, ordered by game count)
      DriverLists(String::List limited, String::List raw) : mLimited(std::move(limited)), mRaw(std::move(raw)) {}
      DriverLists() = default;
    };

    /*!
     * @brief Micro-structure that hold a driver name and an index
     */
    struct Driver
    {
      int Index;
      String Name;
      Driver(int index, const String& name) : Index(index), Name(name) {}
    };

    /*!
     * @brief Constructor
     * @param drivers Final drivers
     * @param games Arcade game spec list
     * @param lookup Lookup map
     */
    ArcadeDatabase(String::List&& rawDrivers, String::List&& limitedDrivers, Array<ArcadeGame>&& games)
      : mDrivers(limitedDrivers, rawDrivers)
      , mGames(games)
      , mLookup()
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
     * @brief Check if games in this database can be filtered (more than one driver)
     * @return True if there is more than one driver
     */
    [[nodiscard]] bool CanBeFiltered() const { return mDrivers.mLimited.size() > 1; }

    /*!
     * @brief Get driver name from its index (limited list)
     * @param index driver index
     * @return Driver name or empty string
     */
    [[nodiscard]] const String& LimitedDriverNameFromIndex(int index) const { if ((unsigned int)index < mDrivers.mLimited.size()) return mDrivers.mLimited[index]; static String __nulldriver; return __nulldriver; }

    /*!
     * @brief Get driver name from its index (limited list)
     * @param name Driver name
     * @return Index or -1
     */
    [[nodiscard]] int LimitedDriverIndexFromName(const String& name) const
    {
      for(int i = (int)mDrivers.mLimited.size(); --i >= 0;)
        if (mDrivers.mLimited[i] == name)
          return i;
      return -1;
    }

    /*!
     * @brief Get driver name from its index (raw list)
     * @param index driver index
     * @return Driver name or empty string
     */
    [[nodiscard]] const String& RawDriverNameFromIndex(int index) const { if ((unsigned int)index < mDrivers.mRaw.size()) return mDrivers.mRaw[index]; static String __nulldriver; return __nulldriver; }

    /*!
     * @brief Get driver name from its index (raw list)
     * @param name Driver name
     * @return Index or -1
     */
    [[nodiscard]] IndexList RawDriverIndexesFromFamilly(const String& name) const
    {
      IndexList result;
      for(int i = (int)mDrivers.mRaw.size(); --i >= 0;)
      {
        const String& rawDriver = mDrivers.mRaw[i];
        if (rawDriver == name) result.Add(i);
        else if (rawDriver.Count() > name.Count())
          if (rawDriver[name.Count()] == '/')
            if (rawDriver.StartsWith(name))
              result.Add(i);
      }
      return result;
    }

    /*!
     * @brief Get driver list (limited list)
     * @return Driver list
     */
    [[nodiscard]] virtual std::vector<Driver> GetLimitedDriverList() const
    {
      std::vector<Driver> result;
      for(int i = 0; i < (int)mDrivers.mLimited.size(); ++i)
        result.push_back(Driver(i, mDrivers.mLimited[i]));
      return result;
    }

  private:
    DriverLists mDrivers;                  //!< Driver lists
    Array<ArcadeGame> mGames;              //!< Game list
    HashMap<const FileData*, int> mLookup; //!< Reverse lookup FileData* => ArcadeGame
};
