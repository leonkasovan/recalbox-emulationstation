// GUI for Local Scraping (in /recalbox/share/screenshots etc)
// Created by Dhani Novan on 31/03/2024.
//

#ifndef EMULATIONSTATION_ALL_GUISCRAPELOCAL_H
#define EMULATIONSTATION_ALL_GUISCRAPELOCAL_H

#include <components/ComponentList.h>
#include "components/ComponentGrid.h"
#include <components/NinePatchComponent.h>
#include <components/ButtonComponent.h>
#include <components/TextEditComponent.h>
#include <components/OptionListComponent.h>
#include <components/VideoComponent.h>
#include <themes/MenuThemeData.h>
#include "systems/SystemManager.h"

class GuiScrapeLocal : public Gui
{
  public:
    GuiScrapeLocal(WindowManager& window, FileData& game);

    ~GuiScrapeLocal() override;

//     float getButtonGridHeight() const;

//     void updateSize();

    bool ProcessInput(const InputCompactEvent& event) override;

    bool getHelpPrompts(Help& help) override;

    void onSizeChanged() override;
    void updateInformations();
    void PopulateGrid(bool All = false);

//     void Update(int deltaTime) override;

//     void Render(const Transform4x4f& parentTrans) override;

//     void PopulateGrid(const String& search);

//     void PopulateGrid2(const String& search);

//     void populateGridMeta(int i);

//     void launch();

//     void GoToGame();

//     void initGridsNStuff();

//     void ResizeGridLogo();

//     void clear();

//     int SearchCSV(const char *csv_fname, char **lword, unsigned int start_no);

  private:
  FileData& mGame;
  std::vector<String> filenames;
    NinePatchComponent mBackground;
    //full grid (entire frame)
    ComponentGrid mGrid;
    std::shared_ptr<MenuTheme> mMenuTheme;
    //grid for list & Meta
    std::shared_ptr<ComponentGrid> mGridMeta;
    //grid for logo + publisher and developer
    std::shared_ptr<ComponentGrid> mGridLogoAndMD;
    //3x3 grid to center logo
    std::shared_ptr<ComponentGrid> mGridLogo;
    std::shared_ptr<ComponentGrid> mButtonGrid;
    std::vector<std::shared_ptr<ButtonComponent> > mButtons;
    std::shared_ptr<TextComponent> mTitle;
    std::shared_ptr<TextComponent> mText;
    std::shared_ptr<TextEditComponent> mSearch;
    std::shared_ptr<TextComponent> mMDDeveloperLabel;
    std::shared_ptr<TextComponent> mMDDeveloper;
    std::shared_ptr<TextComponent> mMDPublisherLabel;
    std::shared_ptr<TextComponent> mMDPublisher;
    std::shared_ptr<ComponentList> mList;
    std::shared_ptr<ImageComponent> mThumbnail;
//     std::shared_ptr<ImageComponent> mResultSystemLogo;
//     std::shared_ptr<VideoComponent> mResultVideo;
//     std::shared_ptr<ScrollableContainer> mDescContainer;
//     std::shared_ptr<TextComponent> mResultDesc;
//     std::shared_ptr<OptionListComponent<FolderData::FastSearchContext>> mSearchChoices;
//     FileData::List mSearchResults;
//     SystemData* mSystemData;
//     std::vector<SearchResult2> mSR2;
};

#endif //EMULATIONSTATION_ALL_GUISCRAPELOCAL_H
