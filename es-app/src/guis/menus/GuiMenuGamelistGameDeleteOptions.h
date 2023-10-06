
#pragma once

#include <guis/menus/GuiMenuBase.h>
#include <views/gamelist/ISimpleGameListView.h>

// Forward declaration
class SystemManager;
class SystemData;
template<class T> class OptionListComponent;

class GuiMenuGamelistGameDeleteOptions : public GuiMenuBase
  , private IGuiMenuBase


{
  public:
    explicit GuiMenuGamelistGameDeleteOptions(WindowManager& window, ISimpleGameListView& view, FileData& game);

  private:
    enum class Components
    {
      Delete,
      Advanced
    };

    //! View reference
    ISimpleGameListView& mView;
    //! Game reference
    FileData& mGame;

    HashSet<String> mGameFiles;
    HashSet<String> mExtraFiles;
    HashSet<String> mMediaFiles;
    HashSet<String> mSaveFiles;

//    std::shared_ptr<OptionListComponent<Path>> mGameFiles;
//    std::shared_ptr<OptionListComponent<Path>> mMedias;
//    std::shared_ptr<OptionListComponent<Path>> mExtras;
//    std::shared_ptr<OptionListComponent<Path>> mSaves;

    void DeleteAllFiles();

    bool ProcessInput(const InputCompactEvent& event) override;

    void SubMenuSelected(int id) override;

    String ComputeMessage();
};



