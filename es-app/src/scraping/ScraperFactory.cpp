//
// Created by bkg2k on 04/12/2019.
//

#include <scraping/scrapers/thegamedb/TheGameDBEngine.h>
#include "ScraperFactory.h"
#include <scraping/scrapers/screenscraper/ScreenScraperEngineImplementation.h>
#include <scraping/scrapers/recalbox/RecalboxEngineImplementation.h>
#include <scraping/scrapers/IScraperEngineFreezer.h>
#include <scraping/ScraperSeamless.h>
#include <patreon/PatronInfo.h>

ScraperFactory::~ScraperFactory()
{
  for(auto scraper : mScrapers)
    delete scraper.second;
}

IScraperEngine* ScraperFactory::Get(ScraperType type, IScraperEngineFreezer* freezer)
{
  // Ensure valid type
  switch(type)
  {
    case ScraperType::ScreenScraper:
    case ScraperType::Recalbox:
    case ScraperType::TheGameDB: break;
    default: type = ScraperType::ScreenScraper;
  }

  // Already created?
  auto scraper = mScrapers.find(type);
  if (scraper != mScrapers.end())
    return scraper->second;

  // Nope, create it
  IScraperEngine* result = nullptr;
  switch(type)
  {
    case ScraperType::ScreenScraper: result = new ScreenScraperEngineImplementation(freezer); break;
    case ScraperType::Recalbox     : result = new RecalboxEngineImplementation(freezer); break;
    case ScraperType::TheGameDB    : result = new TheGameDBEngine(freezer); break;
  }
  mScrapers[type] = result;
  return result;
}

IScraperEngine* ScraperFactory::GetScraper(ScraperType scraper, IScraperEngineFreezer* freezer)
{
  // Get
  IScraperEngine* engine = Get(scraper, freezer);
  // (re)Initialize
  engine->Initialize();

  return engine;
}

const HashMap<ScraperType, String>& ScraperFactory::GetScraperList()
{
  static HashMap<ScraperType, String> _List =
  {
    { ScraperType::ScreenScraper, "ScreenScraper" },
    //{ ScraperType::TheGameDB, "TheGamesDB" },
  };
  static bool RecalboxChecked = false;
  if (!RecalboxChecked)
  {
    if (PatronInfo::Instance().IsPatron()
        #ifdef BETA
          || true
        #endif
        )
      _List[ScraperType::Recalbox] = "Recalbox";
    RecalboxChecked = true;
  }
  return _List;
}

void ScraperFactory::ExtractFileNameUndecorated(FileData& game)
{
  String name = game.RomPath().FilenameWithoutExtension();

  // Remove (text)
  for(int pos = 0; (pos = name.Find('(', pos)) >= 0; )
  {
    int end = name.Find(')', pos);
    if (end < 0) end = (int)name.size() - 1;
    name.erase(pos, end - pos + 1);
  }

  // Remove [text]
  for(int pos = 0; (pos = name.Find('[', pos)) >= 0; )
  {
    int end = name.Find(']', pos);
    if (end < 0) end = (int)name.size() - 1;
    name.erase(pos, end - pos + 1);
  }

  game.Metadata().SetName(name.Trim());
}

void ScraperFactory::ExtractRegionFromFilename(FileData& game)
{
  Regions::RegionPack region = Regions::ExtractRegionsFromFileName(game.RomPath());
  if (region.HasRegion())
    game.Metadata().SetRegion(region);
}

