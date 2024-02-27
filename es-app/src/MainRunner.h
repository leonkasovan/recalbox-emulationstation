//
// Created by bkg2k on 13/11/2019.
//
#pragma once

#include <ApplicationWindow.h>
#include <utils/cplusplus/INoCopy.h>
#include <utils/os/fs/watching/IFileSystemWatcherNotification.h>
#include <utils/os/fs/watching/FileNotifier.h>
#include <usernotifications/NotificationManager.h>
#include <guis/GuiWaitLongExecution.h>
#include <emulators/run/GameRunner.h>
#include <patreon/IPatreonNotification.h>
#include "bluetooth/BluetoothListener.h"
#include "recalbox/BootConf.h"
#include <btautopair/BTAutopairManager.h>
#include <bios/IBiosScanReporting.h>

class AudioManager;
class SystemManager;
class BiosManager;

//! Special operations
enum class HardwareTriggeredSpecialOperations
{
  Suspend,  //!< The hardware require a Suspend operation
  Resume,   //!< The hardware just woken up
  PowerOff, //!< The hardware require a Power-off
  Reset,    //!< The hardware require a reset
};

//! USB Initialization state
enum class USBInitializationAction
{
  None,           //! Do nothing
  OnlyRomFolders, //!< Only make roms folder for rom multi-sources
  CompleteShare,  //!< Initialize (copy) the whole current configuration to move to external share
};

class USBInitialization
{
  public:
    //! Default constructor
    USBInitialization(USBInitializationAction action, const DeviceMount* device)
      : mAction(action)
      , mDevice(device)
    {
    }

    //! Default constructor
    USBInitialization()
      : mAction(USBInitializationAction::None)
      , mDevice(nullptr)
    {
    }

    //! Get action
    [[nodiscard]] USBInitializationAction Action() const { return mAction; }

    //! Get device
    [[nodiscard]] const DeviceMount& Device() const { return *mDevice; }

  private:
    //! Action to execute on the device
    USBInitializationAction mAction;
    // Device to initialize
    const DeviceMount* mDevice;
};

class MainRunner
  : private INoCopy
  , private IFileSystemWatcherNotification
  , public IHardwareNotifications
  , private ILongExecution<HardwareTriggeredSpecialOperations, bool>
  , private IRomFolderChangeNotification
  , private IPatreonNotification
  , private ILongExecution<USBInitialization, bool>
  , public ISdl2EventNotifier
  , public ISpecialGlobalAction
  , public IBiosScanReporting
{
  public:
    //! Pending Exit
    enum class PendingExit
    {
       None,            //!< No Pending exit
       GamelistChanged, //!< At least one gamelist has changed. ES must reload
       ThemeChanged,    //!< Current theme has been modified. ES must reload
       MustExit,        //!< External quit required
       WaitingExit,     //!< Special wait state after a pending exit state has been processed
    };

    //! Runner exit state
    enum class ExitState
    {
      Quit,             //!< Normal quit (usually requested by external software)
      FatalError,       //!< Initialization error or runtime fatal error
      Relaunch,         //!< Relaunch requested!
      RelaunchNoUpdate, //!< Relaunch requested! No gamelist update
      NormalReboot,     //!< Normal reboot machine requested, save everything
      FastReboot,       //!< Fast reboot machine, save nothing
      Shutdown,         //!< Relaunch machine
      FastShutdown,     //!< Relaunch machine, save nothing
    };

    //! Temporary file used as flag of readyness
    static constexpr const char* sReadyFile = "/tmp/externalnotifications/emulationstation.ready";
    //! Temporary file used as quit request
    static constexpr const char* sQuitNow = "/tmp/externalnotifications/emulationstation.quitnow";
    //! Temporary file used to stop game demo loop
    static constexpr const char* sStopDemo = "/tmp/externalnotifications/emulationstation.stopdemo";
    //! Upgrade file flag. Only available once in /tmp after a successful update
    static constexpr const char* sUpgradeFileFlag = "/overlay/.upgrade_success";
    static constexpr const char* sUpgradeFailedFlag = "/overlay/.upgrade_failed";
    static constexpr const char* sUpgradeCorruptedFlag = "/overlay/.upgrade_corrupted";

  private:
    //! Power button: Threshold from short to long press, in milisecond
    static constexpr const int sPowerButtonThreshold = 500;

    //! Requested width
    unsigned int mRequestedWidth;
    //! Requested height
    unsigned int mRequestedHeight;
    //! Requested window mode
    bool mRequestWindowed;

    //! Pending exit
    PendingExit mPendingExit;

    //! Run count
    int mRunCount;

    //! Quit request state
    static ExitState sRequestedExitState;
    //! Quit request
    static bool sQuitRequested;
    //! Force reload list requested
    static bool sForceReloadFromDisk;

    //! Inter-thread messaging system
    SyncMessageFactory mSyncMessageFactory;

    //! Recalbox configuration
    RecalboxConf mConfiguration;
    //! Crt configuration
    CrtConf mCrtConfiguration;
    //! Crt configuration
    BootConf mBootConf;

    //! Nofitication manager
    NotificationManager mNotificationManager;

    //! Window reference
    ApplicationWindow* mApplicationWindow;

    //! Bluetooth listener
    BluetoothListener mBluetooth;

    //! Known added devices
    HashSet<String> mAddedDevices;
    //! Known empty added devices
    HashSet<String> mAddedEmptyDevices;
    //! Known removed devices
    HashSet<String> mRemovedDevices;

    //! Ignored files
    HashSet<String> mIgnoredFiles;

    //! Bluetooth Autopair Manager
    BTAutopairManager mBTAutopairManager;

    /*!
     * @brief Reset last exit state
     */
    static void ResetExitState() { sQuitRequested = false; }

    /*!
     * @brief Reset last exit state
     */
    static void ResetForceReloadState() { sForceReloadFromDisk = false; }

    /*!
     * @brief Display Intro
     * @param debug Set debug logs level
     * @param trace Set trace logs level. Takes precedence over debug
     */
    void Intro(bool debug, bool trace);

    /*!
     * @brief Check home folder existence
     */
    static void CheckHomeFolder();

    /*!
     * @brief Play loading jingle (loading.ogg) from theme if available
     */
    static void PlayLoadingSound(AudioManager& audioManager);

    /*!
     * @brief Try loading system configuration.
     * @param systemManager System manager instance
     * @param gamelistWatcher FileNotifier to fill in with gamelist path
     * @param forceReloadFromDisk force reloading game list from disk
     * @return True if there is at least one system loaded
     */
    static bool TryToLoadConfiguredSystems(SystemManager& systemManager, FileNotifier& gamelistWatcher, bool forceReloadFromDisk);

    /*!
     * @brief Check if Recalbox has been updated and push a display changelog popup
     * @param window Main window
     */
    static void CheckUpdateMessage(WindowManager& window);

    /*!
     * @brief Check if Recalbox upgrade has failed push an error popup
     * @param window Main window
    */
    static void CheckUpdateFailed(WindowManager& window);

    /*!
     * @brief Check if Recalbox upgrade is corrupted push an error popup
     * @param window Main window
    */
    static void CheckUpdateCorrupted(WindowManager& window);

    /*!
     * @brief Check if this is the first launch and run a wizard if required
     * @param window Main window
     */
    static void CheckFirstTimeWizard(WindowManager& window);

    /*!
     * @brief Check Recalbox Lite status and show wizard
     */
    static void CheckRecalboxLite(WindowManager& window);

    /*!
     * @brief Initialize input configurations
     * @param window Main window
     */
    static void InitializeUserInterface(WindowManager& window);

    /*!
     * @brief Check if something must retain user's attention
     * @param window Main window
     * @param systemManager System Manager
     */
    static void CheckAlert(WindowManager& window, SystemManager& systemManager);

    /*!
     * @brief Send mqtt message to enable joystick auopairing
     */
    //static void EnableAutopair();

    /*!
     * @brief Main SDL event loop w/ UI update/refresh
     * @param window Main window
     * @param systemManager System Manager
     * @param fileNotifier gamelist file notifier
     * @param syncMessageFactory Syn'ed message factory
     * @return Exit state
     */
    ExitState MainLoop(ApplicationWindow& window, SystemManager& systemManager, FileNotifier& fileNotifier, SyncMessageFactory& syncMessageFactory);

    /*!
     * @brief Process general special inputs
     * @param event Input event
     * @return bool if the input has been consummed, false otherwise
     */
    bool ProcessSpecialInputs(const InputCompactEvent& event);

    /*!
     * @brief Create ready flag file to notify all external software that
     * Emulationstation is ready
     */
    static void CreateReadyFlagFile();
    /*!
     * @brief Cleanup ready flag file
     */
    static void DeleteReadyFlagFile();

    /*!
     * @brief Tell if we have to save the gamelist, regarding the exitstate
     * @param state exit state
     * @return True if we have to update the gamelists before exiting
     */
    static bool DoWeHaveToUpdateGamelist(ExitState state);

    /*
     * IFileSystemWatcherNotification implementation
     */

    void FileSystemWatcherNotification(EventType event, const Path& path, const DateTime& time) final;

    /*
     * IHardwareEvents implementation
     */

    bool IsApplicationRunning() final { return !GameRunner::IsGameRunning(); }

    /*!
     * @brief Headphone has been pluggen in
     * @param board current board
     */
    void HeadphonePluggedIn(BoardType board) final;

    /*!
     * @brief Headphone has been unplugged
     * @param board current board
     */
    void HeadphoneUnplugged(BoardType board) final;

    /*!
     * @brief Decrease volume
     * @param board current board
     */
    void VolumeDecrease(BoardType board, float percent) final;

    /*!
     * @brief Increase volume
     * @param board current board
     */
    void VolumeIncrease(BoardType board, float percent) final;

    /*!
     * @brief Suspend will occur in a short delay
     * @param board current board
     * @param delayInMs delay in ms before suspend will occurs
     */
    void PowerButtonPressed(BoardType board, int delayInMs) final;



    void ResetButtonPressed(BoardType board) final;
    /*!
     * @brief We have been resumed from suspend mode
     * @param board current board
     */
    void Resume(BoardType board) final;

    /*!
     * @brief Decrease brightness
     * @param board current board
     */
    void BrightnessDecrease(BoardType board, float percent) final;

    /*!
     * @brief Increase brightness
     * @param board current board
     */
    void BrightnessIncrease(BoardType board, float percent) final;

    /*!
     * @brief Undervoltage alert
     * @param board current board
     */
    void UnderVoltage(BoardType board) final;

    /*!
     * @brief HighTemperature detected
     * @param board current board
     */
    void TemperatureAlert(BoardType board) final;

    /*
     * ILongExecution implementation
     */

    /*!
     * @brief Dummy execution of sleeps, allowing UI to draw special operation wait-windows
     * @param from Source window
     * @param parameter Operation
     * @return Not used
     */
    bool Execute(GuiWaitLongExecution<HardwareTriggeredSpecialOperations, bool>& from,
                 const HardwareTriggeredSpecialOperations& parameter) override;

    /*!
     * @brief Called when special wait-window close so that we can execute the required operation
     * @param parameter Operation required
     * @param result Not used
     */
    void Completed(const HardwareTriggeredSpecialOperations& parameter, const bool& result) override;

    /*
     * IPatreonNotification implementation
     */

    /*!
     * @brief Notify of the current user status
     * @param result Authentication result (user status)
     * @param level Patron level
     * @param patreonName Patreon name
     */
    void PatreonState(PatronAuthenticationResult result, int level, const String& patreonName) final;

    bool IsFileIgnored(const String& path)
    {
      bool isIgnored = mIgnoredFiles.contains(path);
      if(isIgnored) {
        mIgnoredFiles.erase(path);
      }

      return isIgnored;
    }

    /*
     * ILongExecution<bool, bool> implementation
     */

    /*!
     * @brief Dummy execution of sleeps, allowing UI to draw special operation wait-windows
     * @param from Source window
     * @param parameter Operation
     * @return Not used
     */
    bool Execute(GuiWaitLongExecution<USBInitialization, bool>& from, const USBInitialization& parameter) final;

    /*!
     * @brief Called when special wait-window close so that we can execute the required operation
     * @param parameter Operation required
     * @param result Not used
     */
    void Completed(const USBInitialization& parameter, const bool& result) final;

    /*
     * IBiosScanReporting implementation
     */

    /*!
     * @brief Report a new Bios has been scanned
     * @param bios Newly scanned bios
     */
    void ScanProgress(const Bios& bios) override { (void)bios; }

    /*!
     * @brief Report the bios scan is complete
     */
    void ScanComplete() override;

  public:
    /*!
     * @brief Constructor
     * @param executablePath current executable path
     * @param width Requested width
     * @param height Requested height
     * @param windowed No fullscreen
     * @param runCount Number of time the MainRunner has been run
     * @param environment Application environment
     * @param debug Debug flag
     * @param trace Trace flag
     */
    MainRunner(const String& executablePath, unsigned int width, unsigned int height, bool windowed, int runCount, char** environment, bool debug, bool trace);

    //! Destructor
    ~MainRunner() override;

    /*!
     * @brief Run the game!
     */
    ExitState Run();

    /*!
     * @brief Request the application to quit using a particular exitstate
     * @param requestedState Requested Exit State
     * @param forceReloadFromDisk Force reload gamelist from disk
     */
    static void RequestQuit(ExitState requestedState, bool forceReloadFromDisk = false);

    /*!
     * @brief Set the system locale
     * @param executablePath Path to current executable
     */
    static void SetLocale(const String& executablePath);

    /*!
     * @brief Set debug log state
     * @param debug True to set debug logs on
     * @param trace True to set trace logs on. Trace takes precedence over debug
     */
    static void SetDebugLogs(bool debug, bool trace);

    /*
     * RomFolderChangeNotification implementaton
     */

    /*!
     * @brief Notify a new rompath has been added
     * @param root rompath added
     */
    void RomPathAdded(const DeviceMount& root) override;

    /*!
     * @brief Notify an existing reompath has been removed
     * @param root rompath removed
     */
    void RomPathRemoved(const DeviceMount& root) override;

    /*!
     * @brief Notify a device has been added with no valid rom path found
     * @param deviceRoot Device mount point
     */
    void NoRomPathFound(const DeviceMount& deviceRoot) override;

    /*
     * ISdl2EventNotifier implementation
     */

    /*!
     * @brief Received a raw SDL2 event
     * @param event SDL2 event
     */
    void Sdl2EventReceived(const SDL_Event& event) final;

    /*
     * ISpecialGlobalAction implementation
     */

    /*!
     * @brief Disable OSD
     * @param imagePath OSD image
     * @param x X coordinate in the range of 0.0 (left) ... 1.0 (right)
     * @param y Y coordinate in the range of 0.0 (up) ... 1.0 (bottom)
     * @param width Width in the range of 0.0 (invisible) ... 1.0 (full screen width)
     * @param height Height in the range of 0.0 (invisible) ... 1.0 (full screen height)
     * @param autoCenter if true, x/y are ignored and the image is screen centered
     */
    void EnableOSDImage(const Path& imagePath, float x, float y, float width, float height, float alpha, bool autoCenter) final
    {
      if (mApplicationWindow != nullptr) mApplicationWindow->EnableOSDImage(imagePath, x, y, width, height, alpha, autoCenter);
    }

    /*!
     * @brief Disable OSD
     */
    void DisableOSDImage() final
    {
      if (mApplicationWindow != nullptr) mApplicationWindow->DisableOSDImage();
    }
};
