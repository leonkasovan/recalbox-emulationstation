//
// Created by bkg2k on 28/01/2021.
//
// As part of the RECALBOX Project
// http://www.recalbox.com
//
#pragma once

#include <guis/menus/GuiMenuBase.h>
#include <views/gamelist/ISimpleGameListView.h>

// Forward declaration
class SystemManager;
class SystemData;
template<class T> class OptionListComponent;
class SwitchComponent;

class GuiMenuGamelistGameOptions : public GuiMenuBase
                                 , private IOptionListComponent<String>
                                 , private IOptionListComponent<Path>
                                 , private IOptionListComponent<GameGenres>
                                 , private ISwitchComponent
                                 , private IEditableComponent
                                 , private IRatingComponent
                                 , private IGuiMenuBase
                                 , private GuiScraperSingleGameRun::IScrapingComplete
{
  public:
    GuiMenuGamelistGameOptions(WindowManager& window, ISimpleGameListView& view, SystemManager& systemManager, SystemData& system, FileData& game);

    ~GuiMenuGamelistGameOptions() override;
  private:
    enum class Components
    {
      Emulator,
      Patch,
      Ratio,
      Favorite,
      Hidden,
      Adult,
      Name,
      Description,
      Rating,
      Genre,
      Scrape,
      Rotation,
    };

    //! View reference
    ISimpleGameListView& mView;
    //! System Manager reference
    SystemManager& mSystemManager;
    //! System reference
    SystemData& mSystem;
    //! Game reference
    FileData& mGame;

    //! Default emulator
    String mDefaultEmulator;
    //! Default core
    String mDefaultCore;

    //! Emulator/Core
    std::shared_ptr<OptionListComponent<String>> mEmulator;

    //! sotpatching path
    std::shared_ptr<OptionListComponent<Path>> mPath;
    //! Ratio
    std::shared_ptr<OptionListComponent<String>> mRatio;
    //! Name
    std::shared_ptr<EditableComponent> mName;
    //! Rating
    std::shared_ptr<RatingComponent> mRating;
    //! Genre
    std::shared_ptr<OptionListComponent<GameGenres>> mGenre;
    //! Description
    std::shared_ptr<EditableComponent> mDescription;
    //! Favorite
    std::shared_ptr<SwitchComponent> mFavorite;
    //! Hidden
    std::shared_ptr<SwitchComponent> mHidden;
    //! Adult
    std::shared_ptr<SwitchComponent> mAdult;
    //! Rotation
    std::shared_ptr<SwitchComponent> mRotation;

    //! Emulator/Core list
    std::shared_ptr<IOptionListComponent<String>> mEmulators;

    //! Get emulator list
    std::vector<ListEntry<String>> GetEmulatorEntries();
    //! Get available patch List
    std::vector<ListEntry<Path>> GetPatchEntries();
    //! Get ratio list
    std::vector<ListEntry<String>> GetRatioEntries();
    //! Get rating list
    std::vector<ListEntry<float>> GetRatingEntries();
    //! Get genre list
    std::vector<ListEntry<GameGenres>> GetGenreEntries();

    /*
     * IOptionListComponent<String> implementation
     */

    void OptionListComponentChanged(int id, int index, const String& value) override;

    void OptionListComponentChanged(int id, int index, const Path& value) override;

    /*
     * IOptionListComponent<GameGenres> implementation
     */

    void OptionListComponentChanged(int id, int index, const GameGenres& value) override;

    /*
     * ISwitchComponent implementation
     */

    void EditableComponentTextChanged(int id, const String& text) override;

    /*
     * IEditableComponent implementation
     */

    void SwitchComponentChanged(int id, bool& status) override;

    /*
     * IRatingComponent implementation
     */

    void RatingChanged(int id, float value) override;

    /*
     * IGuiMenuBase implementation
     */

    void SubMenuSelected(int id) override;

    /*
     * GuiScraperSingleGameRun::IScrapingComplete implementation
     */

    void ScrapingComplete(FileData& game, MetadataType changedMetadata) override;
};



