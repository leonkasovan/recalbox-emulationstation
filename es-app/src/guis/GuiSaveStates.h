//
// Created by gugue_u on 04/01/2023.
//

#include <components/ComponentList.h>
#include "components/ComponentGrid.h"
#include <components/NinePatchComponent.h>
#include <components/OptionListComponent.h>
#include <themes/MenuThemeData.h>
#include "systems/SystemManager.h"
#include "games/SaveState.h"
#include <views/ISaveStateSlotNotifier.h>

class GuiSaveStates : public Gui
{
  public:
    GuiSaveStates(WindowManager& window, SystemManager& systemManager, FileData& game, ISaveStateSlotNotifier* notifier, bool fromMenu);

  private:

    enum class Sort
    {
      Ascending,
      Descending,
    };

    SystemManager& mSystemManager;

    NinePatchComponent mBackground;
    ComponentGrid mGrid;
    std::shared_ptr<MenuTheme> mMenuTheme;
    std::shared_ptr<TextComponent> mTitle;
    std::shared_ptr<TextComponent> mGameName;
    std::shared_ptr<ComponentList> mList;
    std::shared_ptr<ImageComponent> mThumbnail;
    std::vector<SaveState> mSaveStates;
    FileData& mGame;
    bool mIsLibretro;
    bool mFromMenu;
    SaveState mCurrentState;
    Sort mSort;
    ISaveStateSlotNotifier* mInterface;

    bool ProcessInput(const InputCompactEvent& event) override;

    bool getHelpPrompts(Help& help) override;

    void onSizeChanged() override;

    void Update(int deltaTime) override;

    void Render(const Transform4x4f& parentTrans) override;

    void PopulateGrid();

    void updateInformations();

    void launch(int slot);

    void initGridsNStuff();

    void Delete();

    void Scrape();

    static bool asc(const SaveState& first, const SaveState& second)
    {
      if(first.GetIsAuto())
        return true;
      if(second.GetIsAuto())
        return false;

      return first.GetSlotNumber() < second.GetSlotNumber();
    }

    static bool desc(const SaveState& first, const SaveState& second)
    {
      if(first.GetIsAuto())
        return true;

      if(second.GetIsAuto())
        return false;

      return first.GetSlotNumber() > second.GetSlotNumber();
    }

    std::function<void()> func;

};