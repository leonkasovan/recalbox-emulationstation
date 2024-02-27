#include "RecalboxConf.h"
#include <utils/Files.h>
#include <usernotifications/NotificationManager.h>

static Path recalboxConfFile("/recalbox/share/system/recalbox.conf");
static Path recalboxConfFileInit("/recalbox/share_init/system/recalbox.conf");

RecalboxConf::RecalboxConf()
  : IniFile(recalboxConfFile, recalboxConfFileInit, false, true),
    StaticLifeCycleControler<RecalboxConf>("RecalboxConf")
{
}

void RecalboxConf::OnSave() const
{
  NotificationManager::Instance().Notify(Notification::ConfigurationChanged, recalboxConfFile.ToString());
}

String RecalboxConf::GetLanguage()
{
  String locale = RecalboxConf::Instance().GetSystemLanguage().LowerCase();
  return (locale.length() == 5) ? locale.SubString(0, 2) : "en";
}

String RecalboxConf::GetCountry()
{
  String locale = RecalboxConf::Instance().GetSystemLanguage().LowerCase();
  return (locale.length() == 5) ? locale.SubString(3, 2) : "us";
}

RecalboxConf::SoftPatching RecalboxConf::SoftPatchingFromString(const String& softpatching)
{
  if (softpatching == "auto") return SoftPatching::Auto;
  if (softpatching == "select") return SoftPatching::Select;
  if (softpatching == "launchLast") return SoftPatching::LaunchLast;

  return SoftPatching::Disable;
}

const String& RecalboxConf::SoftPatchingFromEnum(SoftPatching softpatching)
{
  switch(softpatching)
  {
    case SoftPatching::Auto: { static String s("auto"); return s; }
    case SoftPatching::Select: { static String s("select"); return s; }
    case SoftPatching::LaunchLast: { static String s("launchLast"); return s; }
    case SoftPatching::Disable:
    default: break;
  }
  static String s("disable");
  return s;
}

RecalboxConf::Screensaver RecalboxConf::ScreensaverFromString(const String& screensaver)
{
  if (screensaver == "black") return Screensaver::Black;
  if (screensaver == "demo") return Screensaver::Demo;
  if (screensaver == "gameclip") return Screensaver::Gameclip;
  if (screensaver == "suspend") return Screensaver::Suspend;

  return Screensaver::Dim;
}

const String& RecalboxConf::ScreensaverFromEnum(RecalboxConf::Screensaver screensaver)
{
  switch(screensaver)
  {
    case Screensaver::Demo: { static String s("demo"); return s; }
    case Screensaver::Gameclip: { static String s("gameclip"); return s; }
    case Screensaver::Suspend: { static String s("suspend"); return s; }
    case Screensaver::Black: { static String s("black"); return s; }
    case Screensaver::Dim:
    default: break;
  }
  static String s("dim");
  return s;
}

RecalboxConf::Menu RecalboxConf::MenuFromString(const String& menu)
{
  if (menu == "bartop") return Menu::Bartop;
  if (menu == "none") return Menu::None;
  return Menu::Default;
}

const String& RecalboxConf::MenuFromEnum(RecalboxConf::Menu menu)
{
  switch (menu)
  {
    case Menu::Bartop: { static String sBartop = "bartop"; return sBartop; }
    case Menu::None:   { static String sNone = "none"; return sNone; }
    case Menu::Default: default: break;
  }
  static String sDefault = "default";
  return sDefault;
}

RecalboxConf::Relay RecalboxConf::RelayFromString(const String& relay)
{
  if (relay == "nyc") return Relay::NewYork;
  if (relay == "madrid") return Relay::Madrid;
  if (relay == "montreal") return Relay::Montreal;
  if (relay == "saopaulo") return Relay::Saopaulo;
  return Relay::None;
}

const String& RecalboxConf::RelayFromEnum(RecalboxConf::Relay relay)
{
  switch (relay)
  {
    case Relay::NewYork:  { static String sNewYork = "nyc"; return sNewYork; }
    case Relay::Madrid:   { static String sMadrid = "madrid"; return sMadrid; }
    case Relay::Montreal: { static String sMontreal = "montreal"; return sMontreal; }
    case Relay::Saopaulo: { static String sSauPaulo = "saopaulo"; return sSauPaulo; }
    case Relay::None: default: break;
  }
  static String sDefault = "none";
  return sDefault;
}

RecalboxConf::PadOSDType RecalboxConf::PadOSDTypeFromString(const String& pad)
{
  if (pad == "megadrive") return PadOSDType::MD;
  if (pad == "xbox") return PadOSDType::XBox;
  if (pad == "playstation") return PadOSDType::PSX;
  if (pad == "nintendo64") return PadOSDType::N64;
  if (pad == "dreamcast") return PadOSDType::DC;
  return PadOSDType::Snes;
}

const String& RecalboxConf::PadOSDTypeFromEnum(PadOSDType type)
{
  switch(type)
  {
    case PadOSDType::MD: { static String s = "megadrive"; return s; }
    case PadOSDType::XBox: { static String s = "xbox"; return s; }
    case PadOSDType::PSX: { static String s = "playstation"; return s; }
    case PadOSDType::N64: { static String s = "nintendo64"; return s; }
    case PadOSDType::DC: { static String s = "dreamcast"; return s; }
    case PadOSDType::Snes:
    default: break;
  }
  static String sDefault = "snes";
  return sDefault;
}

SystemSorting RecalboxConf::SystemSortingFromString(const String& systemSorting)
{
  if (systemSorting == "default")                        return SystemSorting::Default;
  if (systemSorting == "name")                           return SystemSorting::Name;
  if (systemSorting == "releasedate")                    return SystemSorting::ReleaseDate;
  if (systemSorting == "1type2name")                     return SystemSorting::SystemTypeThenName;
  if (systemSorting == "1type2releasedate")              return SystemSorting::SystemTypeThenReleaseDate;
  if (systemSorting == "1manufacturer2name")             return SystemSorting::ManufacturerThenName;
  if (systemSorting == "1manufacturer2releasedate")      return SystemSorting::ManufacturerThenReleaseData;
  if (systemSorting == "1type2manufacturer3name")        return SystemSorting::SystemTypeThenManufacturerThenName;
  if (systemSorting == "1type2manufacturer3releasedate") return SystemSorting::SystemTypeThenManufacturerThenReleasdeDate;
  return SystemSorting::Default;
}

const String& RecalboxConf::SystemSortingFromEnum(SystemSorting systemSorting)
{
  switch (systemSorting)
  {
    case SystemSorting::Default:                                    { static String string("default"); return string; }
    case SystemSorting::Name:                                       { static String string("name"); return string; }
    case SystemSorting::ReleaseDate:                                { static String string("releasedate"); return string; }
    case SystemSorting::SystemTypeThenName:                         { static String string("1type2name"); return string; }
    case SystemSorting::SystemTypeThenReleaseDate:                  { static String string("1type2releasedate"); return string; }
    case SystemSorting::ManufacturerThenName:                       { static String string("1manufacturer2name"); return string; }
    case SystemSorting::ManufacturerThenReleaseData:                { static String string("1manufacturer2releasedate"); return string; }
    case SystemSorting::SystemTypeThenManufacturerThenName:         { static String string("1type2manufacturer3name"); return string; }
    case SystemSorting::SystemTypeThenManufacturerThenReleasdeDate: { static String string("1type2manufacturer3releasedate"); return string; }
  }
  static String sDefault = "none";
  return sDefault;
}

ScraperType RecalboxConf::ScraperTypeFromString(const String& menu)
{
  if (menu == "ScreenScraper") return ScraperType::ScreenScraper;
  if (menu == "Recalbox")      return ScraperType::Recalbox;
  if (menu == "TheGameDB")     return ScraperType::TheGameDB;
  return ScraperType::ScreenScraper;
}

const String& RecalboxConf::ScraperTypeFromEnum(ScraperType type)
{
  static String defaultString("ScreenScraper");
  switch(type)
  {
    case ScraperType::ScreenScraper: { return defaultString; }
    case ScraperType::Recalbox: { static String string("Recalbox"); return string; }
    case ScraperType::TheGameDB: { static String string("TheGameDB"); return string; }
    default: break;
  }
  return defaultString;
}

DefineSystemGetterSetterImplementation(Emulator, String, String, sSystemEmulator, "")
DefineSystemGetterSetterImplementation(Core, String, String, sSystemCore, "")
DefineSystemGetterSetterImplementation(Ratio, String, String, sSystemRatio, GetGlobalRatio())
DefineSystemGetterSetterImplementation(Smooth, bool, Bool, sSystemSmooth, GetGlobalSmooth())
DefineSystemGetterSetterImplementation(Rewind, bool, Bool, sSystemRewind, GetGlobalRewind())
DefineSystemGetterSetterImplementation(AutoSave, bool, Bool, sSystemAutoSave, GetGlobalAutoSave())
DefineSystemGetterSetterImplementation(Shaders, String, String, sSystemShaders, GetGlobalShaders())
DefineSystemGetterSetterImplementation(ShaderSet, String, String, sSystemShaderSet, GetGlobalShaderSet())
DefineSystemGetterSetterImplementation(Ignore, bool, Bool, sSystemIgnore, false)
DefineSystemGetterSetterImplementation(DemoInclude, bool, Bool, sSystemDemoInclude, false)
DefineSystemGetterSetterImplementation(DemoDuration, int, Int, sSystemDemoDuration, GetGlobalDemoDuration())
DefineSystemGetterSetterImplementation(VideoMode, String, String, sSystemVideoMode, GetGlobalVideoMode())

DefineEmulationStationSystemGetterSetterImplementation(FilterAdult, bool, Bool, sSystemFilterAdult, GetFilterAdultGames())
DefineEmulationStationSystemGetterSetterImplementation(FlatFolders, bool, Bool, sSystemFlatFolders, false)
DefineEmulationStationSystemGetterSetterNumericEnumImplementation(Sort, FileSorts::Sorts, sSystemSort, FileSorts::Sorts::FileNameAscending)
DefineEmulationStationSystemGetterSetterNumericEnumImplementation(RegionFilter, Regions::GameRegions, sSystemRegionFilter, Regions::GameRegions::Unknown)

DefineEmulationStationSystemListGetterSetterImplementation(ArcadeSystemHiddenManufacturers, sArcadeSystemHiddenManufacturers, "")

void RecalboxConf::Watch(const String& key, IRecalboxConfChanged& callback)
{
  mWatchers[key].Add(&callback);
}

