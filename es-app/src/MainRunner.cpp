//
// Created by bkg2k on 13/11/2019.
//
#include <utils/locale/LocaleHelper.h>
#include <utils/Log.h>
#include <utils/Files.h>
#include <audio/AudioManager.h>
#include <views/ViewController.h>
#include <guis/GuiMsgBoxScroll.h>
#include <VideoEngine.h>
#include <guis/GuiDetectDevice.h>
#include <bios/BiosManager.h>
#include <guis/GuiMsgBox.h>
#include <scraping/ScraperFactory.h>
#include <audio/AudioController.h>
#include <utils/os/system/ProcessTree.h>
#include <recalbox/RecalboxSystem.h>
#include <guis/wizards/WizardAgo2.h>
#include <guis/wizards/WizardAgo3.h>
#include <guis/wizards/WizardRG353X.h>
#include "MainRunner.h"
#include "EmulationStation.h"
#include "Upgrade.h"
#include "CommandThread.h"
#include <netplay/NetPlayThread.h>
#include "DemoMode.h"
#include "RotationManager.h"
#include "RootFolders.h"
#include "web/RestApiServer.h"
#include "guis/wizards/WizardLite.h"
#include <utils/network/DnsClient.h>
#include <music/RemotePlaylist.h>
#include <hardware/devices/storage/StorageDevices.h>
#include <guis/GuiInfoPopup.h>
#include <scraping/ScraperSeamless.h>
#include <sdl2/Sdl2Runner.h>
#include <emulators/run/GameRunner.h>
#include <sdl2/Sdl2Init.h>
#include <patreon/PatronInfo.h>

MainRunner::ExitState MainRunner::sRequestedExitState = MainRunner::ExitState::Quit;
bool MainRunner::sQuitRequested = false;
bool MainRunner::sForceReloadFromDisk = false;

MainRunner::MainRunner(const String& executablePath, unsigned int width, unsigned int height, bool windowed, int runCount, char** environment, bool debug, bool trace)
  : mRequestedWidth(width)
  , mRequestedHeight(height)
  , mRequestWindowed(windowed)
  , mPendingExit(PendingExit::None)
  , mRunCount(runCount)
  , mNotificationManager(environment)
  , mApplicationWindow(nullptr)
  , mBluetooth()
  , mBTAutopairManager()
{
  Intro(debug, trace);
  SetLocale(executablePath);
  CheckHomeFolder();

  // Initialize SDL
  Sdl2Init::Initialize();
}

MainRunner::~MainRunner()
{
  // Finalize SDL
  Sdl2Init::Finalize();
}

MainRunner::ExitState MainRunner::Run()
{
  try
  {
    // Hardware board
    Board board(*this);

    // Set best performance/power CPU governor for battery-powered devices
    board.SetFrontendCPUGovernor();

    // Audio controller
    AudioController audioController;
    audioController.SetVolume(audioController.GetVolume());
    String originalAudioDevice = RecalboxConf::Instance().GetAudioOuput();
    String fixedAudioDevice = audioController.SetDefaultPlayback(originalAudioDevice);
    if (fixedAudioDevice != originalAudioDevice)
      RecalboxConf::Instance().SetAudioOuput(fixedAudioDevice).Save();

    // Notification Manager
    mNotificationManager.Notify(Notification::Start, String(mRunCount));

    // Shut-up joysticks :)
    SDL_JoystickEventState(SDL_DISABLE);

    SystemManager systemManager(*this, mIgnoredFiles);
    GameRunner gameRunner(nullptr, systemManager, *this);
    FileNotifier fileNotifier;
    InputManager inputManager;
    inputManager.Initialize();

    // Autorun?
    if (mRunCount == 0) {
      if (mConfiguration.GetKodiEnabled() && mConfiguration.GetKodiAtStartup())
        gameRunner.RunKodi();
      if (RecalboxConf::Instance().GetAutorunEnabled() && !RecalboxConf::Instance().GetAutorunGamePath().empty())
      {
        systemManager.LoadSingleSystemConfigurations(RecalboxConf::Instance().GetAutorunSystemUUID());
        ResetForceReloadState();
        FileData *game = systemManager.LookupGameByFilePath(RecalboxConf::Instance().GetAutorunGamePath());
        if (game != nullptr && inputManager.ConfiguredControllersCount() > 0)
          gameRunner.RunGame(*game, EmulatorManager::GetGameEmulator(*game), GameLinkedData());
        else
          { LOG(LogInfo) << "[MainRunner] Will not boot on game as game = null or no configured controllers found"; }
      }
    }

    // Initialize the renderer first,'cause many things depend on renderer width/height
    Renderer renderer((int)mRequestedWidth, (int)mRequestedHeight, mRequestWindowed,
                      RotationManager::GetSystemRotation());
    if (!renderer.Initialized()) { LOG(LogError) << "[Renderer] Error initializing the GL renderer."; return ExitState::FatalError; }

    // Initialize main Window and ViewController
    ApplicationWindow window(systemManager);
    if (!window.Initialize(mRequestedWidth, mRequestedHeight, false)) { LOG(LogError) << "[Renderer] Window failed to initialize!"; return ExitState::FatalError; }
    mApplicationWindow = &window;
    gameRunner.SetWindowManager(&window);
    mBluetooth.Register(&window.OSD().GetBluetoothOSD());
    // Brightness
    if (board.HasBrightnessSupport())
      board.SetBrightness(RecalboxConf::Instance().GetBrightness());

    // Initialize audio manager
    AudioManager audioManager(window);

    // Board-related background processes
    // Initialize here so that all global object are available
    board.StartGlobalBackgroundProcesses();

    // Display "loading..." screen
    window.RenderAll();
    PlayLoadingSound(audioManager);

    if (!TryToLoadConfiguredSystems(systemManager, fileNotifier, sForceReloadFromDisk))
      return ExitState::FatalError;
    ResetForceReloadState();

    // Scrapers
    ScraperFactory scraperFactory;

    ExitState exitState = ExitState::Quit;
    try
    {
      // Bios (must be created before the webmanager starts)
      BiosManager biosManager;
      biosManager.LoadFromFile();
      biosManager.Scan(nullptr);

      // Start webserver
      { LOG(LogDebug) << "[MainRunner] Launching Webserver"; }
      RestApiServer webManager(systemManager);

      // Patron Information
      PatronInfo patronInfo(this);
      // Remote music
      RemotePlaylist remotePlaylist;

      // Start update thread
      { LOG(LogDebug) << "[MainRunner] Launching Network thread"; }
      Upgrade networkThread(window);
      // Start the socket server
      { LOG(LogDebug) << "[MainRunner] Launching Command thread"; }
      CommandThread commandThread(systemManager);
      // Start Video engine
      { LOG(LogDebug) << "[MainRunner] Launching Video engine"; }
      VideoEngine videoEngine;
      // Start Netplay thread
      { LOG(LogDebug) << "[MainRunner] Launching Netplay thread"; }
      NetPlayThread netPlayThread(window);

      // Seamless scraper
      ScraperSeamless seamlessScraper;

      // Input ok?
      InitializeUserInterface(window);

      // Update?
      CheckUpdateMessage(window);
      CheckUpdateFailed(window);
      CheckUpdateCorrupted(window);
      // Wizard
      CheckFirstTimeWizard(window);
      // Alert
      CheckAlert(window, systemManager);

      // Enable joystick autopairing
      if(RecalboxConf::Instance().GetAutoPairOnBoot())
        mBTAutopairManager.StartDiscovery();

      // Main Loop!
      CreateReadyFlagFile();
      Path externalNotificationFolder = Path(sQuitNow).Directory();
      (void)externalNotificationFolder.CreatePath();
      fileNotifier.WatchFile(externalNotificationFolder)
                  .SetEventNotifier(EventType::CloseWrite | EventType::Remove | EventType::Create, this);

      // Main SDL loop
      exitState = MainLoop(window, systemManager, fileNotifier, mSyncMessageFactory);

      ResetExitState();
      fileNotifier.ResetEventNotifier();
      DeleteReadyFlagFile();
      window.deleteAllGui();
    }
    catch(std::exception& ex)
    {
      { LOG(LogError) << "[MainRunner] Main thread crashed (inner)."; }
      { LOG(LogError) << "[MainRunner] Exception: " << ex.what(); }
      exitState = ExitState::Relaunch;
    }

    // Exit
    mNotificationManager.Notify(Notification::Stop, String(mRunCount));
    window.GoToQuitScreen();
    systemManager.DeleteAllSystems(DoWeHaveToUpdateGamelist(exitState));
    WindowManager::Finalize();
    mApplicationWindow = nullptr;

    { LOG(LogInfo) << "[MainRunner] Quit requested (outer) [" << (int)sRequestedExitState << ']'; }
    switch(exitState)
    {
      case ExitState::Quit:
      case ExitState::FatalError: mNotificationManager.Notify(Notification::Quit, exitState == ExitState::FatalError ? "fatalerror" : "quitrequested"); break;
      case ExitState::Relaunch:
      case ExitState::RelaunchNoUpdate: mNotificationManager.Notify(Notification::Relaunch); break;
      case ExitState::NormalReboot:
      case ExitState::FastReboot:
      {
        mNotificationManager.Notify(Notification::Reboot, exitState == ExitState::FastReboot ? "fast" : "normal");
        board.OnRebootOrShutdown();
        break;
      }
      case ExitState::Shutdown:
      case ExitState::FastShutdown: {
        mNotificationManager.Notify(Notification::Shutdown, exitState == ExitState::FastShutdown ? "fast" : "normal");
        board.OnRebootOrShutdown();
        break;
      }
    }

    // Wait for all notifications to be processed before
    // main objects are destroyed
    mNotificationManager.WaitCompletion();

    return exitState;
  }
  catch(std::exception& ex)
  {
    { LOG(LogError) << "[MainRunner] Main thread crashed (outer)."; }
    { LOG(LogError) << "[MainRunner] Exception: " << ex.what(); }
  }

  // If we get there, a severe and probably non-recoverable error occured.
  // Just quit
  return ExitState::FatalError;
}

void MainRunner::CreateReadyFlagFile()
{
  // Create a flag in  temporary directory to signal READY state
  Path ready(sReadyFile);
  Files::SaveFile(ready, "ready");
}

void MainRunner::DeleteReadyFlagFile()
{
  Path ready(sReadyFile);
  (void)ready.Delete();
}

MainRunner::ExitState MainRunner::MainLoop(ApplicationWindow& window, SystemManager& systemManager, FileNotifier& fileNotifier, SyncMessageFactory& syncMessageFactory)
{
  // Allow joystick event
  SDL_JoystickEventState(SDL_ENABLE);

  // Demo mode (real game launching)
  DemoMode demoMode(window, systemManager);

  { LOG(LogDebug) << "[MainRunner] Entering main loop"; }
  Path mustExit(sQuitNow);
  int lastTime = (int)SDL_GetTicks();
  for(;;)
  {
    // File watching
    fileNotifier.CheckAndDispatch();
    InputManager::Instance().WatchJoystickAddRemove(&window);

    // Sync'ed message
    syncMessageFactory.DispatchMessage();

    // SDL
    SDL_Event event;
    while (SDL_PollEvent(&event) != 0)
    {
      switch (event.type)
      {
        case SDL_QUIT: return ExitState::Quit;
        case SDL_TEXTINPUT:
        {
          window.textInput(event.text.text);
          break;
        }
        case SDL_JOYHATMOTION:
        case SDL_JOYBUTTONDOWN:
        case SDL_JOYBUTTONUP:
        case SDL_KEYDOWN:
        case SDL_KEYUP:
        case SDL_JOYAXISMOTION:
        //case SDL_JOYDEVICEADDED:
        //case SDL_JOYDEVICEREMOVED:
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
        case SDL_MOUSEWHEEL:
        {
          // Convert event
          for(;;)
          {
            //{ LOG(LogInfo) << "[MainRunner] Event in Loop event."; }
            InputCompactEvent compactEvent = InputManager::Instance().ManageSDLEvent(&window, event);
            // TODO: invert those lines, special events should be managed by the board in priority
            if (!ProcessSpecialInputs(compactEvent))
              if (!Board::Instance().ProcessSpecialInputs(compactEvent, this))
                if (!compactEvent.Empty())
                  window.ProcessInput(compactEvent);
            // Mouse Wheel event must resend a null move
            if (event.type == SDL_MOUSEWHEEL)
              if ((int)event.wheel.which >= 0) { event.wheel.which = -1; continue; }
            break;
          }
          // Quit?
          if (window.Closed()) RequestQuit(ExitState::Quit);
          break;
        }
        default: break;
      }
    }

    if (window.isSleeping() && !GameClipView::IsGameClipEnabled())
    {
      if (DemoMode::hasDemoMode())
        demoMode.runDemo();

      lastTime = (int)SDL_GetTicks();
      // Take a breath
      SDL_Delay(1);
      continue;
    }

    int curTime = (int)SDL_GetTicks();
    int deltaTime = curTime - lastTime;
    lastTime = curTime;
    if (deltaTime > 1000 || deltaTime < 0) // cap deltaTime at 1000
      deltaTime = 1000;

    window.Update(deltaTime);
    window.RenderAll();

    // Quit Request?
    if (sQuitRequested)
    {
      { LOG(LogInfo) << "[MainRunner] Quit requested (inner) [" << (int)sRequestedExitState << ']'; }
      return sRequestedExitState;
    }

    // Quit pending?
    switch(mPendingExit)
    {
      case PendingExit::None: break;
      case PendingExit::GamelistChanged:
      case PendingExit::ThemeChanged:
      {
        String text = (mPendingExit == PendingExit::GamelistChanged) ?
          _("EmulationStation has detected external changes on a gamelist file.\nTo avoid loss of data, EmulationStation is about to relaunch and reload all files.") :
          _("EmulationStation has detected external changes on a theme file.\nTo avoid loss of data, EmulationStation is about to relaunch and reload all files.");
        GuiMsgBox* msgBox = new GuiMsgBox(window, text, _("OK"), [] { RequestQuit(ExitState::Relaunch, true); });
        window.pushGui(msgBox);
        break;
      }
      case PendingExit::MustExit: RequestQuit(ExitState::Quit);
      case PendingExit::WaitingExit: break;
    }
    if (mPendingExit != PendingExit::None)
      mPendingExit = PendingExit::WaitingExit; // Wait for exit
  }
}

void MainRunner::InitializeUserInterface(WindowManager& window)
{
  (void)window;
  { LOG(LogDebug) << "[MainRunner] Preparing GUI"; }
  ViewController::Instance().goToStart();
}

void MainRunner::CheckAlert(WindowManager& window, SystemManager& systemManager)
{
  int memory = Board::Instance().TotalMemory();
  int maxSystem = 20 * (memory / 256);
  int maxGames = 5000 * (memory / 256);
  if (memory != 0 && memory <= 512)
  {
    int realSystemCount = 0;
    for(const SystemData* system : systemManager.VisibleSystemList())
      if (system->HasVisibleGame())
        realSystemCount++;
    if (realSystemCount > maxSystem)
    {
      String text = _("Your system has not enough memory to handle %SYSTEMS% systems. You should not exceed %MAXSYSTEMS% consoles/computers or you may face stability issues!\n\nYou can hide preinstalled games in UI SETTINGS menu to decrease active systems")
                    .Replace("%SYSTEMS%", String(realSystemCount))
                    .Replace("%MAXSYSTEMS%", String(maxSystem));
      window.pushGui(new GuiMsgBoxScroll(window, _("WARNING! SYSTEM OVERLOAD!"), text, _("OK"), nullptr, "", nullptr, "", nullptr, TextAlignment::Left));
    }
    else if (systemManager.GameCount() > maxGames)
    {
      String text = _("Your system has not enough memory to handle %GAMES% games. You should not exceed %MAXGAMES% or you may face stability issues!")
                         .Replace("%GAMES%", String((int)systemManager.GameCount()))
                         .Replace("%MAXGAMES%", String(maxGames));
      window.pushGui(new GuiMsgBoxScroll(window, _("WARNING! SYSTEM OVERLOAD!"), text, _("OK"), nullptr, "", nullptr, "", nullptr, TextAlignment::Left));
    }
  }
}

void MainRunner::CheckRecalboxLite(WindowManager& window)
{
  if (RecalboxSystem::IsLiteVersion())
    window.pushGui(new WizardLite(window));
}

void MainRunner::CheckFirstTimeWizard(WindowManager& window)
{
  if (RecalboxConf::Instance().GetFirstTimeUse())
  {
    switch (Board::Instance().GetBoardType())
    {
      case BoardType::OdroidAdvanceGo:
      {
        window.pushGui(new WizardAGO2(window));
        return; // Let the OGA Wizard reset the flag
      }
      case BoardType::OdroidAdvanceGoSuper:
      {
        window.pushGui(new WizardAgo3(window));
        return; // Let the OGA Wizard reset the flag
      }
      case BoardType::RG353P:
      case BoardType::RG353V:
      case BoardType::RG353M:
      case BoardType::RG503:
      {
        window.pushGui(new WizardRG353X(window));
        return; // Let the RG Wizard reset the flag
      }
      case BoardType::RG351V: // todo
      case BoardType::RG351P: // todo
      case BoardType::PCx86:
      case BoardType::PCx64:
      case BoardType::UndetectedYet:
      case BoardType::Unknown:
      case BoardType::Pi0:
      case BoardType::Pi02:
      case BoardType::Pi1:
      case BoardType::Pi2:
      case BoardType::Pi3:
      case BoardType::Pi4:
      case BoardType::Pi400:
      case BoardType::Pi5:
      case BoardType::Pi3plus:
      case BoardType::UnknownPi:
      default:
      {
        CheckRecalboxLite(window);
        break;
      }
    }
    RecalboxConf::Instance().SetFirstTimeUse(false);
  }
}

void MainRunner::CheckUpdateMessage(WindowManager& window)
{
  // Push a message box with the changelog if Recalbox has been updated
  Path flag(sUpgradeFileFlag);
  if (flag.Exists())
  {
    String changelog = Files::LoadFile(Path(Upgrade::sLocalReleaseNoteFile));
    String message = "Changes :\n" + changelog;
    window.pushGui(new GuiMsgBoxScroll(window, _("THE SYSTEM IS UP TO DATE"), message, _("OK"), []{}, "", nullptr, "", nullptr, TextAlignment::Left));
    (void)flag.Delete();
  }
}

void MainRunner::CheckUpdateFailed(WindowManager& window)
{
  // Push a message if Recalbox upgrade has failed
  Path flag(sUpgradeFailedFlag);
  if (flag.Exists())
  {
    String version = Upgrade::CurrentVersion();
    String message = _("The upgrade process has failed. You are back on Recalbox %s.\nPlease retry to upgrade your Recalbox, and contact the team on https://forum.recalbox.com if the problem persists.")
                     .Replace("%s", version.c_str());
    window.pushGui(new GuiMsgBoxScroll(window, _("THE UPGRADE HAS FAILED"), message, _("OK"), []{}, "", nullptr, "", nullptr, TextAlignment::Left));
    (void)flag.Delete();
  }
}

void MainRunner::CheckUpdateCorrupted(WindowManager& window)
{
  // Push a message if Recalbox upgrade has failed
  Path flag(sUpgradeCorruptedFlag);
  if (flag.Exists())
  {
    String version = Upgrade::CurrentVersion();
    String message = _("One or more files are corrupted. You are back on Recalbox %s.\nPlease retry to upgrade your Recalbox, check your Recalbox storage (SD Card, USB Key or hard drive).\nContact the team on https://forum.recalbox.com if the problem persists.")
                     .Replace("%s", version.c_str());
    window.pushGui(new GuiMsgBoxScroll(window, _("THE UPGRADE IS CORRUPTED"), message, _("OK"), []{}, "", nullptr, "", nullptr, TextAlignment::Left));
    (void)flag.Delete();
  }
}

void MainRunner::PlayLoadingSound(AudioManager& audioManager)
{
  String selectedTheme = RecalboxConf::Instance().GetThemeFolder();
  Path loadingMusic = RootFolders::DataRootFolder / "system/.emulationstation/themes" / selectedTheme / "fx/loading.ogg";
  if (!loadingMusic.Exists())
    loadingMusic = RootFolders::DataRootFolder / "themes" / selectedTheme / "fx/loading.ogg";
  if (loadingMusic.Exists())
  {
    audioManager.PlayMusic(audioManager.LoadMusic(loadingMusic), false);
  }
}

bool MainRunner::TryToLoadConfiguredSystems(SystemManager& systemManager, FileNotifier& gamelistWatcher, bool forceReloadFromDisk)
{
  IniFile recalboxBootConf(Path("/boot/recalbox-boot.conf"), false, false);
  bool portable = recalboxBootConf.AsString("case") == "GPiV1:1";
  switch(Board::Instance().GetBoardType())
  {
    case BoardType::RG351V:
    case BoardType::RG351P:
    case BoardType::RG353P:
    case BoardType::RG353V:
    case BoardType::RG353M:
    case BoardType::RG503:
    case BoardType::OdroidAdvanceGo:
    case BoardType::OdroidAdvanceGoSuper: portable = true; break;
    case BoardType::UndetectedYet:
    case BoardType::Unknown:
    case BoardType::Pi0:
    case BoardType::Pi02:
    case BoardType::Pi1:
    case BoardType::Pi2:
    case BoardType::Pi3:
    case BoardType::Pi3plus:
    case BoardType::Pi4:
    case BoardType::Pi400:
    case BoardType::Pi5:
    case BoardType::UnknownPi:
    case BoardType::PCx86:
    case BoardType::PCx64:
    default: break;
  }

  if (!systemManager.LoadSystemConfigurations(gamelistWatcher, forceReloadFromDisk, portable))
  {
    { LOG(LogError) << "[MainRunner] Error while parsing systems configuration file!"; }
    { LOG(LogError) << "[MainRunner] IT LOOKS LIKE YOUR SYSTEMS CONFIGURATION FILE HAS NOT BEEN SET UP OR IS INVALID. YOU'LL NEED TO DO THIS BY HAND, UNFORTUNATELY.\n\n"
                       "VISIT EMULATIONSTATION.ORG FOR MORE INFORMATION."; }
    return false;
  }

  if (systemManager.VisibleSystemList().Empty())
  {
    { LOG(LogError) << "[MainRunner] No systems found! Does at least one system have a game present? (check that extensions match!)\n(Also, make sure you've updated your es_systems.cfg for XML!)"; }
    { LOG(LogError) << "[MainRunner]  WE CAN'T FIND ANY SYSTEMS!\n"
                       "CHECK THAT YOUR PATHS ARE CORRECT IN THE SYSTEMS CONFIGURATION FILE, AND "
                       "YOUR GAME DIRECTORY HAS AT LEAST ONE GAME WITH THE CORRECT EXTENSION.\n"
                       "\n"
                       "VISIT RECALBOX.FR FOR MORE INFORMATION."; }
    return false;
  }

  return true;
}

void onExit()
{
  ::Log::Close();
}

void Sdl2Log(void *userdata, int category, SDL_LogPriority priority, const char *message)
{
  (void)userdata;
  const char* cat = "Unknown";
  switch(category)
  {
    case SDL_LOG_CATEGORY_APPLICATION: cat = "Application"; break;
    case SDL_LOG_CATEGORY_ERROR: cat = "Error"; break;
    case SDL_LOG_CATEGORY_ASSERT: cat = "Assert"; break;
    case SDL_LOG_CATEGORY_SYSTEM: cat = "System"; break;
    case SDL_LOG_CATEGORY_AUDIO: cat = "Audio"; break;
    case SDL_LOG_CATEGORY_VIDEO: cat = "Video"; break;
    case SDL_LOG_CATEGORY_RENDER: cat = "Render"; break;
    case SDL_LOG_CATEGORY_INPUT: cat = "Input"; break;
    case SDL_LOG_CATEGORY_TEST: cat = "Test"; break;
    default: break;
  }
  const char*  subType = "Unknown";
  switch(priority)
  {
    case SDL_LOG_PRIORITY_VERBOSE: subType = "Verbose"; break;
    case SDL_LOG_PRIORITY_DEBUG: subType = "Debug"; break;
    case SDL_LOG_PRIORITY_INFO: subType = "Info"; break;
    case SDL_LOG_PRIORITY_WARN: subType = "Warning"; break;
    case SDL_LOG_PRIORITY_ERROR: subType = "Error"; break;
    case SDL_LOG_PRIORITY_CRITICAL: subType = "Critical"; break;
    case SDL_NUM_LOG_PRIORITIES:
    default: break;
  }
  { LOG(LogDebug) << "[SDL2] (" << cat << ':' << subType << ") " << message; }
}

void MainRunner::SetDebugLogs(bool debug, bool trace)
{
  if (trace)
  {
    if (::Log::ReportingLevel() < LogLevel::LogTrace) ::Log::SetReportingLevel(LogLevel::LogTrace);
    SDL_LogSetOutputFunction(Sdl2Log, nullptr);
    SDL_LogSetAllPriority(SDL_LogPriority::SDL_LOG_PRIORITY_VERBOSE);
  }
  else if (debug)
  {
    if (::Log::ReportingLevel() < LogLevel::LogDebug) ::Log::SetReportingLevel(LogLevel::LogDebug);
    SDL_LogSetOutputFunction(Sdl2Log, nullptr);
    SDL_LogSetAllPriority(SDL_LogPriority::SDL_LOG_PRIORITY_VERBOSE);
  }
  else
  {
    ::Log::SetReportingLevel(LogLevel::LogInfo);
    SDL_LogSetOutputFunction(nullptr, nullptr);
    SDL_LogSetAllPriority(SDL_LogPriority::SDL_LOG_PRIORITY_ERROR);
  }
}

void MainRunner::Intro(bool debug, bool trace)
{
  if (atexit(&onExit) != 0) // Always close the log on exit
    { LOG(LogError) << "[MainRunner] Error setting exit function!"; }

  SetDebugLogs(debug || mConfiguration.GetDebugLogs(), trace);

  { LOG(LogInfo) << "[MainRunner] EmulationStation - v" << PROGRAM_VERSION_STRING << ", built " << PROGRAM_BUILT_STRING; }
}

void MainRunner::CheckHomeFolder()
{
  //make sure the config directory exists
  Path home = RootFolders::DataRootFolder;
  Path configDir = home / "system/.emulationstation";
  if (!configDir.Exists())
  {
    { LOG(LogError) << "[MainRunner] Creating config directory \"" << configDir.ToString() << "\"\n"; }
    if (!configDir.CreatePath()) { LOG(LogError) << "[MainRunner] Config directory could not be created!\n"; }
  }
}

void MainRunner::SetLocale(const String& executablePath)
{
  Path path(executablePath);
  path = path.Directory(); // Get executable folder
  if (path.IsEmpty() || !path.Exists()) { LOG(LogError) << "[Locale] Error getting executable path (received: " << executablePath << ')'; }

  // Get locale from configuration
  String localeName = RecalboxConf::Instance().GetSystemLanguage();

  // Set locale
  if (!Internationalizer::InitializeLocale(localeName, { path / "locale/lang", Path("/usr/share/locale") }, "emulationstation2"))
  { LOG(LogWarning) << "[Locale] No locale found. Default text used."; }
}

void MainRunner::RequestQuit(MainRunner::ExitState requestedState, bool forceReloadFromDisk)
{
  sQuitRequested = true;
  sRequestedExitState = requestedState;
  sForceReloadFromDisk = forceReloadFromDisk;
}

bool MainRunner::DoWeHaveToUpdateGamelist(MainRunner::ExitState state)
{
  switch(state)
  {
    case ExitState::Quit:
    case ExitState::NormalReboot:
    case ExitState::Shutdown:
    case ExitState::Relaunch: return true;
    case ExitState::RelaunchNoUpdate:
    case ExitState::FatalError:
    case ExitState::FastReboot:
    case ExitState::FastShutdown: break;
  }
  return false;
}

void MainRunner::FileSystemWatcherNotification(EventType event, const Path& path, const DateTime& time)
{
  (void)time;

  if (path == sQuitNow)
    event = event | EventType::None;

  if (mPendingExit == PendingExit::None)
  {
    if (((event & EventType::Create) != 0) && (path == sQuitNow))
      mPendingExit = PendingExit::MustExit;
    else if ((event & (EventType::Remove | EventType::CloseWrite)) != 0)
    {
      if(IsFileIgnored(path.ToString()))
        return;

      String name = path.Filename();
      if (name == "gamelist.xml" || name == "gamelist.zip")
        mPendingExit = PendingExit::GamelistChanged;
      else if (path.ToString().Find("themes") >= 0)
        mPendingExit = PendingExit::ThemeChanged;
    }
  }
}

void MainRunner::HeadphonePluggedIn(BoardType board)
{
  (void)board;
  { LOG(LogInfo) << "[Audio] Headphones plugged!"; }
  Board::Instance().HeadphonePlugged();
}

void MainRunner::HeadphoneUnplugged(BoardType board)
{
  (void)board;
  { LOG(LogInfo) << "[Audio] Headphones unplugged!"; }
  Board::Instance().HeadphoneUnplugged();
}

void MainRunner::ResetButtonPressed(BoardType board)
{
  (void)board;
  if (IsApplicationRunning())
  {
    // The application is running and is on screen.
    // Display little window to notify the user we are going to reset
    { LOG(LogDebug) << "[MainRunner] Reset Button Pressed : reseting"; }
    mApplicationWindow->pushGui((new GuiWaitLongExecution<HardwareTriggeredSpecialOperations, bool>(*mApplicationWindow, *this))
                                      ->Execute(HardwareTriggeredSpecialOperations::Reset, _("Restarting.")));
  } else {
    // Something is running (game, demo, kodi)
    Files::SaveFile(Path(sStopDemo), String());
    { LOG(LogDebug) << "[MainRunner] Reset Button Pressed in game : exiting subprocesses"; }
    ProcessTree::TerminateAll(1000);
  }
}

void MainRunner::UnderVoltage(BoardType board)
{
  (void)board;
  { LOG(LogInfo) << "[MainRunner] Undervoltage popup."; }
  String message = _("An undervoltage has been detected, the system may slow down.\n");
  String suffix;
  switch(board)
  {
    case(BoardType::Pi400): suffix = " 400."; break;
    case(BoardType::Pi5): suffix = " 5."; break;
    case(BoardType::Pi4): suffix = " 4."; break;
    case BoardType::UndetectedYet:
    case BoardType::Unknown:
    case BoardType::Pi0:
    case BoardType::Pi02:
    case BoardType::Pi1:
    case BoardType::Pi2:
    case BoardType::Pi3:
    case BoardType::Pi3plus:
    case BoardType::UnknownPi:
    case BoardType::OdroidAdvanceGo:
    case BoardType::OdroidAdvanceGoSuper:
    case BoardType::RG351V:
    case BoardType::RG351P:
    case BoardType::RG353P:
    case BoardType::RG353V:
    case BoardType::RG353M:
    case BoardType::RG503:
    case BoardType::PCx86:
    case BoardType::PCx64:
    default: suffix = "."; break;

  }
  if(Board::Instance().CrtBoard().GetCrtAdapter() == CrtAdapterType::RGBJamma)
    message.Append(_("We recommend adjusting your JAMMA cabinet power supply to increase the voltage to between 5.05V and 5.2V"));
  else if (BoardTypeUtil::IsRaspberryPi(Board::Instance().GetBoardType()))
    message.Append(_("We recommend that you purchase an official USB-C power supply designed for your Raspberry Pi").Append(suffix));

  mApplicationWindow->InfoPopupAdd(new GuiInfoPopup(*mApplicationWindow, message, 15, PopupType::Warning));
}

void MainRunner::TemperatureAlert(BoardType board)
{
  (void)board;
  { LOG(LogInfo) << "[MainRunner] Temperature popup."; }
  String message = _("The temperature of your system is high.\nThe system may slow down. Try cooling your Raspberry Pi with a fan or disable overclock if it's enabled.");
  mApplicationWindow->InfoPopupAdd(new GuiInfoPopup(*mApplicationWindow, message, 15, PopupType::Warning));
}

void MainRunner::PowerButtonPressed(BoardType board, int milliseconds)
{
  (void)board;
  if (IsApplicationRunning())
  {
    // The application is running and is on screen.
    // Display little window to notify the user
    if (milliseconds < sPowerButtonThreshold)
    {
      // Only if supported. Otherwise does nothing
      { LOG(LogDebug) << "[MainRunner] Short Power Button Press: standby"; }
      if (Board::Instance().HasSuspendResume())
        mApplicationWindow->pushGui((new GuiWaitLongExecution<HardwareTriggeredSpecialOperations, bool>(*mApplicationWindow, *this))
                                      ->Execute(HardwareTriggeredSpecialOperations::Suspend, _("Entering standby...")));
    }
    else
    {
      { LOG(LogDebug) << "[MainRunner] Long Power Button Press: shutting down"; }
      mApplicationWindow->pushGui((new GuiWaitLongExecution<HardwareTriggeredSpecialOperations, bool>(*mApplicationWindow, *this))
                                      ->Execute(HardwareTriggeredSpecialOperations::PowerOff, _("Bye bye!")));
    }
    return;
  }

  // The application is not Running, execute orders immediately
  if (milliseconds < sPowerButtonThreshold)
  {
    { LOG(LogDebug) << "[MainRunner] Power Button Pressed while running game: suspending"; }
    // Only if supported. Otherwise does nothing
    if (Board::Instance().HasSuspendResume())
      Board::Instance().Suspend();
  }
  else
  {
    { LOG(LogDebug) << "[MainRunner] Power Button Pressed while running game: shutting down"; }
    Files::SaveFile(Path(sQuitNow), String());
    Files::SaveFile(Path(sStopDemo), String());
    // Gracefuly qui emulators and all the call chain
    ProcessTree::TerminateAll(1000);
    // Quit
    RequestQuit(ExitState::Shutdown);
  }
}

void MainRunner::Resume(BoardType board)
{
  (void)board;
  // so... Waking up :)
  if (mApplicationWindow != nullptr && IsApplicationRunning())
    mApplicationWindow->pushGui((new GuiWaitLongExecution<HardwareTriggeredSpecialOperations, bool>(*mApplicationWindow, *this))
                                ->Execute(HardwareTriggeredSpecialOperations::Resume, _("Waking up!")));
}

bool MainRunner::Execute(GuiWaitLongExecution<HardwareTriggeredSpecialOperations, bool>& from,
                         const HardwareTriggeredSpecialOperations& parameter)
{
  (void)from;
  switch(parameter)
  {
    case HardwareTriggeredSpecialOperations::PowerOff:
    case HardwareTriggeredSpecialOperations::Reset:
    {
      mApplicationWindow->DoWake();
      Thread::Sleep(1000); // Just sleep one second
      break;
    }
    case HardwareTriggeredSpecialOperations::Suspend:
    case HardwareTriggeredSpecialOperations::Resume:
    default: Thread::Sleep(1000); // Just sleep one second
  }
  return false; // unused
}

void MainRunner::Completed(const HardwareTriggeredSpecialOperations& parameter, const bool& result)
{
  (void)result;
  switch(parameter)
  {
    case HardwareTriggeredSpecialOperations::Suspend:
    {
      // Here is a little trick to erase the window from the screen before suspending
      // To show the user we're soon suspending the hardware, just display the last screen in half luminosity
      mApplicationWindow->Update(20);
      mApplicationWindow->RenderAll(true);

      // This method won't return until wake up
      Board::Instance().Suspend();
      break;
    }
    case HardwareTriggeredSpecialOperations::PowerOff:
    {
      // Bye bye :)
      RequestQuit(ExitState::Shutdown);
      break;
    }
    case HardwareTriggeredSpecialOperations::Reset:
    {
      RequestQuit(ExitState::NormalReboot);
      break;
    }
    case HardwareTriggeredSpecialOperations::Resume:
    {
      // Set audio output since headphone may have been plugged/unplugged
      String output = RecalboxConf::Instance().GetAudioOuput();
      AudioManager::Instance().Deactivate();
      AudioController::Instance().Refresh();
      AudioController::Instance().SetDefaultPlayback(output);
      AudioManager::Instance().Reactivate();
    }
    default: break;
  }
}

void MainRunner::VolumeDecrease(BoardType board, float percent)
{
  (void)board;

  int value = RecalboxConf::Instance().GetAudioVolume() - (int)(100 * percent);
  value = Math::clampi(value, 0, 100);
  value = (value / 10) * 10;
  AudioController::Instance().SetVolume(value);
  RecalboxConf::Instance().SetAudioVolume(value);
  RecalboxConf::Instance().Save();
}

void MainRunner::VolumeIncrease(BoardType board, float percent)
{
  (void)board;

  int value = RecalboxConf::Instance().GetAudioVolume() + (int)(100 * percent);
  value = Math::clampi(value, 0, 100);
  value = (value / 10) * 10;
  AudioController::Instance().SetVolume(value);
  RecalboxConf::Instance().SetAudioVolume(value);
  RecalboxConf::Instance().Save();
}

void MainRunner::BrightnessDecrease(BoardType board, float percent)
{
  (void)percent;
  (void)board;
  int value = RecalboxConf::Instance().GetBrightness() - 1;
  value = Math::clampi(value, 0, 8);
  Board::Instance().SetBrightness(value);
  RecalboxConf::Instance().SetBrightness(value);
  RecalboxConf::Instance().Save();
}

void MainRunner::BrightnessIncrease(BoardType board, float percent)
{
  (void)percent;
  (void)board;
  int value = RecalboxConf::Instance().GetBrightness() + 1;
  value = Math::clampi(value, 0, 8);
  Board::Instance().SetBrightness(value);
  RecalboxConf::Instance().SetBrightness(value);
  RecalboxConf::Instance().Save();
}

void MainRunner::RomPathAdded(const DeviceMount& device)
{
  String text = _("The device %NAME% containing roms has been plugged in! EmulationStation must relaunch to load new games.")
                     .Replace("%NAME%", device.Name());
  if (device.ReadOnly())
    text.Append(_("\nWARNING: You device may not have been properly unplugged and has consistency errors. As a result, it's been mounted as read-only. You should plug your device in a Window PC and use the repair tool."));
  GuiMsgBox* msgBox = new GuiMsgBox(*mApplicationWindow, text, _("OK"), [] { RequestQuit(ExitState::Relaunch, true); }, _("LATER"), []{});
  mApplicationWindow->pushGui(msgBox);
}

void MainRunner::RomPathRemoved(const DeviceMount& device)
{
  (void)device;
  String text = _("A device containing roms has been unplugged! EmulationStation must relaunch to remove unavailable games.");
  GuiMsgBox* msgBox = new GuiMsgBox(*mApplicationWindow, text, _("OK"), [] { RequestQuit(ExitState::Relaunch, true); }, _("LATER"), []{});
  mApplicationWindow->pushGui(msgBox);
}

void MainRunner::NoRomPathFound(const DeviceMount& device)
{
  auto initializeRoms = [this, &device]
  {
    USBInitialization init(USBInitializationAction::OnlyRomFolders, &device);
    mApplicationWindow->pushGui((new GuiWaitLongExecution<USBInitialization, bool>(*mApplicationWindow, *this))->Execute(init, _("Initializing roms folders...")));
  };

  auto moveShareFolder = [this, &device]
  {
    USBInitialization init(USBInitializationAction::CompleteShare, &device);
    mApplicationWindow->pushGui((new GuiWaitLongExecution<USBInitialization, bool>(*mApplicationWindow, *this))->Execute(init, _("Initializing share folders...")));
  };

  String text = _("The USB device %NAME% with no rom folder and no share folder has been plugged in! Would you like to initialize this device?");
  text.Append('\n')
      .Append(_("• Choose '%INIT%' to create only all the rom folders")).Append('\n')
      .Append(_("• Choose '%MOVE%' to copy all the current share to the new device, automatically switch to this device, and reboot")).Append('\n')
      .Append(_("• Or just chose '%CANCEL%' to do nothing with this new device"))
      .Replace("%NAME%", device.Name())
      .Replace("%INIT%", _("INITIALIZE"))
      .Replace("%MOVE%", _("MOVE SHARE"))
      .Replace("%CANCEL%", _("CANCEL"));
  GuiMsgBox* msgBox = new GuiMsgBox(*mApplicationWindow, text, _("INITIALIZE"), initializeRoms,
                                                               _("MOVE SHARE"), moveShareFolder,
                                                               _("CANCEL"), nullptr, TextAlignment::Left);
  mApplicationWindow->pushGui(msgBox);
}

void MainRunner::PatreonState(PatronAuthenticationResult result, int level, const String& name)
{
  String message;
  switch(result)
  {
    case PatronAuthenticationResult::Patron:
    {
      message = _("Welcome back %NAME%!\nPatron level %LEVEL%\nYou are now connected to your recalbox patron account, and all exclusives features are available!")
                .Replace("%NAME%", name)
                .Replace("%LEVEL%", String(level));
      break;
    }
    case PatronAuthenticationResult::FormerPatron:
    {
      message = _("Hello %NAME%, your private key is linked to a Patreon account which is no longer a Recalbox Patron.\nWe still hope to see you back soon as a Recalbox Patron!\nDelete your private key to suppress this message.")
                .Replace("%NAME%", name);
      break;
    }
    case PatronAuthenticationResult::Invalid:
    {
      message = _("Your private key does not allow to retrieve your Patreon information. Go to recalbox.com/patreon to generate a new valid key!");
      break;
    }
    case PatronAuthenticationResult::NetworkError:
    {
      message = _("Sorry we're not able to retrieve your Patron level because no network is available!");
      break;
    }
    case PatronAuthenticationResult::HttpError:
    case PatronAuthenticationResult::ApiError:
    {
      message = _("We're not able to retrieve your Patron level! Sorry for the inconvenience, we're already working on a fix!");
      break;
    }
    case PatronAuthenticationResult::NoPatron:
    case PatronAuthenticationResult::Unknown:
    default: break;
  }

  if (!message.empty())
    mApplicationWindow->InfoPopupAdd(new GuiInfoPopup(*mApplicationWindow, message, 15, PopupType::Help));
}

bool MainRunner::Execute(GuiWaitLongExecution<USBInitialization, bool>& from, const USBInitialization& parameter)
{
  (void) from;
  switch(parameter.Action())
  {
    case USBInitializationAction::None: break;
    case USBInitializationAction::OnlyRomFolders: return SystemManager::CreateRomFoldersIn(parameter.Device());
    case USBInitializationAction::CompleteShare:
    {
      struct
      {
        Path From;
        Path To;
      }
      pathFromTo[]
      {
        { Path("/recalbox/share/bios")       , parameter.Device().MountPoint() / "recalbox/bios"       },
        { Path("/recalbox/share/cheats")     , parameter.Device().MountPoint() / "recalbox/cheats"     },
        { Path("/recalbox/share/kodi")       , parameter.Device().MountPoint() / "recalbox/kodi"       },
        { Path("/recalbox/share/music")      , parameter.Device().MountPoint() / "recalbox/music"      },
        { Path("/recalbox/share/overlays")   , parameter.Device().MountPoint() / "recalbox/overlays"   },
        { Path("/recalbox/share/roms")       , parameter.Device().MountPoint() / "recalbox/roms"       },
        { Path("/recalbox/share/saves")      , parameter.Device().MountPoint() / "recalbox/saves"      },
        { Path("/recalbox/share/screenshots"), parameter.Device().MountPoint() / "recalbox/screenshots"},
        { Path("/recalbox/share/shaders")    , parameter.Device().MountPoint() / "recalbox/shaders"    },
        { Path("/recalbox/share/system")     , parameter.Device().MountPoint() / "recalbox/system"     },
        { Path("/recalbox/share/themes")     , parameter.Device().MountPoint() / "recalbox/themes"     },
        { Path("/recalbox/share/userscripts"), parameter.Device().MountPoint() / "recalbox/userscripts"},
      };

      // Copy share folders
      for(const auto& ft : pathFromTo)
      {
        { LOG(LogInfo) << "[MainRunner] USB Initialization: Copying " << ft.From.Filename(); }
        from.SetText(_("Copying %s folder...").Replace("%s", ft.From.Filename()));
        if (!Files::CopyFolder(ft.From, ft.To, true)) return false;
      }

      // Install new device
      { LOG(LogInfo) << "[MainRunner] USB Initialization: Setup boot device."; }
      from.SetText(_("Setting up boot device..."));
      StorageDevices storages;
      for(const StorageDevices::Device& storage : storages.GetStorageDevices())
        if (storage.DevicePath == parameter.Device().Device().ToString())
        {
          { LOG(LogInfo) << "[MainRunner] USB Initialization: Boot device set to " << storage.UUID; }
          storages.SetStorageDevice(storage);
          return true;
        }
    }
  }
  return false;
}

void MainRunner::Completed(const USBInitialization& parameter, const bool& result)
{
  switch(parameter.Action())
  {
    case USBInitializationAction::None: break;
    case USBInitializationAction::OnlyRomFolders:
    {
      mApplicationWindow->pushGui(new GuiMsgBox(*mApplicationWindow, result ? _("Your USB device has been initialized! You can unplug it and copy your games on it.")
                                                                            : _("Initialization failed! Your USB device is full or contains errors. Please repair or use another device."), _("OK"), nullptr));
      break;
    }
    case USBInitializationAction::CompleteShare:
    {
      if (result)
        mApplicationWindow->pushGui(new GuiMsgBox(*mApplicationWindow, _("Your USB device has been initialized! Ready to reboot on your new share device!"),
                                                  _("OK"), [] { MainRunner::RequestQuit(MainRunner::ExitState::Relaunch); }));
      else
        mApplicationWindow->pushGui(new GuiMsgBox(*mApplicationWindow, _("Initialization failed! Your USB device is full or contains errors. Please repair or use another device."), _("OK"), nullptr));
      break;
    }
  }
}

void MainRunner::Sdl2EventReceived(const SDL_Event& event)
{
  if (event.type == SDL_KEYDOWN)
    ProcessSpecialInputs(InputManager::Instance().ManageSDLEvent(mApplicationWindow, event));
}

bool MainRunner::ProcessSpecialInputs(const InputCompactEvent& event)
{
  // Setting this high quality code will avoid the MainRunner to manage
  // power buttons on those specific boards.
  // TODO: Must be cleaned after 8.1
  const BoardType board = Board::Instance().GetBoardType();
  if (board == BoardType::OdroidAdvanceGo || board == BoardType::OdroidAdvanceGoSuper || board == BoardType::Pi400 ||
      board == BoardType::RG353P || board == BoardType::RG353V || board == BoardType::RG353M || board == BoardType::RG503 ||
      board == BoardType::RG351V || board == BoardType::RG351P)
    return false;
  const InputEvent& raw = event.RawEvent();
  if (raw.Type() == InputEvent::EventType::Key)
    if (raw.Value() == 1) // KEYDOWN
      switch(raw.Id())
      {
        case SDLK_POWER: PowerButtonPressed(board, sPowerButtonThreshold); return true;
        case SDLK_SLEEP: ResetButtonPressed(board); return true;
        default: break;
      }
  return false;
}

void MainRunner::ScanComplete()
{
  BiosManager& manager = BiosManager::Instance();
  WindowManager& window = *mApplicationWindow;
  if (manager.Moved() && !manager.MoveError())
    window.displayMessage(_("With regard to the new BIOS folder structure, some of your bios files have been moved automatically to their new path.").Append("\n\n").Append(_("This move is applied only once. No additional operation required.")));
  else if (manager.Moved() && manager.MoveError())
    window.displayMessage(_("With regard to the new BIOS folder structure, some of your bios files have been moved automatically to their new path.").Append("\n\n").Append(_("However, some files failed to move. You should run the BIOS Checker and move some files manually.")));
  else if (manager.MoveError())
    window.displayMessage(_("With regard to the new BIOS folder structure, some of your bios files have been moved automatically to their new path.").Append("\n\n").Append(_("However, all files failed to move. You should:\n- either run the BIOS Checker and move the required files manually.\n- or if your bios files are on a read-only device or remote share, change it to read-write, reboot your recalbox, wait until all files are moved, then protect it again.")));
}
