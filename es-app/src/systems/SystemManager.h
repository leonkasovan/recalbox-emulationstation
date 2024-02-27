#pragma once

#include <utils/cplusplus/INoCopy.h>
#include <systems/SystemData.h>
#include <emulators/EmulatorManager.h>
#include "hardware/devices/mount/MountMonitor.h"
#include <utils/os/system/IThreadPoolWorkerInterface.h>
#include <views/IProgressInterface.h>
#include "IRomFolderChangeNotification.h"
#include "SystemHasher.h"
#include "VirtualSystemDescriptor.h"
#include "VirtualSystemResult.h"
#include "ISystemLoadingPhase.h"
#include "ISystemChangeNotifier.h"

class SystemManager : private INoCopy // No copy allowed
                    , public IThreadPoolWorkerInterface<SystemDescriptor, SystemData*> // Multi-threaded system loading
                    , public IThreadPoolWorkerInterface<SystemData*, bool> // Multi-threaded system unloading
                    , public IThreadPoolWorkerInterface<VirtualSystemDescriptor, VirtualSystemResult> // Multi-threaded system unloading
                    , public IMountMonitorNotifications
                    , public ISlowSystemOperation
{
  public:
    //! Requested Visibility
    enum class Visibility
    {
      Show,          //! Show the system
      ShowAndSelect, //! Show and select the system
      Hide,          //! Hide the system
    };

    //! Convenient alias for System list
    typedef Array<SystemData*> List;
    //! Convenient alias for SystemDescriptor list
    typedef std::vector<SystemDescriptor> DescriptorList;

    //! Arcade manufacturer virtual system's name prefix
    static constexpr const char* sArcadeManufacturerPrefix = "arcade-manufacturer-";
    //! Genre virtual system's name prefix
    static constexpr const char* sGenrePrefix = "genre-";

    //! Favorite system internal name
    static constexpr const char* sFavoriteSystemShortName = "favorites";
    //! Multiplayer system internal name
    static constexpr const char* sMultiplayerSystemShortName = "multiplayer";
    //! Last Played system internal name
    static constexpr const char* sLastPlayedSystemShortName = "lastplayed";
    //! All games system internal name
    static constexpr const char* sAllGamesSystemShortName = "allgames";
    //! Tate games system internal name
    static constexpr const char* sTateSystemShortName = "tate";
    //! Ports system internal name
    static constexpr const char* sPortsSystemShortName = "ports";
    //! Ports system internal name
    static constexpr const char* sLightgunSystemShortName = "lightgun";
    //! Ports system internal name
    static constexpr const char* sArcadeSystemShortName = "arcade";

    //! Multiplayer system user-friendly name
    static constexpr const char* sMultiplayerSystemFullName = "Multi Players";
    //! Last Played system user-friendly name
    static constexpr const char* sLastPlayedSystemFullName = "Last Played";
    //! All games system user-friendly name
    static constexpr const char* sAllGamesSystemFullName = "All Games";
    //! Tate games system user-friendly name
    static constexpr const char* sTateSystemFullName = "Tate";
    //! Ports system user-friendly name
    static constexpr const char* sPortsSystemFullName = "Ports";
    //! Ports system user-friendly name
    static constexpr const char* sLightgunSystemFullName = "LightGun Games";
    //! Ports system internal name
    static constexpr const char* sArcadeSystemFullName = "Arcade";

  private:
    //! Rom source folder to read/write (false) / read-only (true) state
    typedef HashMap<String, bool> RomSources;
    //! System's raw rom path to extension list
    typedef HashMap<String, String> SystemRootPathToExtensions;

    //! File path to system weight file for fast loading/saving
    static constexpr const char* sWeightFilePath = "/recalbox/share/system/.emulationstation/.weights";

    //! Root tag
    static constexpr const char* sRootTag = "%ROOT%";

    //! Writable share roms
    static constexpr const char* sShareRomRoot = "/recalbox/share/roms";
    //! Read-only share roms
    static constexpr const char* sShareInitRomRoot = "/recalbox/share_init/roms";
    //! Update share roms
    static constexpr const char* sShareUpgradeRomRoot = "/recalbox/share_upgrade/roms";

    //! Special process of ports
    enum class PortTypes
    {
        None,          //!< Not a port / Do nothing
        ShareInitOnly, //!< From share_init only
        ShareOnly,     //!< From share only
    };

    enum class RomStructure
    {
      None,   //!< No rom structure at all
      Empty,  //!< Rom structure exists but empty
      Filled, //!< Rom structure with roms inside
    };

    //! Mount points
    Path::PathList mMountPoints;
    //! Mount point monitoring
    MountMonitor mMountPointMonitoring;
    //! Emulator manager guard
    Mutex mEmulatorGuard;

    //! Hasher
    SystemHasher mHasher;

    //! Visible system, including virtual system (Arcade)
    List mVisibleSystems;
    //! ALL systems, visible and hidden
    List mAllSystems;
    //! Original of all system (copy)
    List mOriginalOrderedSystems;

    //! System's raw root rom path to extensions
    SystemRootPathToExtensions mSystemNameToSystemRootPath;

    //! Fast search cache
    std::vector<FolderData::FastSearchItemSerie> mFastSearchSeries;
    //! Fast search cache hash
    unsigned int mFastSearchCacheHash;

    //! Progress interface called when loading/unloading
    IProgressInterface* mProgressInterface;
    //! System loading phase interface
    ISystemLoadingPhase* mLoadingPhaseInterface;
    //! Rom path change notifications interface
    IRomFolderChangeNotification& mRomFolderChangeNotificationInterface;
    //! Interface for system changes
    ISystemChangeNotifier* mSystemChangeNotifier;

    HashSet<String>& mWatcherIgnoredFiles;

    //! The system manager is instructed to reload game list from disk, not only from gamelist.xml
    bool mForceReload;

    /*!
     * @brief Check if there are at least one file from the given path whose extension is in the given set
     * @param path Path (folder) to check files in
     * @param extensionSet Extension list to check files against
     * @return True if a file with a matchng extension has been found, false otherwise
     */
    static bool HasFileWithExt(const Path& path, HashSet<String>& extensionSet);

    /*!
     * @brief Check if the root folder contains at least a valid rom path and return it
     * @param root Root mount point
     * @param romPath Output rom path found
     * @return True if a rom path has been found and stored in romPath. False otherwise
     */
    RomStructure CheckMountPoint(const DeviceMount& root, Path& romPath);

    /*!
     * @brief Initialize initial mount points
     */
    void InitializeMountPoints();

    /*!
     * @brief Get valid rom source for the given system descriptor
     * @param systemDescriptor System descriptor
     * @return Rom source folders and associated RW/RO states
     */
    RomSources GetRomSource(const SystemDescriptor& systemDescriptor, PortTypes port);

    /*!
     * @brief Create regular system from a SystemDescriptor object
     * @param systemDescriptor SystemDescriptor object
     * @return New system
     */
    SystemData* CreateRegularSystem(const SystemDescriptor& systemDescriptor);

    /*!
     * @brief Create Favorite system
     * @return New Favorite system
     */
    SystemData* CreateFavoriteSystem();

    /*!
     * @brief Create Ports system
     * @return New Ports system
     */
    SystemData* CreatePortsSystem();

    /*!
     * @brief Create Last played system
     * @return New Last played system
     */
    SystemData* CreateLastPlayedSystem();

    /*!
     * @brief Create Multi-player system
     * @return New Multi-player system
     */
    SystemData* CreateMultiPlayerSystem();

    /*!
     * @brief Create All-games system
     * @return New All-games system
     */
    SystemData* CreateAllGamesSystem();

    /*!
     * @brief Create Light-gun system
     * @return New Light-gun system
     */
    SystemData* CreateLightgunSystem();

    /*!
     * @brief Create Tate system
     * @return New Tate system
     */
    SystemData* CreateTateSystem();

    /*!
     * @brief Create Tate system
     * @return New Tate system
     */
    SystemData* CreateArcadeSystem();

    /*!
     * @brief Create Genre system
     * @return New Genre system
     */
    SystemData* CreateGenreSystem(GameGenres genre);

    /*!
     * @brief Create Arcade Manufacturers system
     * @return New Arcade Manufacturers system
     */
    SystemData* CreateArcadeManufacturersSystem(const String& manufacturer);

    /*!
     * @brief Manager hiden ports' sub-systems
     */
    void ManagePortsVirtualSystem();

    /*!
     * @brief Initialize system - Populate, then call either InitializeVirtualSystem or InitializeRegularSystem
     * Calling this method multiple times has no effect
     * @param system System to initialize
     * @param initializeOnly True to initialize the system without populating it first. False to populate and initialize
     */
    void InitializeSystem(SystemData* system);

    /*!
     * @brief Top level virtual system populate - load theme, then set initialized
     * Calling this method multiple times has no effect
     * @param system System to initialize
     */
    void PopulateVirtualSystem(SystemData* system);

    /*!
     * @brief Initialize system - load theme, call other modifier/initializers, then set initialized
     * Calling this method multiple times has no effect
     * @param system System to initialize
     */
    void PopulateRegularSystem(SystemData* system);

    /*!
     * @brief Populate favorite system
     * @param systemFavorite Favorite system
     */
    void PopulateFavoriteSystem(SystemData* systemFavorite);

    /*!
     * @brief Populate ports system
     * @param systemPorts Ports system
     */
    void PopulatePortsSystem(SystemData* systemPorts);

    /*!
     * @brief Populate last played system
     * @param systemPorts last played system
     */
    void PopulateLastPlayedSystem(SystemData* systemLastPlayed);

    /*!
     * @brief Populate Multi-player system
     * @param systemPorts Multi-player system
     */
    void PopulateMultiPlayerSystem(SystemData* systemMultiPlayer);

    /*!
     * @brief Populate All games system
     * @param systemPorts All games system
     */
    void PopulateAllGamesSystem(SystemData* systemAllGames);

    /*!
     * @brief Populate Light-gun system
     * @param systemPorts Light-gun system
     */
    void PopulateLightgunSystem(SystemData* systemLightGun);

    /*!
     * @brief Populate Tate system
     * @param systemPorts Tate system
     */
    void PopulateTateSystem(SystemData* systemTate);

    /*!
     * @brief Populate Tate system
     * @param systemPorts Tate system
     */
    void PopulateArcadeSystem(SystemData* systemArcade);

    /*!
     * @brief Populate Genre system
     * @param systemPorts Genre system
     */
    void PopulateGenreSystem(SystemData* systemArcade);

    /*!
     * @brief Populate Arcade Manufacturers system
     * @param systemPorts Arcade Manufacturers system
     */
    void PopulateArcadeManufacturersSystem(SystemData* systemArcade);

    /*!
     * @brief Generic method to pupulate a virtual system using contents from a list of regular systems
     * @param target system to populate
     * @param systems System list to populate the target system with
     * @param doppelganger Pre-build doppelganger
     * @param includesubfolder True to include subfolder, false to include only top level
     */
    static void PopulateVirtualSystemWithSystem(SystemData* system, const List & systems, FileData::StringMap& doppelganger, bool includesubfolder);

    /*!
     * @brief Generic method to pupulate a virtual system using contents from a list of games
     * @param target system to populate
     * @param games Game list to populate the target system with
     * @param doppelganger Pre-build doppelganger
     */
    static void PopulateVirtualSystemWithGames(SystemData* system, const FileData::List& games, FileData::StringMap& doppelganger);

    /*!
     * @brief Populate meta system with a filtered game list (from all regular systems)
     * @param system System to fill with filter results
     * @param filter Filter interface
     * @param comparer Optional comparer - if not null, filter results are sorted using this filter
     * @param arcadeOnly if True, only true arcade systems are filtered
     */
    void PopulateMetaSystemWithFilter(SystemData* system, IFilter* filter, FileData::Comparer comparer);

    /*!
     * @brief Ensure the given system is in the visible list (== initialized with games)
     * @param system System to make visible
     */
    void MakeSystemVisible(SystemData* system);

    /*!
     * @brief Make the given system invisible
     * @param system System to invisibilise
     */
    void MakeSystemInvisible(SystemData* system);

    /*
     * ThreadPoolWorkingInterface implementation
     */

    /*!
     * @brief Load and parse a single gamelist, then return a complete fulfilled system
     * @param systemDescriptor System object from es_systems.cfg
     * @return New SystemData object or nullptr
     */
    SystemData* ThreadPoolRunJob(SystemDescriptor& systemDescriptor) override;

    /*!
     * @brief Update a single gamelist from the metadata
     * @param feed System from which to update the gamelist
     * @return Always true
     */
    bool ThreadPoolRunJob(SystemData*& feed) override;

    /*!
     * @brief Create virtual systems
     * @param feed Virtual system descriptor
     * @return Always true
     */
    VirtualSystemResult ThreadPoolRunJob(VirtualSystemDescriptor& virtualDescriptor) override;

    /*!
     * @brief Thread pool tick to give the oportunity to display progression
     * @param completed Completed pending jobs
     * @param total Total jobs
     */
    void ThreadPoolTick(int completed, int total) override;

    /*
     * IMountMonitorNotifications implementation
     */

    /*!
     * @brief Notify a new device partition has been mounted
     * @param root Mount Point
     */
    void NotifyDeviceMount(const DeviceMount& root) override;

    /*!
     * @brief Notify a new device partition has been unmounted
     * @param root Mount Point
     */
    void NotifyDeviceUnmount(const DeviceMount& root) override;

    /*
     * Fast search cache management
     */

    /*!
     * @brief Create fast search caches
     * @param resultIndexes Index to get context from
     * @param searchableSystems Searchable systems
     */
    void CreateFastSearchCache(const MetadataStringHolder::FoundTextList& resultIndexes, const Array<const SystemData*>& searchableSystems);

    //! Remove all cache
    void DeleteFastSearchCache();

    /*!
     * @brief Called when a game has been deleted
     * This method update internal systems and provide the caller with a liste of added or removed systems
     * @param target Game whose metadata have been modified or game deleted. May be nullptr is more than one game changed
     * @param removedSystems Output: Removed systems. May be empty
     * @param modifiedSystems Output: Modified systems. May be empty
     * @return True if there has been any change, false otherwise
     */
    bool UpdateSystemsOnGameDeletion(FileData* target, List& removedSystems, List& modifiedSystems);

    /*!
     * @brief Called when a game has been deleted
     * This method update internal systems and provide the caller with a liste of added or removed systems
     * @param changes Metadata changes
     * @param addedSystems Output: Added systems. May be empty
     * @param removedSystems Output: Removed systems. May be empty
     * @param modifiedSystems Output: Modified systems. May be empty
     * @return True if there has been any change, false otherwise
     */
    bool UpdateSystemsOnMultipleGameChanges(MetadataType changes, List& addedSystems, List& removedSystems, List& modifiedSystems);

    /*!
     * @brief Called when a game has been deleted
     * This method update internal systems and provide the caller with a liste of added or removed systems
     * @param target Game whose metadata have been modified or game deleted. May be nullptr is more than one game changed
     * @param changes Metadata changes
     * @param addedSystems Output: Added systems. May be empty
     * @param removedSystems Output: Removed systems. May be empty
     * @param modifiedSystems Output: Modified systems. May be empty
     * @return True if there has been any change, false otherwise
     */
    bool UpdateSystemsOnSingleGameChanges(FileData* target, MetadataType changes, List& addedSystems, List& removedSystems, List& modifiedSystems);

    /*!
     * @brief Check if the given game should belong to the given virtual system, regarding its metadata
     * @param game Game to check
     * @param system Target virtual system
     * @return True of the game has metadata that make it belonging to the target virtual system. False otherwise
     */
    static bool ShouldGameBelongToThisVirtualSystem(const FileData* game, const SystemData* system);

    /*!
     * @brief Notify system changes via the ISystemChangeNotifier interface
     * @param addedSystems Added systems or nullptr
     * @param removedSystems Removed system or nullptr
     * @param modifiedSystems Modified system or nullptr
     * @param autoSelectMonoSystem If the list contains only one system, tell the GUI to move onto this system)
     */
    void ApplySystemChanges(List* addedSystems, List* removedSystems, List* modifiedSystems, bool autoSelectMonoSystem);

    /*!
     * @brief Check the given list, looking for uninitialized systems
     * @param list List to check
     * @return True if at least one systm is not initialized, false if they are all initialized
     */
    static bool ContainsUnitializedSystem(const List& list);

    /*!
     * @brief Check the given list and remove system that must stay hidden
     * @param list List to filter
     */
    static void RemoveAlwaysHiddenSystems(List& list);

    /*
     * Log facilities
     */

    static void LogSystemAdded(SystemData* system) { LOG(LogInfo) << "[SystemManager] System " << system->FullName() << " becomes visible."; }
    static void LogSystemRemoved(SystemData* system) { LOG(LogInfo) << "[SystemManager] System " << system->FullName() << " becomes INvisible."; }
    static void LogSystemGameAdded(SystemData* system, FileData* game) { LOG(LogWarning) << "[SystemManager] Metadata changed. Add " << game->Name() << " into " << system->FullName(); }
    static void LogSystemGameRemoved(SystemData* system, FileData* game) { LOG(LogWarning) << "[SystemManager] Metadata changed. Remove " << game->Name() << " from " << system->FullName(); }

    /*
     * ISlowSystemOperation implementation
     */

    //! Populate operation
    void SlowPopulateExecute(const List& listToPopulate) override;

    //! Completed
    void SlowPopulateCompleted(const List& listToPopulate, bool autoSelectMonoSystem) override;

  public:
    /*!
     * @brief constructor
     */
    explicit SystemManager(IRomFolderChangeNotification& interface, HashSet<String>& watcherIgnoredFiles)
      : mMountPointMonitoring(this)
      , mFastSearchCacheHash(0)
      , mProgressInterface(nullptr)
      , mLoadingPhaseInterface(nullptr)
      , mRomFolderChangeNotificationInterface(interface)
      , mSystemChangeNotifier(nullptr)
      , mWatcherIgnoredFiles(watcherIgnoredFiles)
      , mForceReload(false)
    {
      MetadataDescriptor::InitializeDefaultMetadata();
    }

    //! Destructor
    ~SystemManager() override = default;

    //! Access Mount monitor
    MountMonitor& GetMountMonitor() { return mMountPointMonitoring; }

    /*!
     * @brief Set the progress interface
     * @param interface interface referenceinterface reference
     */
    void SetProgressInterface(IProgressInterface* interface) { mProgressInterface = interface; }

    /*!
     * @brief Set the progress interface
     * @param interface interface reference
     */
    void SetLoadingPhaseInterface(ISystemLoadingPhase* interface) { mLoadingPhaseInterface = interface; }

    /*!
     * @brief Set System change interface
     * @param interface interface reference
     */
    void SetChangeNotifierInterface(ISystemChangeNotifier* interface) { mSystemChangeNotifier = interface; }

    /*!
     * @brief Notify loading phase to the interface, if ona has been set
     * @param phase Loading phase
     */
    void NotifyLoadingPhase(ISystemLoadingPhase::Phase phase) const { if (mLoadingPhaseInterface !=  nullptr) mLoadingPhaseInterface->SystemLoadingPhase(phase); }

    /*!
     * @brief Get favorite system
     * @return Favorite system of nullptr if there is no favorite system
     */
    SystemData* FavoriteSystem();

    /*!
     * @brief Get system by (short) name
     * @param name Short name
     * @return System instance of nullptr if not found
     */
    SystemData* SystemByName(const String& name);

    /*!
     * @brief Lookup virtual system by type
     * @param type Virtual system type
     * @return System reference - never null
     */
    SystemData* VirtualSystemByType(VirtualSystemType type);

    /*!
     * @brief Lookup arcade manufacturer virtual system by name
     * @param name Short name w/o prefix
     * @return System reference - never null
     */
    SystemData* VirtualArcadeManufacturerSystemByName(const String& name);

    /*!
     * @brief Lookup genre virtual system by genre enumeration
     * * @param genre Genre type
     * @return System reference - never null
     */
    SystemData* VirtualGenreSystemByGenre(GameGenres genre);

    /*!
     * @brief Get the first non-empty system
     * @return First non empty system or null if all systems are empty
     */
    SystemData* FirstNonEmptySystem();

    /*!
     * @brief Get visible system index by name
     * @param name System name
     * @return System index or -1 if not found
     */
    int getVisibleSystemIndex(const String& name);

    /*!
     * @brief Update gamelist that contain modified game metadata
     */
    void UpdateAllGameLists();

    /*!
     * @brief Delete all systems and all sub-objects
     * @param updateGamelists
     */
    void DeleteAllSystems(bool updateGamelists);

    /*!
     * @brief Load the system config file at getConfigPath(). Returns true if no errors were encountered. An example will be written if the file doesn't exist.
     * @param gamelistWatcher FileNotifier to fill in with gamelist path
     * @param ForeReload force reloading from disk
     * @param portableSystem true if the current board is a portable system and does not need lightgun
     */
    bool LoadSystemConfigurations(FileNotifier& gamelistWatcher, bool ForeReload, bool portableSystem);

    /*!
     * @brief Load a single system to get mgame metadata in autorun mode
     * @param UUID UUID of system to load
     */
    bool LoadSingleSystemConfigurations(const String& UUID);

    /*!
     * @brief Load akk systems from the descriptor list
     * @param systemList System to load
     * @param gamelistWatcher FileNotifier to fill in with gamelist path
     * @param portableSystem true if the current board is a portable system and does not need lightgun
     * @param novirtuals True to bypass virtual system loading (only useful for fast system loading in autorun mode)
     */
    bool LoadSystems(const DescriptorList& systemList, FileNotifier* gamelistWatcher, bool portableSystem, bool novirtuals);

    /*!
     * @brief Build all virtual systems
     * @param systemList true system list to load/reload
     * @param portableSystem true if the current board is a portable system and does not need lightgun
     */
    void LoadVirtualSystems(const DescriptorList& systemList, bool portableSystem);

    /*!
     * @brief Check if a Virtual system identified by the given type needs to be refreshed according to the given list
     * of loaded/reloaded systems
     * @param systemList System list
     * @param type Virtual system type
     * @return True if the given Virtual system needs to be refreshed, false otherwise
     */
    [[nodiscard]] bool VirtualSystemNeedRefresh(const DescriptorList& systemList, VirtualSystemType type) const;

    /*!
     * @brief Set file watching on all gamelist so that the frontend may know if they have been modified from elsewhere
     * @param gamelistWatcher file notifier instance
     */
    void WatchGameList(FileNotifier& gamelistWatcher);

    /*!
     * @brief Get total games
     * @return Total games
     */
    [[nodiscard]] int GameCount() const;

    /*!
     * @brief Get regular (non-virtual) system count from visible list
     * @return regular system count
     */
    [[nodiscard]] int GetVisibleRegularSystemCount() const;

    /*!
     * @brief Get all system, including EMPTY systems
     * @return all system list
     */
    [[nodiscard]] const List& AllSystems() const { return mAllSystems; }

    /*!
     * @brief Get visible-only system list
     * @return System list
     */
    [[nodiscard]] const List& VisibleSystemList() const { return mVisibleSystems; }

    /*!
     * @brief Get next system to the given system
     * @param to Reference system
     * @return Next system
     */
    SystemData* NextVisible(SystemData* to) const
    {
      int index = mVisibleSystems.IndexOf(to);
      if (index < 0) return mVisibleSystems[0];
      return mVisibleSystems[(++index) % mVisibleSystems.Count()];
    }

    /*!
     * @brief Get previous system to the given system
     * @param to Reference system
     * @return Previous system
     */
    SystemData* PreviousVisible(SystemData* to) const
    {
      int index = mVisibleSystems.IndexOf(to);
      if (index < 0) return mVisibleSystems[0];
      return mVisibleSystems[(--index + mVisibleSystems.Count()) % mVisibleSystems.Count()];
    }

    /*!
     * @brief Search games from text
     * @param text Text to search for
     * @param maxglobal Maximum results
     * @return Sorted game found list
     */
    FileData::List SearchTextInGames(FolderData::FastSearchContext context, const String& text, int maxglobal, const SystemData* targetSystem);

    /*!
     * @brief Autoscrape system with game in png
     * @param system System to scrape
     */
    static void CheckAutoScraping(SystemData& system);

    /*!
     * @brief Override folder informations?
     * @param system System to check folders from
     */
    static void CheckFolderOverriding(SystemData& system);

    /*!
     * @brief Build dynamic metadata for the given system
     * @param system System for which to build dynamic metadata
     */
    static void BuildDynamicMetadata(SystemData& system);

    //! Sort or resort system list
    void SystemSorting();

    /*!
     * @brief Lookup a game by filepath in the whole system list
     * @param filePath file path to lookup
     * @return FileData or nullptr if no game is found
     */
    FileData* LookupGameByFilePath(const String& filePath);

    /*!
     * @brief Create an empty rom structure in
     * @param device Source device
     * @return True if th  path have been created successfully, false otherwise
     */
    static bool CreateRomFoldersIn(const DeviceMount& device);

    void AddWatcherIgnoredFiles(const String& path) { mWatcherIgnoredFiles.insert(path); }

    /*!
     * @brief Get an existing system or create it if it does not exists!
     * Do work only on "normal" systems.
     * If the system is created, it's also added to the visible list at position zero
     * @param descriptor System descriptor
     * @return existting system or newly created system
     */
    SystemData& GetOrCreateSystem(const SystemDescriptor& descriptor);

    /*!
     * @brief Called when something moved, either in games metadata or in gamelist (game removed)
     * This method update internal systems and provide the caller with a liste of added or removed systems
     * @param target Game whose metadata have been modified or game deleted. May be nullptr is more than one game changed
     * @param changes Metadata changes
     * @param deleted True if the game has been deleted
     */
    void UpdateSystemsOnGameChange(FileData* target, MetadataType changes, bool deleted);

    /*!
     * @brief Top level filter component has been updated
     * All systems must check if they are becoming visible or invisible or just updated
     */
    [[nodiscard]] bool UpdatedTopLevelFilter();

    /*!
     * @brief Show or Hide the given system.Initialize the given system ir required, then make is visible!
     * This method is a high level method that make the move in/out the Visible list, initialize the system if required
     * and call the SystemNotifier
     * @param system System to change visibility
     * @param show Tru to show the system, false to hide
     */
    void UpdateSystemsVisibility(SystemData* system, Visibility visibility);

    /*!
     * @brief Show or Hide the given virtual system. Initialize the given system ir required, then make is visible!
     * This method is a high level method that make the move in/out the Visible list, initialize the system if required
     * and call the SystemNotifier
     * @param type Virtual system to change visibility
     * @param show Tru to show the system, false to hide
     */
    void UpdateVirtualSystemsVisibility(VirtualSystemType type, Visibility visibility)
    {
      UpdateSystemsVisibility(VirtualSystemByType(type), visibility);
    }

    /*!
     * @brief Show or Hide the given arcade manufacturer virtual system. Initialize the given system ir required, then make is visible!
     * This method is a high level method that make the move in/out the Visible list, initialize the system if required
     * and call the SystemNotifier
     * @param name arcade manufacturer virtual system name to change visibility
     * @param show Tru to show the system, false to hide
     */
    void UpdateVirtualArcadeManufacturerSystemsVisibility(const String& name, Visibility visibility)
    {
      UpdateSystemsVisibility(VirtualArcadeManufacturerSystemByName(name), visibility);
    }

    /*!
     * @brief Show or Hide the given genre virtual system. Initialize the given system ir required, then make is visible!
     * This method is a high level method that make the move in/out the Visible list, initialize the system if required
     * and call the SystemNotifier
     * @param genre Virtual genre system to change visibility
     * @param show Tru to show the system, false to hide
     */
    void UpdateVirtualGenreSystemsVisibility(GameGenres genre, Visibility visibility)
    {
      UpdateSystemsVisibility(VirtualGenreSystemByGenre(genre), visibility);
    }

    /*!
     * @brief Manager hiden/shown arcade system, regarding arcade virtual system configuration
     * @param startup If true, the method make required system visible/invisible without calling
     * the SystemInterfaceNotifier on added/removed systems from the visible list
     */
    void ManageArcadeVirtualSystem(bool startup = false);

    /*!
     * @brief Get system name from the given arcade manufacturer name
     * @param manufacturer Manufacturer name
     * @return System name
     */
    static String BuildArcadeManufacturerSystemName(const String& manufacturer)
    {
      return String(sArcadeManufacturerPrefix).Append(manufacturer).Replace('\\', '-');
    }

    /*!
     * @brief Get system name from the given genre
     * @param genre Game genre
     * @return System name
     */
    static String BuildGenreSystemName(GameGenres genre)
    {
      return String(sGenrePrefix).Append(Genres::GetShortName(genre));
    }

    /*!
     * @brief Get absolute system index from the all system list
     * this index must not change whether there are shown/hidden/loaded/unloaded systems before or after
     * @param system System to get index from
     * @return System index or -1
     */
    int SystemAbsoluteIndex(SystemData* const system)
    {
      return mAllSystems.IndexOf(system);
    }
};
