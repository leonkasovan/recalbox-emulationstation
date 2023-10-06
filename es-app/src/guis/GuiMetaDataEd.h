#pragma once

#include "guis/Gui.h"
#include "components/MenuComponent.h"
#include "GuiScraperSingleGameRun.h"

#include <systems/SystemData.h>
#include <views/gamelist/ISimpleGameListView.h>

class GuiMetaDataEd : public Gui, public GuiScraperSingleGameRun::IScrapingComplete
{
  public:
    class IMetaDataAction
    {
      public:
        virtual void Delete(FileData& game) = 0;
        virtual void Modified(ISimpleGameListView* gamelistview, FileData& game) = 0;
    };

    GuiMetaDataEd(WindowManager&window, SystemManager& systemManager, FileData& game,
                  ISimpleGameListView* gamelistview, IMetaDataAction* actions , bool main);

    bool ProcessInput(const InputCompactEvent& event) override;
    void onSizeChanged() override;
    bool getHelpPrompts(Help& help) override;

  private:
    void save();
    void fetch();
    void close(bool closeAllWindows);

    FileData& mGame;
    SystemManager& mSystemManager;

    NinePatchComponent mBackground;
    ComponentGrid mGrid;

    std::shared_ptr<TextComponent> mTitle;
    std::shared_ptr<TextComponent> mSubtitle;
    std::shared_ptr<ComponentGrid> mHeaderGrid;
    std::shared_ptr<ComponentList> mList;
    std::shared_ptr<ComponentGrid> mButtons;

    std::vector<std::shared_ptr<Component>> mEditors;
    std::vector<const MetadataFieldDescriptor*> mMetaDataEditable;

    MetadataDescriptor& mMetaData;
    ISimpleGameListView* mGameListView;
    IMetaDataAction* mActions;

    /*
     * GuiScraperSingleGameRun::IScrapingCommplete
     */
    void ScrapingComplete(FileData& game, MetadataType changedMetadata) override;
};
