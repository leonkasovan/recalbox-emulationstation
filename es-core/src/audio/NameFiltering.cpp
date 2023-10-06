//
// Created by bkg2k on 14/08/2020.
//

#include "NameFiltering.h"

String NameFiltering::Filter(const String& sourceString, Source from)
{
  String result = sourceString;

  // Pi3 replacement
  if (result == "bcm2835 ALSA")
  {
    if (from == Source::Card) return "";
    return "Headphones";
  }
  if (result == "bcm2835 IEC958/HDMI") return "HDMI";


  result.Replace("bcm2835", "") // Pi4 filtering
        .Replace("IEC958", "Digital/HDMI"); // Generic HDMI

  return result.Trim();
}
