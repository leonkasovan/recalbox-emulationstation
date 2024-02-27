//
// Created by bkg2k on 07/06/23.
//

#include "ArcadeVirtualSystems.h"
#include "utils/storage/HashMap.h"
#include <utils/String.h>

const String::List& ArcadeVirtualSystems::GetVirtualArcadeSystemList()
{
  static String::List sList
  {
    "acclaim",
    "atari",
    "atlus",
    "banpresto",
    "capcom",
    "capcom\\cps1",
    "capcom\\cps2",
    "capcom\\cps3",
    "cave",
    "data\u00a0east",
    "exidy",
    "igs",
    "irem",
    "itech",
    "jaleco",
    "kaneko",
    "konami",
    "midway",
    "mitchell",
    "namco",
    "neogeo",
    "nichibutsu",
    "nintendo",
    "nmk",
    "sammy",
    "sega",
    "seibu",
    "seta",
    "snk",
    "taito",
    "technos",
    "tecmo",
    "toaplan",
    "visco",
  };

  return sList;
}

const std::vector<std::pair<String, String>>& ArcadeVirtualSystems::GetVirtualArcadeSystemListExtended()
{
  static std::vector<std::pair<String, String>> sList
    {
      { "acclaim",        "Acclaim" },
      { "atari",          "Atari" },
      { "atlus",          "Atlus" },
      { "banpresto",      "BanPresto" },
      { "capcom",         "Capcom" },
      { "capcom\\cps1",   "Camcom CPS1" },
      { "capcom\\cps2",   "Capcom CPS2" },
      { "capcom\\cps3",   "Capcom CPS3" },
      { "cave",           "CAVE" },
      { "data\u00a0east", "Data East" },
      { "exidy",          "Exidy" },
      { "igs",            "International Games System (I.G.S)" },
      { "irem",           "IREM Software Engineering" },
      { "itech",          "Incredible Technologies" },
      { "jaleco",         "Jaleco" },
      { "kaneko",         "Kaneko" },
      { "konami",         "Konami" },
      { "midway",         "Midway" },
      { "mitchell",       "Mitchell" },
      { "namco",          "NAMCO" },
      { "neogeo",         "Neo-Geo" },
      { "nichibutsu",     "Nichibutsu" },
      { "nintendo",       "Nintendo" },
      { "nmk",            "NML" },
      { "sammy",          "Sammy" },
      { "sega",           "SEGA" },
      { "seibu",          "Seibu" },
      { "seta",           "SETA" },
      { "snk",            "SNK" },
      { "taito",          "TAITO" },
      { "technos",        "Technōs" },
      { "tecmo",          "TECMO" },
      { "toaplan",        "TOAPLAN" },
      { "visco",          "VISCO" },
    };

  return sList;
}

/*String ArcadeVirtualSystems::GetRealName(const String& driverName)
{
  static HashMap<String, String> sList
  {
    { "f32", "F2 System" },
    { "funworld", "Fun World" },
    { "fuuki", "Fuuki" },
    { "gaelco", "Gaelco" },
    { "gameplan", "Game Plan" },
    { "gametron", "Game-A-Tron" },
    { "gottlieb", "Gottlieb" },
    { "ice", "ICE" },
    { "igs", "IGS" },
    { "igt", "IGT - International Game Technology" },
    { "irem", "Irem" },
    { "itech", "Incredible Technologies" },
    { "jaleco", "Jaleco" },
    { "jpm", "JPM" },
    { "kaneko", "Kaneko" },
    { "kiwako", "Kiwako" },
    { "konami", "Konami" },
    { "matic", "Eletro Matic Equipamentos Eletromecânicos" },
    { "maygay", "Maygay" },
    { "meadows", "Meadows Games, Inc." },
    { "merit", "Merit" },
    { "metro", "Metro" },
    { "midcoin", "Midcoin" },
    { "midw8080", "Midway" },
    { "midway", "Midway Games" },
    { "miltonbradley", "Roy Abel & Associates" },
    { "mr", "Model Racing" },
    { "namco", "Namco" },
    { "neogeo", "SNK NeoGeo" },
    { "nichibutsu", "Nichibutsu" },
    { "nintendo", "Nintendo" },
    { "nix", "NIX" },
    { "nmk", "NMK" },
    { "olympia", "Olympia" },
    { "omori", "Omori Electric Co., Ltd." },
    { "orca", "Orca" },
    { "pacific", "Pacific Novelty" },
    { "philips", "Philips" },
    { "playmark", "Playmark" },
    { "promat", "Promat" },
    { "psikyo", "Psikyo" },
    { "ramtek", "Ramtek" },
    { "rare", "Rare" },
    { "sanritsu", "Sanritsu" },
    { "sega", "Sega" },
    { "seibu", "Seibu Kaihatsu" },
    { "seta", "Seta" },
    { "sigma", "Sigma" },
    { "skeleton", "Inder" },
    { "snk", "SNK" },
    { "stern", "Stern Electronics" },
    { "subsino", "Subsino" },
    { "suna", "SunA" },
    { "sunelectronics", "Sun Electronics" },
    { "taito", "Taito" },
    { "tatsumi", "Tatsumi" },
    { "tch", "TCH" },
    { "tecfri", "Tecfri" },
    { "technos", "Technos Japan" },
    { "tecmo", "Tecmo" },
    { "thepit", "The Pit - Taito" },
    { "toaplan", "Toaplan" },
    { "unico", "Unico" },
    { "universal", "Universal" },
    { "upl", "UPL" },
    { "valadon", "Valadon Automation" },
    { "venture", "Venture Line" },
    { "vsystem", "V-System Co." },
    { "wing", "Wing" },
    { "yunsung", "Yun Sung" },
    { "zaccaria", "Zaccaria" },
  };

  String* result = sList.try_get(driverName);
  if (result != nullptr) return *result;

  // Check separator and try to isolate constructor/system
  // The return "Constructor name - SYSTEM"
  if (driverName.Contains('/'))
    if (int separator = driverName.Find('/'); separator > 0)
      if (result = sList.try_get(driverName.SubString(0, separator)); result != nullptr)
        return String(*result).Append(" - ").Append(driverName.SubString(separator + 1).ToUpperCaseUTF8());

  return driverName;
}*/


