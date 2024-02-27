#pragma once

#include <views/GameClipView.h>
#include "views/gamelist/ISimpleGameListView.h"
#include "views/SystemView.h"
#include "SplashView.h"
#include "views/crt/CrtView.h"
#include "guis/menus/IFastMenuListCallback.h"
#include "ISaveStateSlotNotifier.h"
#include "ISoftPatchingNotifier.h"
#include "views/gamelist/DetailedGameListView.h"
#include "guis/GuiWaitLongExecution.h"
#include <emulators/run/GameLinkedData.h>

class SystemData;

struct DelayedSystemOperationData
{
  SystemManager::List mSystemList;
  ISlowSystemOperation* mSlowIMethodInterface;
  bool autoSelectMonoSystem;
};

// Used to smoothly transition the camera between multiple views (e.g. from system to system, from gamelist to gamelist).
class ViewController : public StaticLifeCycleControler<ViewController>
                     , private INoCopy
                     , public Gui
                     , public ISystemChangeNotifier
                     , public IFastMenuListCallback
                     , public ISaveStateSlotNotifier
                     , public ISoftPatchingNotifier
                     , private Thread
                     , public ISyncMessageReceiver<SlowDataInformation>
                     , public ILongExecution<DelayedSystemOperationData, bool>
{
  public:
    //! Flags used in launch method to check what option is already selected
    enum class LaunchCheckFlags
    {
      None             = 0x00, //!< Nothing
      Bios             = 0x01, //!< Bios checked
      CrtResolution    = 0x02, //!< Crt Resolution selected
      Frequency        = 0x04, //!< Frequency selected
      SoftPatching     = 0x10, //!< Soft patching selected
      SuperGameboy     = 0x20, //!< Supergameboy selected
      SaveState        = 0x40, //!< Savestate selected
    };

    //! View type
    enum class ViewType
    {
      None,           //!< Unitialized
      SplashScreen,   //!< Splash screen (startup or stop)
      SystemList,     //!< System list
      GameList,       //!< Game list
      GameClip,       //!< Game clip
      CrtCalibration, //!< CRT Calibration screen
    };

    /*!
     * @brief Constructor
     * @param window Window manager
     * @param systemManager System manager
     */
  	ViewController(WindowManager& window, SystemManager& systemManager);
    //! Destructor
    ~ViewController() override;

    /*!
     * @brief Wake up the system if it is in a sleeping state
     */
    void WakeUp() { mWindow.DoWake(); }

    /*!
     * @brief Check bios and call LaunchAnimated
     * @param game game to launch
     * @param netplay optional netplay data
     * @param centerCameraOn optional camera target point
     * @param forceGoToGame Force gamelist & game display after the game end. This is only usefull when a game is not run from its gamelist
     */
    void Launch(FileData* game, const GameLinkedData& netplay, const Vector3f& centerCameraOn, bool forceGoToGame);

    bool GetOrReCreateGamelistView(SystemData* view, bool reloadTheme = false);
    void InvalidateGamelist(const SystemData* system);
    void InvalidateAllGamelistsExcept(const SystemData* systemExclude);

    // Navigation.
    SystemData* goToNextGameList();
    void goToPrevGameList();
    void goToGameList(SystemData* system);
    void goToSystemView(SystemData* system);
    void goToGameClipView();
    void goToCrtView(CrtView::CalibrationType screenType);
    void selectGamelistAndCursor(FileData* file);
    void goToStart();
    void goToQuitScreen();

    /*!
     * @brief Set the previous vue back - Only once!
     */
    void BackToPreviousView();

    [[nodiscard]] inline bool isViewing(ViewType viewing) const { return mCurrentViewType == viewing; }

    bool getHelpPrompts(Help& help) override;
    void ApplyHelpStyle() override;

    ISimpleGameListView* GetOrCreateGamelistView(SystemData* system);
    SystemView& getSystemListView() { return mSystemListView; }

    [[nodiscard]] Gui& CurrentUi() const { return *mCurrentView; }

    void ToggleFavorite(FileData* game, bool forceStatus = false, bool forcedStatus = false);

    /*!
     * @brief Get the progress interface
     * @return Progress interface
     */
    IProgressInterface& GetProgressInterface() { return mSplashView; }

    bool CheckFilters();

    [[nodiscard]] ViewType CurrentView() const { return mCurrentViewType; }

    //! Get current system
    [[nodiscard]] SystemData* CurrentSystem() const { assert(mCurrentViewType == ViewType::GameList || mCurrentViewType == ViewType::SystemList); return mCurrentSystem; }

    /*
     * Gui implementation
     */

    [[nodiscard]] bool DoNotDisturb() const override
    {
      switch(mCurrentViewType)
      {
        case ViewType::SplashScreen: return true;
        case ViewType::SystemList: return mSystemListView.DoNotDisturb();
        case ViewType::GameList: return mCurrentView->DoNotDisturb();
        case ViewType::GameClip: return false;
        case ViewType::CrtCalibration: return true;
        case ViewType::None:
        default: break;
      }
      return false;
    }

    /*
     * ISystemChangeNotifier
     */

    //! System must show
    void ShowSystem(SystemData* system) override;

    //! System must hide
    void HideSystem(SystemData* system) override;

    //! System must be updated (games have been updated inside)
    void UpdateSystem(SystemData* system) override;

    //! System should be selected
    void SelectSystem(SystemData* system) override;

    //! Request threaded operations
    void RequestSlowOperation(ISlowSystemOperation* interface, ISlowSystemOperation::List systems, bool autoSelectMonoSystem) override;

    //! User notification
    void SystemShownWithNoGames(SystemData* system) override;

    /*
     * Component override
     */

    bool ProcessInput(const InputCompactEvent& event) override;
    void Update(int deltaTime) override;
    void Render(const Transform4x4f& parentTrans) override;

    void FetchSlowDataFor(FileData* data);

    /*
     * ILongExecution implementation
     */

    bool Execute(GuiWaitLongExecution<DelayedSystemOperationData, bool>& from,
                 const DelayedSystemOperationData& parameter) override;

    void Completed(const DelayedSystemOperationData& parameter, const bool& result) override;

  private:
    //! Fast menu types
    enum class FastMenuType
    {
      Frequencies,        //!< Frequency choice
      FrequenciesMulti60, //!< Frequency choice w/ multiple 60hz
      CrtResolution,      //!< CRT resolution choice
      SuperGameboy,       //!< Supergameboy choice
    };

    //! Game linked data internal instance
    GameLinkedData mGameLinkedData;
    //! Game to launch
    FileData* mGameToLaunch;
    //! Camera target for launch
    Vector3f mLaunchCameraTarget;
    //! Check flags
    LaunchCheckFlags mCheckFlags;
    //! Force go to game after the game ends
    bool mForceGoToGame;

    //! SystemManager instance
    SystemManager& mSystemManager;

    //! Current view reference
    Gui* mCurrentView;
    // Current system for views dealing with systems (System list/Game list)
    SystemData* mCurrentSystem;

    HashMap<SystemData*, ISimpleGameListView*> mGameListViews;
    SystemView mSystemListView;
    SplashView mSplashView;
    GameClipView mGameClipView;
    CrtView mCrtView;
    HashMap<SystemData*, bool> mInvalidGameList;

    ViewType mCurrentViewType;  //!< Current view type
    ViewType mPreviousViewType; //!< Previous view type


    Transform4x4f mCamera;
    float mFadeOpacity;
    bool mLockInput;

    //! Last launch time
    DateTime mLastGameLaunched;

    //! Keep choice of frequency (megadrive multi-60)
    int mFrequencyLastChoiceMulti60;
    //! Keep choice of frequency (all systems, except megadrive)
    int mFrequencyLastChoice;
    //! Keep choice of CRT resolution
    int mResolutionLastChoice;
    //! Keep choice of Supergameboy launch
    int mSuperGameboyLastChoice;
    //! Keep choice of Soft patching
    int mSoftPatchingLastChoice;

    //! Item information synchronizer
    SyncMessageSender<SlowDataInformation> mSender;
    //! Next item to fetch slow data from
    FileData* mNextItem;
    //! Last slow data extracted: folder images' path
    FolderImagesPath mLastFolderImagePath;
    //! Locker
    Mutex mLocker;
    //! Fetch info thread signal
    Signal mSignal;

    /*!
     * @brief  Check if softpatching is required and let the user select
     * @param emulator Emulator data
     * @result True if softpatching has been selected, false otherwise
     */
    bool CheckSoftPatching(const EmulatorData& emulator);

    /*!
     * @brief Change the current view and store the previous
     * @param newViewMode New view mode
     * @param targetSystem Target system for view requiring a system
     */
    void ChangeView(ViewType newViewMode, SystemData* targetSystem);

    /*!
     * @brief Check bios and call LaunchAnimated
     */
    void LaunchCheck();

    /*!
     * @brief Run animation and call LaunchActually
     * @param game game to launch
     * @param netplay optional netplay data
     * @param centerCameraOn optional camera target point
     */
    void LaunchAnimated(const EmulatorData& emulator);

    /*!
     * @brief Actually run the game :)
     * @param game game to launch
     * @param netplay optional netplay data
     */
    void LaunchActually(const EmulatorData& emulator);

    /*!
     * @brief Reset game filters
     */
    static void ResetFilters();

    /*!
     * @brief Check if no critical bios if missing for the given game
     * @return True if evrything is ok, false otherwise
     */
    bool CheckBiosBeforeLaunch();

    void playViewTransition();

    /*
     * IFastMenuLineCallback implementation
     */

    /*!
     * @brief Main callback
     * @param menuIndex Menu identifier to identify multiple menu in a single callback
     * @param itemIndex Item index, starting from
     */
    void FastMenuLineSelected(int menuIndex, int itemIndex) override;

    /*
     * ISaveStateSlotNotifier implementation
     */

    /*!
     * @brief Notifie a slot has been selected
     * @param slot Slot number or -1
     */
    void SaveStateSlotSelected(int slot) override;

    /*
     * ISoftPathcingNotifier implementation
     */

    /*!
     * @brief Notify the soft patching is disabled
     */
    void SoftPathingDisabled() override;

    /*!
     * @brief Notify a path has been selected
     * @param path Selected patch's path
     */
    void SoftPatchingSelected(const Path& path) override;

    /*
     * Thread implementation
     */

    //! Break the system info fetching thread
    void Break() override { mSignal.Fire(); }

    //! Implementation of system fetching info
    void Run() override;

    /*
     * ISyncMessageReceiver<const FileData*> implementation
     */

    //! Receive enf-of-fetch info for the very last system the user is on
    void ReceiveSyncMessage(const SlowDataInformation& data) override;
};

DEFINE_BITFLAG_ENUM(ViewController::LaunchCheckFlags, int)