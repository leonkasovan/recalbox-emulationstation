//
// Created by matthieu on 12/09/15.
//
#pragma once

#include <utils/IniFile.h>
#include <utils/String.h>
#include <utils/cplusplus/StaticLifeCycleControler.h>
#include <games/FileSorts.h>
#include <scraping/ScraperTools.h>
#include <scraping/scrapers/screenscraper/ScreenScraperEnums.h>
#include <scraping/scrapers/screenscraper/Languages.h>
#include <audio/AudioMode.h>
#include "systems/SystemSorting.h"
#include "IRecalboxConfChanged.h"
#include <scraping/ScraperType.h>

// Forward declaration
class SystemData;

class RecalboxConf: public IniFile, public StaticLifeCycleControler<RecalboxConf>
{
  public:
    /*!
     * @brief Constructor
     * @param initialConfigOnly true if only the original file has to be loaded
     */
    RecalboxConf();

    //! Virtual destructor
    ~RecalboxConf() override = default;

    /*!
     * @brief Called when file has been saved
     */
    void OnSave() const override;

    /*
     * Watching
     */

    void Watch(const String& key, IRecalboxConfChanged& callback);

    /*
     * Enums
     */

    enum class Menu
    {
        Default, //!< All menu available
        Bartop,  //!< Limited menu
        None,    //!< No menu except exit
    };

    enum class Screensaver
    {
      Black,    //!< Black screen
      Dim,      //!< 50% luminosity
      Demo,     //!< Real game demos
      Gameclip, //!< Game clips
      Suspend,  //!< Hardware suspend
    };

    enum class Relay
    {
        None,     //!< No relay
        NewYork,  //!< New-york relay
        Madrid,   //!< Madrid relay
        Montreal, //!< Montreal relay
        Saopaulo, //!< SaoPaulo relay
    };

    enum class SoftPatching
    {
      Disable, //!< No soft patching
      Auto,    //!< Automatic patch selection
      LaunchLast,    //!< Automatic patch selection
      Select,  //!< Manual patch selection
    };

    enum class PadOSDType
    {
      Snes, //!< SNES type pad
      MD  , //!< Megadrive type pad
      XBox, //!< XBox type pad
      PSX , //!< Playstation type pad
      N64 , //!< Nintendo 64 type pad
      DC  , //!< Dreamcast pad type
    };

    /*
     * Shortcuts
     */

    #define DefineGetterSetterGeneric(clazz, name, type, type2, key, defaultValue) \
      type Get##name() const { return As##type2(key, defaultValue); } \
      clazz& Delete##name() { Delete(key); return *this; } \
      bool IsDefined##name() const { return IsDefined(key); } \
      clazz& Set##name(const type& value) { Set##type2(key, value); return *this; } \
      bool Has##name() const { return HasKey(key); }

    #define DefineGetterSetter(name, type, type2, key, defaultValue) \
      DefineGetterSetterGeneric(RecalboxConf, name, type, type2, key, defaultValue)

    #define DefineListGetterSetter(name, key, defaultValue) \
      String::List Get##name() const { return AsString(key, defaultValue).Split(','); } \
      String GetRaw##name() const { return AsString(key, defaultValue); } \
      bool IsIn##name(const String& value) const { return isInList(key, value); } \
      RecalboxConf& Set##name(const String::List& value) { SetString(key, String::Join(value, ',')); return *this; }

    #define DefineGetterSetterParameterized(name, type, type2, keybefore, keyafter, defaultValue) \
      type Get##name(const String& subkey) const { return As##type2(String(keybefore).Append(subkey).Append(keyafter), defaultValue); } \
      RecalboxConf& Set##name(const String& subkey, const type& value) { Set##type2(String(keybefore).Append(subkey).Append(keyafter), value); return *this; }

    #define DefineSystemGetterSetterImplementation(name, type, type2, key, defaultValue) \
      type RecalboxConf::GetSystem##name(const SystemData& system) const { return As##type2(String(system.Name()).Append('.').Append(key), defaultValue); } \
      RecalboxConf& RecalboxConf::SetSystem##name(const SystemData& system, const type& value) { Set##type2(String(system.Name()).Append('.').Append(key), value); return *this; } \
      RecalboxConf& RecalboxConf::DeleteSystem##name(const SystemData& system) { Delete(String(system.Name()).Append('.').Append(key)); return *this; } \
      bool RecalboxConf::IsDefinedSystem##name(const SystemData& system) const { return IsDefined(String(system.Name()).Append('.').Append(key)); }

    #define DefineEmulationStationSystemGetterSetterImplementation(name, type, type2, key, defaultValue) \
      type RecalboxConf::GetSystem##name(const SystemData& system) const { return As##type2(String("emulationstation.").Append(system.Name()).Append('.').Append(key), defaultValue); } \
      RecalboxConf& RecalboxConf::SetSystem##name(const SystemData& system, const type& value) { Set##type2(String("emulationstation.").Append(system.Name()).Append('.').Append(key), value); return *this; }

    #define DefineSystemGetterSetterDeclaration(name, type, type2, key) \
      type GetSystem##name(const SystemData& system) const; \
      RecalboxConf& SetSystem##name(const SystemData& system, const type& value); \
      RecalboxConf& DeleteSystem##name(const SystemData& system); \
      bool IsDefinedSystem##name(const SystemData& system) const;

    #define DefineEmulationStationSystemGetterSetterDeclaration(name, type, type2, key) \
      type GetSystem##name(const SystemData& system) const; \
      RecalboxConf& SetSystem##name(const SystemData& system, const type& value);

    #define DefineGetterSetterEnumGeneric(clazz, name, enumType, key, adapterPrefix) \
      enumType Get##name() const { return adapterPrefix##FromString(AsString(key, "")); } \
      clazz& Set##name(enumType value) { SetString(key, adapterPrefix##FromEnum(value)); return *this; }

    #define DefineGetterSetterEnum(name, enumType, key, adapterPrefix) \
       DefineGetterSetterEnumGeneric(RecalboxConf, name, enumType, key, adapterPrefix)

    #define DefineEmulationStationSystemGetterSetterNumericEnumDeclaration(name, enumType) \
      enumType GetSystem##name(const SystemData& system) const; \
      RecalboxConf& SetSystem##name(const SystemData& system, enumType value);

    #define DefineEmulationStationSystemGetterSetterNumericEnumImplementation(name, enumType, key, defaultValue) \
      enumType RecalboxConf::GetSystem##name(const SystemData& system) const { return (enumType)AsInt(String("emulationstation.").Append(system.Name()).Append('.').Append(key), (int)(defaultValue)); } \
      RecalboxConf& RecalboxConf::SetSystem##name(const SystemData& system, enumType value) { SetInt(String("emulationstation.").Append(system.Name()).Append('.').Append(key), (int)value); return *this; }

    #define DefineEmulationStationSystemListGetterSetterDeclaration(name, key) \
      String::List Get##name(const SystemData& system) const; \
      bool IsIn##name(const SystemData& system, const String& value) const; \
      RecalboxConf& Set##name(const SystemData& system, const String::List& value);

    #define DefineEmulationStationSystemListGetterSetterImplementation(name, key, defaultValue) \
      String::List RecalboxConf::Get##name(const SystemData& system) const { return AsString(String("emulationstation.").Append(system.Name()).Append('.').Append(key), defaultValue).Split(','); } \
      bool RecalboxConf::IsIn##name(const SystemData& system, const String& value) const { return isInList(String("emulationstation.").Append(system.Name()).Append('.').Append(key), value); } \
      RecalboxConf& RecalboxConf::Set##name(const SystemData& system, const String::List& value) { SetString(String("emulationstation.").Append(system.Name()).Append('.').Append(key), String::Join(value, ',')); return *this; }

    DefineEmulationStationSystemListGetterSetterDeclaration(ArcadeSystemHiddenManufacturers, sArcadeSystemHiddenManufacturers)

    DefineGetterSetterEnum(MenuType, Menu, sMenuType, Menu)
    DefineGetterSetterEnum(ScraperNameOptions, ScraperNameOptions, sScraperGetNameFrom, ScraperTools::ScraperNameOptions)
    DefineGetterSetterEnum(ScreenScraperRegionPriority, ScreenScraperEnums::ScreenScraperRegionPriority, sScreenScraperRegionPriority, ScreenScraperEnums::ScreenScraperRegionPriority)
    DefineGetterSetterEnum(ScreenScraperRegion, Regions::GameRegions, sScreenScraperRegion, Regions::GameRegions)
    DefineGetterSetterEnum(ScreenScraperLanguage, Languages, sScreenScraperLanguage, LanguagesTools::Language)
    DefineGetterSetterEnum(ScreenScraperMainMedia, ScreenScraperEnums::ScreenScraperImageType, sScreenScraperMainMedia, ScreenScraperEnums::ScreenScraperImageType)
    DefineGetterSetterEnum(ScreenScraperThumbnail, ScreenScraperEnums::ScreenScraperImageType, sScreenScraperThumbnail, ScreenScraperEnums::ScreenScraperImageType)
    DefineGetterSetterEnum(ScreenScraperVideo, ScreenScraperEnums::ScreenScraperVideoType, sScreenScraperVideo, ScreenScraperEnums::ScreenScraperVideoType)
    DefineGetterSetterEnum(AudioMode, AudioMode, sAudioOptions, AudioModeTools::AudioMode)
    DefineGetterSetterEnum(SystemSorting, SystemSorting, sSystemSorting, SystemSorting)

    DefineGetterSetter(DebugLogs, bool, Bool, sDebugLogs, false)

    DefineGetterSetter(Hostname, String, String, sHostname, "RECALBOX")

    DefineGetterSetter(WifiEnabled, bool, Bool, sWifiEnabled, false)
    DefineGetterSetter(WifiSSID, String, String, sWifiSSID, "")
    DefineGetterSetter(WifiKey, String, String, sWifiKey, "")

    DefineGetterSetter(SwapValidateAndCancel, bool, Bool, sSwapValidateAndCancel, true)

    DefineGetterSetter(AudioVolume, int, Int, sAudioVolume, 60)
    DefineGetterSetter(AudioOuput, String, String, sAudioOuput, "")

    DefineGetterSetter(MusicRemoteEnable, bool, Bool, sMusicDisableRemote, false)

    DefineGetterSetter(ScreenSaverTime, int, Int, sScreenSaverTime, 5)
    DefineGetterSetterEnum(ScreenSaverType, Screensaver, sScreenSaverType, Screensaver)
    DefineListGetterSetter(ScreenSaverSystemList, sScreenSaverSystemList, "")

    DefineGetterSetter(PopupHelp, int, Int, sPopupHelp, 10)
    DefineGetterSetter(PopupMusic, int, Int, sPopupMusic, 5)
    DefineGetterSetter(PopupNetplay, int, Int, sPopupNetplay, 8)

    DefineGetterSetter(ThemeCarousel, bool, Bool, sThemeCarousel, 1)
    DefineGetterSetter(ThemeTransition, String, String, sThemeTransition, "slide")
    DefineGetterSetter(ThemeFolder, String, String, sThemeFolder, "recalbox-next")

    DefineGetterSetterParameterized(ThemeColorSet    , String, String, sThemeGeneric, ".colorset", "")
    DefineGetterSetterParameterized(ThemeIconSet     , String, String, sThemeGeneric, ".iconset", "")
    DefineGetterSetterParameterized(ThemeMenuSet     , String, String, sThemeGeneric, ".menuset", "")
    DefineGetterSetterParameterized(ThemeSystemView  , String, String, sThemeGeneric, ".systemview", "")
    DefineGetterSetterParameterized(ThemeGamelistView, String, String, sThemeGeneric, ".gamelistview", "")
    DefineGetterSetterParameterized(ThemeGameClipView, String, String, sThemeGeneric, ".gameclipview", "")
    DefineGetterSetterParameterized(ThemeRegion      , String, String, sThemeGeneric, ".region", "")

    DefineGetterSetter(Brightness, int, Int, sBrightness, 7)
    DefineGetterSetter(Clock, bool, Bool, sClock, true)
    DefineGetterSetter(ShowHelp, bool, Bool, sShowHelp, true)
    DefineGetterSetter(ShowGameClipHelpItems, bool, Bool, sShowGameClipHelpItems, true)
    DefineGetterSetter(ShowGameClipClippingItem, bool, Bool, sShowGameClipClippingItem, true)
    DefineGetterSetter(QuickSystemSelect, bool, Bool, sQuickSystemSelect, true)
    DefineGetterSetter(FilterAdultGames, bool, Bool, sFilterAdultGames, true)
    DefineGetterSetter(FavoritesOnly, bool, Bool, sFavoritesOnly, false)
    DefineGetterSetter(ShowHidden, bool, Bool, sShowHidden, false)
    DefineGetterSetter(DisplayByFileName, bool, Bool, sDisplayByFileName, false)
    DefineGetterSetter(ShowOnlyLatestVersion, bool, Bool, sShowOnlyLatestVersion, false)
    DefineGetterSetter(HideNoGames, bool, Bool, sHideNoGames, false)

    DefineGetterSetter(FirstTimeUse, bool, Bool, sFirstTimeUse, true)

    DefineGetterSetter(SystemLanguage, String, String, sSystemLanguage, "en_US")
    DefineGetterSetter(SystemKbLayout, String, String, sSystemKbLayout, "us")
    DefineGetterSetter(SystemManagerEnabled, bool, Bool, sSystemManagerEnabled, false)

    DefineGetterSetter(Overclocking, String, String, sOverclocking, "none")
    DefineGetterSetter(Overscan, bool, Bool, sOverscan, false)

    DefineGetterSetter(KodiEnabled, bool, Bool, sKodiEnabled, false)
    DefineGetterSetter(KodiAtStartup, bool, Bool, sKodiAtStartup, false)
    DefineGetterSetter(KodiXButton, bool, Bool, sKodiXButton, false)

    DefineGetterSetterEnum(ScraperSource, ScraperType, sScraperSource, ScraperType)
    DefineGetterSetter(ScraperAuto, bool, Bool, sScraperAuto, true)

    DefineGetterSetter(RecalboxPrivateKey, String, String, sRecalboxPrivateKey, "")

    DefineGetterSetter(ScreenScraperLogin, String, String, sScreenScraperLogin, "")
    DefineGetterSetter(ScreenScraperPassword, String, String, sScreenScraperPassword, "")
    DefineGetterSetter(ScreenScraperWantMarquee, bool, Bool, sScreenScraperWantMarquee, false)
    DefineGetterSetter(ScreenScraperWantWheel, bool, Bool, sScreenScraperWantWheel, false)
    DefineGetterSetter(ScreenScraperWantManual, bool, Bool, sScreenScraperWantManual, false)
    DefineGetterSetter(ScreenScraperWantMaps, bool, Bool, sScreenScraperWantMaps, false)
    DefineGetterSetter(ScreenScraperWantP2K, bool, Bool, sScreenScraperWantP2K, false)

    DefineGetterSetter(NetplayEnabled, bool, Bool, sNetplayEnabled, false)
    DefineGetterSetter(NetplayLogin, String, String, sNetplayLogin, "")
    DefineGetterSetter(NetplayLobby, String, String, sNetplayLobby, "http://lobby.libretro.com/list/")
    DefineGetterSetter(NetplayPort, int, Int, sNetplayPort, sNetplayDefaultPort)
    DefineGetterSetterEnum(NetplayRelay, Relay, sNetplayRelay, Relay)

    DefineGetterSetter(RetroAchievementOnOff, bool, Bool, sRetroAchievementOnOff, false)
    DefineGetterSetter(RetroAchievementHardcore, bool, Bool, sRetroAchievementHardcore, false)
    DefineGetterSetter(RetroAchievementLogin, String, String, sRetroAchievementLogin, "")
    DefineGetterSetter(RetroAchievementPassword, String, String, sRetroAchievementPassword, "")

    DefineGetterSetter(StartupGamelistOnly, bool, Bool, sStartupGamelistOnly, false)
    DefineGetterSetter(StartupSelectedSystem, String, String, sStartupSelectedSystem, "")
    DefineGetterSetter(StartupStartOnGamelist, bool, Bool, sStartupStartOnGamelist, false)
    DefineGetterSetter(StartupHideSystemView, bool, Bool, sStartupHideSystemView, false)

    DefineGetterSetter(PowerSwitch, String, String, sPowerSwitch, "")

    DefineGetterSetter(GlobalRatio, String, String, sGlobalRatio, "auto")
    DefineGetterSetter(GlobalSmooth, bool, Bool, sGlobalSmooth, true)
    DefineGetterSetter(GlobalRecalboxOverlays, bool, Bool, sGlobalRecalboxOverlays, true)
    DefineGetterSetter(GlobalRewind, bool, Bool, sGlobalRewind, false)
    DefineGetterSetter(GlobalAutoSave, bool, Bool, sGlobalAutoSave, true)
    DefineGetterSetter(GlobalQuitTwice, bool, Bool, sGlobalQuitTwice, false)
    DefineGetterSetter(GlobalHidePreinstalled, bool, Bool, sGlobalHidePreinstalled, false)
    DefineGetterSetter(GlobalIntegerScale, bool, Bool, sGlobalIntegerScale, false)
    DefineGetterSetterEnum(GlobalSoftpatching, SoftPatching, sGlobalSoftpatching, SoftPatching)
    DefineGetterSetter(GlobalShaders, String, String, sGlobalShaders, "")
    DefineGetterSetter(GlobalShaderSet, String, String, sGlobalShaderSet, "none")
    DefineGetterSetter(GlobalShowFPS, bool, Bool, sGlobalShowFPS, false)
    DefineGetterSetter(GlobalInputDriver, String, String, sGlobalInputDriver, "auto")
    DefineGetterSetter(GlobalDemoDuration, int, Int, sGlobalDemoDuration, 90)
    DefineGetterSetter(GlobalDemoInfoScreen, int, Int, sGlobalDemoInfoScreen, 6)
    DefineGetterSetter(GlobalReduceLatency, bool, Bool, sGlobalReduceLatency, false)
    DefineGetterSetter(GlobalRunAhead, bool, Bool, sGlobalRunAhead, false)
    DefineGetterSetter(GlobalShowSaveStateBeforeRun, bool, Bool, sGlobalShowSaveStateBeforeRun, false)
    DefineGetterSetter(GlobalHDMode, bool, Bool, sGlobalHDMode, false)
    DefineGetterSetter(GlobalWidescreenMode, bool, Bool, sGlobalWidescreen, false)
    DefineGetterSetter(GlobalVulkanDriver, bool, Bool, sGlobalVulkanDriver, true)

    DefineGetterSetter(CollectionLastPlayed, bool, Bool, sCollectionLastPlayed, false)
    DefineGetterSetter(CollectionMultiplayer, bool, Bool, sCollectionMultiplayer, false)
    DefineGetterSetter(CollectionAllGames, bool, Bool, sCollectionAllGames, false)
    DefineGetterSetter(CollectionLightGun, bool, Bool, sCollectionLightGun, false)
    DefineGetterSetter(CollectionPorts, bool, Bool, sCollectionPorts, true)
    DefineGetterSetter(CollectionTate, bool, Bool, sCollectionTate, false)
    DefineListGetterSetter(CollectionGenre, sCollectionGenre, "")
    DefineGetterSetter(TateGameRotation, int, Int, sTateGameRotation, 0)
    DefineGetterSetter(TateOnly, bool, Bool, sTateOnly, false)

    DefineListGetterSetter(CollectionArcadeManufacturers, sCollectionArcadeManufacturers, "")
    DefineGetterSetter(CollectionArcade, bool, Bool, sCollectionArcade, false)
    DefineGetterSetter(CollectionArcadeNeogeo, bool, Bool, sCollectionArcadeNeogeo, true)
    DefineGetterSetter(CollectionArcadeHideOriginals, bool, Bool, sCollectionArcadeHideOriginals, true)
    DefineGetterSetter(CollectionArcadePosition, int, Int, sCollectionArcadePosition, 0)

    DefineGetterSetter(ArcadeUseDatabaseNames, bool, Bool, sArcadeUseDatabaseNames, true)
    DefineGetterSetter(ArcadeViewEnhanced, bool, Bool, sArcadeViewEnhanced, true)
    DefineGetterSetter(ArcadeViewHideBios, bool, Bool, sArcadeViewHideBios, false)
    DefineGetterSetter(ArcadeViewFoldClones, bool, Bool, sArcadeViewFoldClones, false)
    DefineGetterSetter(ArcadeViewHideNonWorking, bool, Bool, sArcadeViewHideNonWorking, false)

    DefineGetterSetter(UpdatesEnabled, bool, Bool, sUpdatesEnabled, false)
    DefineGetterSetter(UpdatesType, String, String, sUpdatesType, "stable")

    DefineGetterSetter(EmulationstationVideoMode, String, String, sEsVideoMode, "")
    DefineGetterSetter(GlobalVideoMode, String, String, sGlobalVideoMode, "")
    DefineGetterSetter(KodiVideoMode, String, String, sKodiVideoMode, "")
    DefineGetterSetter(ESForce43, bool, Bool, sESForce43, false)
    DefineGetterSetter(SplashEnabled, bool, Bool, sSplashEnabled, false)


    DefineGetterSetter(BatteryHidden, bool, Bool, sBatteryHidden, false)
    DefineGetterSetter(PadOSD, bool, Bool, sPadOSD, false)
    DefineGetterSetterEnum(PadOSDType, PadOSDType, sPadOSDType, PadOSDType)
    DefineGetterSetter(AutoPairOnBoot, bool, Bool, sAutoPairOnBoot, true)

    DefineGetterSetter(SuperGameBoy, String, String, sSuperGameBoyOption, "gb")
    DefineGetterSetter(Experimental, bool, Bool, sExperimental, GetUpdatesType() != "stable")

    DefineGetterSetter(AutorunEnabled, bool, Bool, sAutorunEnabled, false)
    DefineGetterSetter(AutorunSystemUUID, String, String, sAutorunSystemUUID, "")
    DefineGetterSetter(AutorunGamePath, String, String, sAutorunGamePath, "")

    /*
     * System
     */

    DefineSystemGetterSetterDeclaration(Emulator, String, String, sSystemEmulator)
    DefineSystemGetterSetterDeclaration(Core, String, String, sSystemCore)
    DefineSystemGetterSetterDeclaration(Ratio, String, String, sSystemRatio)
    DefineSystemGetterSetterDeclaration(Smooth, bool, Bool, sSystemSmooth)
    DefineSystemGetterSetterDeclaration(Rewind, bool, Bool, sSystemRewind)
    DefineSystemGetterSetterDeclaration(AutoSave, bool, Bool, sSystemAutoSave)
    DefineSystemGetterSetterDeclaration(Shaders, String, String, sSystemShaders)
    DefineSystemGetterSetterDeclaration(ShaderSet, String, String, sSystemShaderSet)
    DefineSystemGetterSetterDeclaration(Ignore, bool, Bool, sSystemIgnore)
    DefineSystemGetterSetterDeclaration(DemoInclude, bool, Bool, sSystemDemoInclude)
    DefineSystemGetterSetterDeclaration(DemoDuration, int, Int, sSystemDemoDuration)
    DefineSystemGetterSetterDeclaration(VideoMode, String, String, sSystemVideoMode)

    DefineEmulationStationSystemGetterSetterDeclaration(FilterAdult, bool, Bool, sSystemFilterAdult)
    DefineEmulationStationSystemGetterSetterDeclaration(FlatFolders, bool, Bool, sSystemFlatFolders)
    DefineEmulationStationSystemGetterSetterNumericEnumDeclaration(Sort, FileSorts::Sorts)
    DefineEmulationStationSystemGetterSetterNumericEnumDeclaration(RegionFilter, Regions::GameRegions)

    #undef DefineGetterSetter
    #undef DefineListGetterSetter
    #undef DefineGetterSetterParameterized

    /*
     * Direct Implementations - Pads
     */

    [[nodiscard]] String GetPad(int index) const { return AsString(String(sPadHeader).Append(index), ""); }
    RecalboxConf& SetPad(int index, const String& padid) { SetString(String(sPadHeader).Append(index), padid); return *this; }

    /*
     * System keys
     */

    static constexpr const char* sSystemEmulator             = "emulator";
    static constexpr const char* sSystemCore                 = "core";
    static constexpr const char* sSystemRatio                = "ratio";
    static constexpr const char* sSystemSmooth               = "smooth";
    static constexpr const char* sSystemRewind               = "rewind";
    static constexpr const char* sSystemAutoSave             = "autosave";
    static constexpr const char* sSystemIgnore               = "ignore";
    static constexpr const char* sSystemShaders              = "shaders";
    static constexpr const char* sSystemShaderSet            = "shaderset";
    static constexpr const char* sSystemFilterAdult          = "filteradultgames";
    static constexpr const char* sDisplayByFileName          = "displaybyfilename";
    static constexpr const char* sSystemRegionFilter         = "regionfilter";
    static constexpr const char* sSystemFlatFolders          = "flatfolders";
    static constexpr const char* sSystemSort                 = "sort";
    static constexpr const char* sSystemDemoInclude          = "demo.include";
    static constexpr const char* sSystemDemoDuration         = "demo.duration";
    static constexpr const char* sSystemVideoMode            = "videomode";

    /*
     * Collection Keys
     */

    static constexpr const char* sCollectionHide             = "hide";
    static constexpr const char* sCollectionTheme            = "theme";
    static constexpr const char* sCollectionLimit            = "limit";

    /*
     * Key headers
     */

    static constexpr const char* sCollectionHeader           = "emulationstation.collection";
    static constexpr const char* sArcadeCollectionHeader     = "emulationstation.collection.arcade";

    /*
     * Keys
     */

    static constexpr const char* sExperimental               = "global.experimental";

    static constexpr const char* sGlobalRatio                = "global.ratio";
    static constexpr const char* sGlobalSmooth               = "global.smooth";
    static constexpr const char* sGlobalRecalboxOverlays     = "global.recalboxoverlays";
    static constexpr const char* sGlobalRewind               = "global.rewind";
    static constexpr const char* sGlobalAutoSave             = "global.autosave";
    static constexpr const char* sGlobalSoftpatching         = "global.softpatching";
    static constexpr const char* sGlobalShaders              = "global.shaders";
    static constexpr const char* sGlobalShaderSet            = "global.shaderset";
    static constexpr const char* sGlobalQuitTwice            = "global.quitpresstwice";
    static constexpr const char* sGlobalHidePreinstalled     = "global.hidepreinstalledgames";
    static constexpr const char* sGlobalIntegerScale         = "global.integerscale";
    static constexpr const char* sGlobalShowFPS              = "global.showfps";
    static constexpr const char* sGlobalDemoDuration         = "global.demo.duration";
    static constexpr const char* sGlobalDemoInfoScreen       = "global.demo.infoscreenduration";
    static constexpr const char* sGlobalReduceLatency        = "global.reducelatency";
    static constexpr const char* sGlobalRunAhead             = "global.runahead";
    static constexpr const char* sGlobalHDMode               = "global.hdmode";
    static constexpr const char* sGlobalWidescreen           = "global.widescreenmode";
    static constexpr const char* sGlobalShowSaveStateBeforeRun = "global.show.savestate.before.run";
    static constexpr const char* sSplashEnabled              = "system.splash.enabled";
    static constexpr const char* sGlobalVulkanDriver         = "global.vulkandriver";

    static constexpr const char* sGlobalInputDriver          = "global.inputdriver";

    static constexpr const char* sHostname                   = "system.hostname";

    static constexpr const char* sWifiEnabled                = "wifi.enabled";
    static constexpr const char* sWifiSSID                   = "wifi.ssid";
    static constexpr const char* sWifiKey                    = "wifi.key";

    static constexpr const char* sSwapValidateAndCancel      = "controllers.swapvalidateandcancel";

    static constexpr const char* sAudioVolume                = "audio.volume";
    static constexpr const char* sAudioOptions               = "audio.mode";
    static constexpr const char* sAudioOuput                 = "audio.device";

    static constexpr const char* sMusicDisableRemote         = "music.remoteplaylist.enable";

    static constexpr const char* sScreenSaverTime            = "emulationstation.screensaver.time";
    static constexpr const char* sScreenSaverType            = "emulationstation.screensaver.type";
    static constexpr const char* sScreenSaverSystemList      = "global.demo.systemlist";

    static constexpr const char* sPopupHelp                  = "emulationstation.popoup.help";
    static constexpr const char* sPopupMusic                 = "emulationstation.popoup.music";
    static constexpr const char* sPopupNetplay               = "emulationstation.popoup.netplay";

    static constexpr const char* sThemeGeneric               = "emulationstation.theme.";
    static constexpr const char* sThemeCarousel              = "emulationstation.theme.carousel";
    static constexpr const char* sThemeTransition            = "emulationstation.theme.transition";
    static constexpr const char* sThemeFolder                = "emulationstation.theme.folder";

    static constexpr const char* sBrightness                 = "emulationstation.brightness";
    static constexpr const char* sClock                      = "emulationstation.clock";
    static constexpr const char* sShowHelp                   = "emulationstation.showhelp";
    static constexpr const char* sShowGameClipHelpItems      = "emulationstation.showgamecliphelpitems";
    static constexpr const char* sShowGameClipClippingItem   = "emulationstation.showgameclipclippingitem";
    static constexpr const char* sQuickSystemSelect          = "emulationstation.quicksystemselect";
    static constexpr const char* sFilterAdultGames           = "emulationstation.filteradultgames";
    static constexpr const char* sFavoritesOnly              = "emulationstation.favoritesonly";
    static constexpr const char* sShowHidden                 = "emulationstation.showhidden";
    static constexpr const char* sShowOnlyLatestVersion      = "emulationstation.showonlylatestversion";
    static constexpr const char* sHideNoGames                = "emulationstation.hidenogames";

    static constexpr const char* sSystemSorting              = "emulationstation.systemsorting";

    static constexpr const char* sBatteryHidden              = "emulationstation.battery.hidden";
    static constexpr const char* sPadOSD                     = "emulationstation.pads.osd";
    static constexpr const char* sPadOSDType                 = "emulationstation.pads.osd.type";
    static constexpr const char* sAutoPairOnBoot             = "controllers.bluetooth.autopaironboot";

    static constexpr const char* sEsVideoMode                = "system.es.videomode";
    static constexpr const char* sGlobalVideoMode            = "global.videomode";
    static constexpr const char* sKodiVideoMode              = "kodi.videomode";
    static constexpr const char* sESForce43                  = "system.es.force43";

    static constexpr const char* sFirstTimeUse               = "system.firsttimeuse";
    static constexpr const char* sSystemLanguage             = "system.language";
    static constexpr const char* sSystemKbLayout             = "system.kblayout";
    static constexpr const char* sSystemManagerEnabled       = "system.manager.enabled";

    static constexpr const char* sOverclocking               = "system.overclocking";
    static constexpr const char* sOverscan                   = "system.overscan";

    static constexpr const char* sPowerSwitch                = "system.power.switch";

    static constexpr const char* sKodiEnabled                = "kodi.enabled";
    static constexpr const char* sKodiAtStartup              = "kodi.atstartup";
    static constexpr const char* sKodiXButton                = "kodi.xbutton";

    static constexpr const char* sScraperSource              = "scraper.source";
    static constexpr const char* sScraperAuto                = "scraper.auto";
    static constexpr const char* sScraperGetNameFrom         = "scraper.getnamefrom";

    static constexpr const char* sRecalboxPrivateKey         = "patron.privatekey";

    static constexpr const char* sScreenScraperLogin         = "scraper.screenscraper.user";
    static constexpr const char* sScreenScraperPassword      = "scraper.screenscraper.password";
    static constexpr const char* sScreenScraperRegionPriority= "scraper.screenscraper.regionPriority";
    static constexpr const char* sScreenScraperRegion        = "scraper.screenscraper.region";
    static constexpr const char* sScreenScraperLanguage      = "scraper.screenscraper.language";
    static constexpr const char* sScreenScraperMainMedia     = "scraper.screenscraper.media";
    static constexpr const char* sScreenScraperThumbnail     = "scraper.screenscraper.thumbnail";
    static constexpr const char* sScreenScraperVideo         = "scraper.screenscraper.video";
    static constexpr const char* sScreenScraperWantMarquee   = "scraper.screenscraper.marquee";
    static constexpr const char* sScreenScraperWantWheel     = "scraper.screenscraper.wheel";
    static constexpr const char* sScreenScraperWantManual    = "scraper.screenscraper.manual";
    static constexpr const char* sScreenScraperWantMaps      = "scraper.screenscraper.maps";
    static constexpr const char* sScreenScraperWantP2K       = "scraper.screenscraper.p2k";

    static constexpr const char* sNetplayEnabled             = "global.netplay.active";
    static constexpr const char* sNetplayLogin               = "global.netplay.nickname";
    static constexpr const char* sNetplayLobby               = "global.netplay.lobby";
    static constexpr const char* sNetplayPort                = "global.netplay.port";
    static constexpr const char* sNetplayRelay               = "global.netplay.relay";

    static constexpr const char* sRetroAchievementOnOff      = "global.retroachievements";
    static constexpr const char* sRetroAchievementHardcore   = "global.retroachievements.hardcore";
    static constexpr const char* sRetroAchievementLogin      = "global.retroachievements.username";
    static constexpr const char* sRetroAchievementPassword   = "global.retroachievements.password";

    static constexpr const char* sStartupGamelistOnly        = "emulationstation.gamelistonly";
    static constexpr const char* sStartupSelectedSystem      = "emulationstation.selectedsystem";
    static constexpr const char* sStartupStartOnGamelist     = "emulationstation.bootongamelist";
    static constexpr const char* sStartupHideSystemView      = "emulationstation.hidesystemview";

    static constexpr const char* sMenuType                   = "emulationstation.menu";
    static constexpr const char* sHideSystemView             = "emulationstation.hidesystemview";
    static constexpr const char* sBootOnGamelist             = "emulationstation.bootongamelist";
    static constexpr const char* sForceBasicGamelistView     = "emulationstation.forcebasicgamelistview";

    static constexpr const char* sCollectionLastPlayed       = "emulationstation.collection.lastplayed";
    static constexpr const char* sCollectionMultiplayer      = "emulationstation.collection.multiplayer";
    static constexpr const char* sCollectionAllGames         = "emulationstation.collection.allgames";
    static constexpr const char* sCollectionLightGun         = "emulationstation.collection.lightgun";
    static constexpr const char* sCollectionPorts            = "emulationstation.collection.ports";
    static constexpr const char* sCollectionTate             = "emulationstation.collection.tate";
    static constexpr const char* sCollectionGenre            = "emulationstation.collection.genre";
    static constexpr const char* sTateGameRotation           = "tate.gamerotation";
    static constexpr const char* sTateOnly                   = "emulationstation.tateonly";

    static constexpr const char* sCollectionArcade           = "emulationstation.virtualarcade";
    static constexpr const char* sCollectionArcadeManufacturers = "emulationstation.virtualarcade.manufacturers";
    static constexpr const char* sCollectionArcadeNeogeo     = "emulationstation.virtualarcade.includeneogeo";
    static constexpr const char* sCollectionArcadeHideOriginals = "emulationstation.virtualarcade.hideoriginals";
    static constexpr const char* sCollectionArcadePosition   = "emulationstation.virtualarcade.position";

    static constexpr const char* sArcadeViewEnhanced         = "emulationstation.arcade.view.enhanced";
    static constexpr const char* sArcadeViewFoldClones       = "emulationstation.arcade.view.hideclones";
    static constexpr const char* sArcadeViewHideBios         = "emulationstation.arcade.view.hidebios";
    static constexpr const char* sArcadeViewHideNonWorking   = "emulationstation.arcade.view.hidenonworking";
    static constexpr const char* sArcadeUseDatabaseNames     = "emulationstation.arcade.usedatabasenames";

    static constexpr const char* sAutorunEnabled             = "autorun.enabled";
    static constexpr const char* sAutorunSystemUUID          = "autorun.uuid";
    static constexpr const char* sAutorunGamePath            = "autorun.path";

    static constexpr const char* sUpdatesEnabled             = "updates.enabled";
    static constexpr const char* sUpdatesType                = "updates.type";

    static constexpr const char* sPadHeader                  = "emulationstation.pad";

    static constexpr const char* sDebugLogs                  = "emulationstation.debuglogs";

    static constexpr const int sNetplayDefaultPort           = 55435;

    static constexpr const char* sSuperGameBoyOption         = "gb.supergameboy";

    static constexpr const char* sArcadeSystemHiddenManufacturers  = "hiddendrivers";

  private:
    HashMap<String, Array<IRecalboxConfChanged*>> mWatchers;

    /*
     * Culture
     */

    static String GetLanguage();
    static String GetCountry();

    /*
     * Enumeration getter/setter
     */

    static SoftPatching SoftPatchingFromString(const String& menu);
    static const String& SoftPatchingFromEnum(SoftPatching screensaver);
    static Screensaver ScreensaverFromString(const String& menu);
    static const String& ScreensaverFromEnum(Screensaver screensaver);
    static Menu MenuFromString(const String& menu);
    static const String& MenuFromEnum(Menu menu);
    static Relay RelayFromString(const String& relay);
    static const String& RelayFromEnum(Relay relay);
    static SystemSorting SystemSortingFromString(const String& systemSorting);
    static const String& SystemSortingFromEnum(SystemSorting systemSorting);
    static ScraperType ScraperTypeFromString(const String& menu);
    static const String& ScraperTypeFromEnum(ScraperType type);
    static PadOSDType PadOSDTypeFromString(const String& pad);
    static const String& PadOSDTypeFromEnum(PadOSDType type);
};
