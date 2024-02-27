//
// Created by bkg2k on 12/03/23.
//
#pragma once

#include <utils/String.h>
#include "ArcadeGame.h"
#include "utils/storage/Set.h"
#include "ArcadeDatabase.h"

// Forward declaration
class SystemData;

class ArcadeDatabaseManager
{
  public:
    /*!
     * @brief Load arcade databases for a given system
     * @param parentSystem Parent system
     */
    explicit ArcadeDatabaseManager(SystemData& parentSystem);

    // Destructor
    ~ArcadeDatabaseManager();

    /*!
     * @brief Load all arcade database
     */
    void LoadDatabases();

    /*!
     * @brief Lookup an arcade database for the current system
     * regarding only default system configuration to find out what emulator & core to use
     * @return GameDatabase or null
     */
    [[nodiscard]] const ArcadeDatabase* LookupDatabase() const;

    /*!
     * @brief Lookup an arcade database for the current game
     * regarding all overridden configuration to find out what emulator & core to use
     * @param game Game from which lookup emulator configuration
     * @param emulatorName Filled with the default emulator name
     * @param coreName Filled with the default core name
     * @return GameDatabase or null
     */
    [[nodiscard]] const ArcadeDatabase* LookupDatabase(const FileData& game, String& emulatorName, String& coreName) const;

    /*!
     * @brief Lookup an arcade database for the current system in given current folder
     * regarding all overridden configuration to find out what emulator & core to use
     * @param folder folder to lookup emulator configuration in
     * @param emulatorName Filled with the default emulator name for the given folder
     * @param coreName Filled with the default core name for the given folder
     * @return GameDatabase or null
     */
    [[nodiscard]] const ArcadeDatabase* LookupDatabase(const FolderData& folder, String& emulatorName, String& coreName) const;

    /*!
     * @brief Lookup an arcade database for the current system in given current folder
     * regarding all overridden configuration to find out what emulator & core to use
     * @return GameDatabase or null
     */
    [[nodiscard]] const ArcadeDatabase* LookupDatabase(const FolderData& folder) const;

    /*!
     * @brief Lookup an arcade database for a particulat emulator and a particular core
     * @param emulatorName Filled with the default emulator name for the given game
     * @param coreName Filled with the default core name for the given game
     * @return GameDatabase or null
     */
    [[nodiscard]] const ArcadeDatabase* LookupDatabaseFor(const String& emulatorName, const String& coreName) const;

    /*!
     * @brief Remove all reference to the given game, from all database
     * @param game Game to remove
     */
    void RemoveGame(const FileData& game);

    /*!
     * @brief Call it once all database have been loaded to save memory
     */
    static void Finalize() { ArcadeGame::Finalize(); }

    /*!
     * @brief Check if the current database manager contains at least one database
     * @return True if the manager contains no database, false otherwise
     */
    [[nodiscard]] bool IsEmpty() const { return mDatabases.empty(); }

  private:
    //! Manufacturer limit
    static constexpr int sManufacturerLimits = 21; // 20 + special all-remaining-manufacturers at index 0

    //! Raw Manufacturer structure
    struct RawManufacturer
    {
      String mName;   //!< Manufacturer name
      int mIndex;     //!< Manufacturer index
      int mGameCount; //!< Game from this Manufacturer
    };

    //! Map type
    typedef HashMap<String, RawManufacturer> ManufacturerMap;

    //! System reference
    SystemData& mSystem;

    //! Arcade database typedef
    typedef HashMap<String, ArcadeDatabase*> Databases;
    //! Database per core per emulator
    Databases mDatabases;
    //! Loaded?
    bool mReady;

    /*!
     * @brief Load flat database for the given emulator/core
     * @param emulator Emulator name
     * @param core Core name
     * @param splitDriverString raw split driver string list
     * @param ignoredManufacturerString raw ignored driver string list
     * @result GameDatabase
     */
    ArcadeDatabase* LoadFlatDatabase(const String& emulator, const String& core, const String& databaseFilename,
                                     const String& ignoredManufacturerString);

    /*!
     * @brief Deserializea line into an ArcadeGame structure
     * @param database Target database
     * @param line Text line to deserialize
     * @param map Map used to lookup FileData
     * @param drivers raw driver map used in loading process
     * @param nextDriverIndex next driver index
     */
    static void DeserializeTo(Array<ArcadeGame>& games, const String& line, const HashMap<String, FileData*>& map,
                              ManufacturerMap& manufacturerMap, const HashSet<String>& ignoreDrivers, int& nextDriverIndex);

    /*!
     * @brief Build final driver list & remap drivers in existing games
     * @param map Raw driver map
     * @param array Game array
     * @param limit maximum final drivers
     * @param rawDriverCount raw driver count
     * @return Final driver list
     */
    static ArcadeDatabase::ManufacturerLists BuildAndRemapManufacturers(const ManufacturerMap& map, Array<ArcadeGame>& games, int rawDriverCount);

    /*!
     * @brief Try to assign best matching names to unamed games
     * regarding the core the game is assigned to (default, or forced in any way)
     */
    void AssignNames();
};
