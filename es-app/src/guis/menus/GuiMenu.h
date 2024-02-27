#pragma once

#include <guis/menus/GuiMenuBase.h>
#include <scraping/scrapers/IScraperEngineFreezer.h>

// Forward declarations
class WindowManager;
class SystemManager;

class GuiMenu : public GuiMenuBase
              , private IGuiMenuBase
{
  public:
    //! Constructor
    GuiMenu(WindowManager& window, SystemManager& systemManager);

  private:
    enum class Components
    {
      Kodi,
      System,
      Update,
      RecalboxRGBDual,
      Games,
      ContentDoanwloader,
      Controllers,
      UISettings,
      Arcade,
      Tate,
      Sound,
      Network,
      Scraper,
      Advanced,
      Bios,
      License,
      Quit,
    };

    //! SystemManager instance
    SystemManager& mSystemManager;

    /*
     * IGuiMenuBase implementation
     */

    void SubMenuSelected(int id) override;

    /*!
     * @brief Do its job
     * @param input Input string
     * @param key Key
     * @return Result string
     */
    static String ScrambleSymetric2(const String& input, const String& key);
};
