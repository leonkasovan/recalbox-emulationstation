//
// Created by gugue_u on 18/02/2021.
//

#include <scraping/ScraperTools.h>
#include "ScreenScraperEnums.h"

ScreenScraperEnums::ScreenScraperRegionPriority ScreenScraperEnums::ScreenScraperRegionPriorityFromString(const String& regionPriority)
{
  if (regionPriority == "favoriteRegion") return ScreenScraperEnums::ScreenScraperRegionPriority::FavoriteRegion;
  return ScreenScraperEnums::ScreenScraperRegionPriority::DetectedRegion;
}

const String& ScreenScraperEnums::ScreenScraperRegionPriorityFromEnum(ScreenScraperRegionPriority regionPriority)
{
  switch (regionPriority)
  {
    case ScreenScraperEnums::ScreenScraperRegionPriority::FavoriteRegion:
    {
      static String sScraper = "favoriteRegion";
      return sScraper;
    }
    case ScreenScraperEnums::ScreenScraperRegionPriority::DetectedRegion:
    {
      break;
    }
  }
  static String sScraper = "DetectedRegion";
  return sScraper;
}

ScreenScraperEnums::ScreenScraperImageType ScreenScraperEnums::ScreenScraperImageTypeFromString(const String& imageType)
{
  if (imageType == "None") return ScreenScraperImageType::None;
  if (imageType == "InGameScreenShot") return ScreenScraperImageType::ScreenshotIngame;
  if (imageType == "TitleScreenshot") return ScreenScraperImageType::ScreenshotTitle;
  if (imageType == "ClearLogo") return ScreenScraperImageType::Wheel;
  if (imageType == "Marquee") return ScreenScraperImageType::Marquee;
  if (imageType == "Box2D") return ScreenScraperImageType::Box2d;
  if (imageType == "Box3D") return ScreenScraperImageType::Box3d;
  if (imageType == "MixV1") return ScreenScraperImageType::MixV1;
  if (imageType == "MixV2") return ScreenScraperImageType::MixV2;
  return ScreenScraperImageType::Unknown;
}

const String& ScreenScraperEnums::ScreenScraperImageTypeFromEnum(ScreenScraperImageType imageType)
{
  switch (imageType)
  {
    case ScreenScraperImageType::None:
    {
      static String sImageType = "None";
      return sImageType;
    }
    case ScreenScraperImageType::ScreenshotIngame:
    {
      static String sImageType = "InGameScreenShot";
      return sImageType;
    }
    case ScreenScraperImageType::ScreenshotTitle:
    {
      static String sImageType = "TitleScreenshot";
      return sImageType;
    }
    case ScreenScraperImageType::Wheel:
    {
      static String sImageType = "ClearLogo";
      return sImageType;
    }
    case ScreenScraperImageType::Marquee:
    {
      static String sImageType = "Marquee";
      return sImageType;
    }
    case ScreenScraperImageType::Box2d:
    {
      static String sImageType = "Box2D";
      return sImageType;
    }
    case ScreenScraperImageType::Box3d:
    {
      static String sImageType = "Box3D";
      return sImageType;
    }
    case ScreenScraperImageType::MixV1:
    {
      static String sImageType = "MixV1";
      return sImageType;
    }
    case ScreenScraperImageType::MixV2:
    {
      static String sImageType = "MixV2";
      return sImageType;
    }
    case ScreenScraperImageType::Unknown:
    {
      break;
    }
  }
  static String sImageType = "Unknown";
  return sImageType;
}

ScreenScraperEnums::ScreenScraperVideoType ScreenScraperEnums::ScreenScraperVideoTypeFromString(const String& videoType)
{
  if (videoType == "NormalizedVideo") return ScreenScraperVideoType::Optimized;
  if (videoType == "OriginalVideo") return ScreenScraperVideoType::Raw;
  return ScreenScraperVideoType::None;
}

const String& ScreenScraperEnums::ScreenScraperVideoTypeFromEnum(ScreenScraperVideoType videoType)
{
  switch (videoType)
  {
    case ScreenScraperVideoType::None:
    {
      static String sScraper = "None";
      return sScraper;
    }
    case ScreenScraperVideoType::Raw:
    {
      static String sFileName = "OriginalVideo";
      return sFileName;
    }
    case ScreenScraperVideoType::Optimized:
    {
      break;
    }
  }
  static String sFileName = "NormalizedVideo";
  return sFileName;
}