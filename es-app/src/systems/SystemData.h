#pragma once

#include <utils/cplusplus/INoCopy.h>
#include <emulators/EmulatorList.h>
#include <games/RootFolderData.h>
#include <WindowManager.h>
#include <systems/SystemDescriptor.h>
#include <themes/ThemeData.h>
#include <systems/arcade/ArcadeDatabaseManager.h>
#include "games/MetadataType.h"
#include "VirtualSystemType.h"
#include <systems/SystemDataBase.h>

class SystemManager;

class SystemData : private INoCopy
                 , public SystemDataBase
{
  public:
    //! System properties
    enum class Properties
    {
      None       =  0, //!< No properties
      Ports      =  1, //!< This system is the special "Ports" system
      Favorite   =  2, //!< This system is the special "Favorite" system
      Virtual    =  4, //!< This system is not a real system
      FixedSort  =  8, //!< This system has its own fixed sort
      AlwaysFlat = 16, //!< This system is presented always flat
      Searchable = 32, //!< We can search games in this system
      GameInPng  = 64, //!< Game is its image and vice-versa :)
      ScreenShots = 128, //! < This system is the special "Screenshots" system
    };

	private:
    // Allow manager to access this class
    friend class SystemManager;

    //! Parent manager
    SystemManager& mSystemManager;

    //! Descriptor
    SystemDescriptor mDescriptor;
    //! Theme object
    ThemeData mTheme;
    //! Root folders - Children are top level visible game/folder of the system
    RootFolderData mRootOfRoot;
    //! Is this system the favorite system?
    Properties mProperties;
    //! Fixed sort
    FileSorts::Sorts mFixedSort;
    //! Arcade database
    ArcadeDatabaseManager mArcadeDatabases;
    //! Metadata sensitivity
    MetadataType mSensitivity;
    //! Virtual type
    VirtualSystemType mVirtualType;

    /*!
     * @brief Populate the system using all available folder/games by gathering recursively
     * all files mathing the extension list
     * @param folder Root folder to recurse in
     * @param doppelgangerWatcher full path map to avoid adding a game more than once
     */
    void populateFolder(RootFolderData& folder, FileData::StringMap& doppelgangerWatcher);

    /*!
     * @brief Private constructor, called from SystemManager - Regular systems only
     * @param systemManager System manager reference
     * @param systemDescriptor system descriptor
     * @param properties System properties
     * @param sensitivity Virtual systems must refresh when these metadata changes in any games
     * @param fixedSort Fixed sort for system that have a forced fixed sort in properties
     */
    SystemData(SystemManager& systemManager, const SystemDescriptor& systemDescriptor, Properties properties);

    /*!
     * @brief Private constructor, called from SystemManager - Virtual systems  only
     * @param systemManager System manager reference
     * @param systemDescriptor system descriptor
     * @param properties System properties
     * @param sensitivity Virtual systems must refresh when these metadata changes in any games
     * @param virtualType Virtual type
     * @param fixedSort Fixed sort for system that have a forced fixed sort in properties
     */
    SystemData(SystemManager& systemManager, const SystemDescriptor& systemDescriptor, Properties properties, MetadataType sensitivity, VirtualSystemType virtualType, FileSorts::Sorts fixedSort = FileSorts::Sorts::FileNameAscending);

    /*!
     * @brief Lookup an existing game entry (or create it) in the current system.
     * @param root Game root path (usually system root path)
     * @param path Game path
     * @param type Type (folder/game)
     * @param doppelgangerWatcher Maps to avoid duplicate entries
     * @return Existing or newly created FileData
     */
    FileData* LookupOrCreateGame(RootFolderData& topAncestor, const Path& rootPath, const Path& path, ItemType type, FileData::StringMap& doppelgangerWatcher) const;

    /*!
     * @brief Parse xml gamelist files and add games to the current system
     * @param root Root rom folder
     * @param doppelgangerWatcher Maps to avoid duplicate entries
     * @param forceCheckFile True to force to check if file exists
     */
    void ParseGamelistXml(RootFolderData& root, FileData::StringMap& doppelgangerWatcher, bool forceCheckFile);

    /*!
     * @brief Get root folder of the given type
     * @param type root type
     * @return Found root or nullptr
     */
    RootFolderData* GetRootFolder(RootFolderData::Types type);

    /*!
     * @brief Get root folder of the given type
     * @param root root path
     * @return Found root or nullptr
     */
    RootFolderData* GetRootFolder(const Path& root);

    /*!
     * @brief Create new root folder
     * @param startpath Path
     * @param childownership Child ownership type
     * @param type Type of root
     * @return New root folder
     */
    RootFolderData& CreateRootFolder(const Path& startpath, RootFolderData::Ownership childownership, RootFolderData::Types type);

    /*!
     * @brief Lookup an existig root folder or create a new one using the given configuration
     * @param startpath Path
     * @param childownership Child ownership type
     * @param type Type of root
     * @return Existing or new root folder
     */
    RootFolderData& LookupOrCreateRootFolder(const Path& startpath, RootFolderData::Ownership childownership, RootFolderData::Types type);

  public:
    //! Load arcade databases
    void LoadArcadeDatabase() { mArcadeDatabases.LoadDatabases(); }

    //! Get Arcade database manager
    [[nodiscard]] const ArcadeDatabaseManager& ArcadeDatabases() const { return mArcadeDatabases; }

    /*!
     * @brief Check if we must include adult games or not
     * @return True to include adult games in game lists
     */
    [[nodiscard]] bool IncludeAdultGames() const;

    /*!
     * @brief Get master root
     * @return Master root
     */
    RootFolderData& MasterRoot() { return mRootOfRoot; }
    /*!
     * @brief Get master root - const version
     * @return Master root
     */
    [[nodiscard]] const RootFolderData& MasterRoot() const { return mRootOfRoot; }

    /*!
     * @brief Get system descriptor
     * @return System descriptor
     */
    [[nodiscard]] const SystemDescriptor& Descriptor() const { return mDescriptor; }

    //! Get system name
    [[nodiscard]] const String& Name() const { return mDescriptor.Name(); }
    //! Get full name
    [[nodiscard]] const String& FullName() const { return mDescriptor.FullName(); }
    //! Theme folder
    [[nodiscard]] const String& ThemeFolder() const { return mDescriptor.ThemeFolder(); }
    //! Get system rotation option in system view (tate mode)
    [[nodiscard]] bool Rotatable() const;

    //! Has favorite in theme?
    // TODO: Please kill me asap!
    [[nodiscard]] bool HasFavoritesInTheme() const { return mTheme.getHasFavoritesInTheme(); }

    [[nodiscard]] FileData::List getFavorites() const;
    [[nodiscard]] FileData::List getAllGames() const;
    [[nodiscard]] FileData::List getTopGamesAndFolders() const;

    [[nodiscard]] inline const ThemeData& Theme() const { return mTheme; }

    static Path getGamelistPath(const RootFolderData& root, bool forWrite);

    /*!
     * @brief Get list of writable Gamelists
     * @return List of writable gamelists
     */
    Path::PathList WritableGamelists();
    [[nodiscard]] Path getThemePath() const;

    [[nodiscard]] bool HasGame() const;
    [[nodiscard]] bool HasVisibleGame() const;

    /*!
    * @brief Check if system has no only RO games
    * @return if has no only RO games
    */
    [[nodiscard]] bool HasScrapableGame() const;
    /*!
     * @brief Count all folder & games recursively w/o any filter
     * @return Complete game & folder count
     */
    [[nodiscard]] int CountAll() const { return mRootOfRoot.CountAll(); }
    /*!
     * @brief Count all visible games, favorites & hidden
     * @param favorites Output favorites count
     * @param hidden Output hidden count
     * @return
     */
    [[nodiscard]] int GameCount([[out]] int& favorites, [[out]] int& hidden) const;

    // Load or re-load theme.
    void loadTheme();

    [[nodiscard]] const EmulatorList& Emulators() const { return mDescriptor.EmulatorTree(); }

    //! Is this system auto scrapable? (Games in png?)
    [[nodiscard]] bool IsAutoScrapable() const;

    //! Is this system the "Favorite" system?
    [[nodiscard]] bool IsFavorite() const;

    //! Is this system the "Ports" system?ScreenShots
    [[nodiscard]] bool IsPorts() const;

    //! Is this the Screenshots system
    [[nodiscard]] bool IsScreenshots() const;

    //! Is this the Last Played system
    [[nodiscard]] bool IsLastPlayed() const;

    //! Is this system virtual?
    [[nodiscard]] bool IsVirtual() const;

    //! Is this system selt sorted
    [[nodiscard]] bool IsSelfSorted() const;

    //! Is this system always flat?
    [[nodiscard]] bool IsAlwaysFlat() const;

    //! Is this system always flat?
    [[nodiscard]] bool IsSearchable() const;

    //! Is this system an arcade system?
    [[nodiscard]] bool IsArcade() const { return mDescriptor.IsArcade(); };

    //! Is this system an arcade system?
    [[nodiscard]] bool IsTrueArcade() const { return mDescriptor.IsTrueArcade(); };

    //! Is this system an arcade system?
    [[nodiscard]] bool IsVirtualArcade() const { return mDescriptor.IsVirtualArcade(); };

    //! Get virtual type
    [[nodiscard]] VirtualSystemType VirtualType() const { return mVirtualType; }

    //! Get fixed sort
    [[nodiscard]] FileSorts::Sorts FixedSort() const { return mFixedSort; }

    [[nodiscard]] MetadataType MetadataSensitivity() const { return mSensitivity; }

    /*!
     * @brief Write modified games back to the gamelist xml file
     */
    void UpdateGamelistXml();

    /*!
     * @brief Update game list with a single game on top of the list
     * @param game game to insert or move
     */
    void UpdateLastPlayedGame(FileData& game);

    /*!
     * @brief Rebuild a complete map path/FileData recursively
     * @param doppelganger Map to fill in
     * @param includefolder Include folder or not
     */
    void BuildDoppelgangerMap(FileData::StringMap& doppelganger, bool includefolder) const;

    /*!
     * @brief Get parent system manager
     * @return
     */
    [[nodiscard]] SystemManager& Manager() const { return mSystemManager; }

    /*!
     * @brief Search for all games containing 'text' and add them to 'result'
     * @param context Field in which to search text for
     * @param indexes item indexes resulting from a fast search
     * @param games Resulting game list
     */
    void LookupGames(FolderData::FastSearchContext context, const MetadataStringHolder::IndexAndDistance& index, FileData::List& games) const;

    /*!
     * @brief Fill in the given Item series with all filedata recursively
     * @param into Item series to fill in
     * @param context Context for indexes
     */
    void BuildFastSearchSeries(FolderData::FastSearchItemSerie& into, FolderData::FastSearchContext context) const;

    /*!
     * @brief Get excludes filter
     * @return excludes Filter
     */
    [[nodiscard]] FileData::Filter Excludes() const;

    /*!
     * @brief Dirty code to quicky identify GB systems
     * @return True if the system is GB et GBC, false otherwise
     */
    [[nodiscard]] bool IsGameBoy() const { return (Name() == "gb"); }

    /*!
     * @brief Remove all arcade references to the given game
     * @param game Game to remove reference to
     */
    void RemoveArcadeReference(const FileData& game);
};

DEFINE_BITFLAG_ENUM(SystemData::Properties, int)
