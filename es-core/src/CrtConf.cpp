//
// Created by bkg2k on 11/01/2022.
//
// As part of the RECALBOX Project
// http://www.recalbox.com
//

#include <CrtConf.h>
#include <usernotifications/NotificationManager.h>

static Path crtConfFile("/boot/crt/recalbox-crt-options.cfg");

CrtConf::CrtConf()
  : IniFile(crtConfFile, true),
    StaticLifeCycleControler<CrtConf>("CrtConf")
{
}

void CrtConf::OnSave()
{
  NotificationManager::Instance().Notify(Notification::ConfigurationChanged, crtConfFile.ToString());
}

CrtAdapterType CrtConf::CrtAdapterFromString(const String& adapter)
{
  String foundAdapter = adapter;
  if(foundAdapter.empty())
  {
    foundAdapter = RecalboxConf::Instance().AsString("system.crt");
  }
  if (foundAdapter == "recalboxrgbdual") return CrtAdapterType::RGBDual;
  if (foundAdapter == "pi2scart"       ) return CrtAdapterType::Pi2Scart;
  if (foundAdapter == "rgbpi"          ) return CrtAdapterType::RGBPi;
  if (foundAdapter == "vga666"         ) return CrtAdapterType::Vga666;
  return CrtAdapterType::None;
}

const String& CrtConf::CrtAdapterFromEnum(CrtAdapterType adapter)
{
  switch(adapter)
  {
    case CrtAdapterType::RGBDual:  { static String adapterString("recalboxrgbdual"); return adapterString; }
    case CrtAdapterType::Pi2Scart: { static String adapterString("pi2scart"); return adapterString; }
    case CrtAdapterType::RGBPi:    { static String adapterString("rgbpi"); return adapterString; }
    case CrtAdapterType::Vga666:   { static String adapterString("vga666"); return adapterString; }
    case CrtAdapterType::None:
    default: break;
  }
  static String sDefault;
  return sDefault;
}

CrtResolution CrtConf::CrtResolutionFromString(const String& menu)
{
  if (menu == "p1920x224") return CrtResolution::r224p;
  if (menu == "p1920x240") return CrtResolution::r240p;
  if (menu == "p320x240") return CrtResolution::r320x240p;
  if (menu == "p1920x288") return CrtResolution::r288p;
  if (menu == "p384x288") return CrtResolution::r384x288p;
  if (menu == "i640x480") return CrtResolution::r480i;
  if (menu == "i768x576") return CrtResolution::r576i;
  if (menu == "p640x480") return CrtResolution::r480p;
  if (menu == "p1920x240at120") return CrtResolution::r240p120Hz;
  return CrtResolution::rNone;
}

const String& CrtConf::CrtResolutionFromEnum(CrtResolution type)
{
  switch(type)
  {
    case CrtResolution::r224p: { static String result("p1920x224"); return result; }
    case CrtResolution::r240p: { static String result("p1920x240"); return result; }
    case CrtResolution::r320x240p: { static String result("p320x240"); return result; }
    case CrtResolution::r288p: { static String result("p1920x288"); return result; }
    case CrtResolution::r384x288p: { static String result("p384x288"); return result; }
    case CrtResolution::r480i: { static String result("i640x480"); return result; }
    case CrtResolution::r576i: { static String result("i768x576"); return result; }
    case CrtResolution::r480p: { static String result("p640x480"); return result; }
    case CrtResolution::r240p120Hz: { static String result("p1920x240at120"); return result; }
    case CrtResolution::_rCount:
    case CrtResolution::rNone:
    default: break;
  }
  static String result("None");
  return result;
}

DefineCrtModeOffsetImplementation(VerticalOffset, int, Int, sVerticalOffset, 0)
DefineCrtModeOffsetImplementation(HorizontalOffset, int, Int, sHorizontalOffset, 0)
DefineCrtViewportImplementation(Width, int, Int, sWidth, 0)


