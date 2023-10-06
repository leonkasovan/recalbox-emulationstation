//
// Created by gugue_U on 17/10/2020
//
#pragma once

#include <vector>
#include <systems/SystemManager.h>
#include <utils/datetime/HighResolutionTimer.h>
#include <components/VideoComponent.h>
#include <components/ScrollableContainer.h>
#include <components/GameClipContainer.h>
#include <components/GameClipNoVideoContainer.h>
#include "games/GameRandomSelector.h"

class GameClipView : public Gui
{
    enum class State
    {
        NoGameSelected,
        EmptyPlayList,
        InitPlaying,
        Playing,
        LaunchGame,
        GoToSystem,
        Quit,
        Terminated,
    };

    enum class Direction
    {
        Next,
        Previous,
    };

  private:
    class Filter : public IFilter
    {
      private:
        FileData::TopLevelFilter mFilter;
      public:
        Filter() : mFilter(FileData::BuildTopLevelFilter()) {}
        [[nodiscard]] bool ApplyFilter(const FileData& file) override
        {
          return  !file.Metadata().VideoAsString().empty() && file.IsDisplayable(mFilter);
        }
    } mFilter;

    //! Window
    WindowManager& mWindow;

    SystemManager& mSystemManager;

    GameRandomSelector mGameRandomSelector;

    static constexpr int sMaxHistory = 10;

    int mHistoryPosition;
    std::vector<FileData*> mHistory;
    Direction mDirection;

    FileData* mGame;

    HighResolutionTimer mTimer;

    State mState;

    GameClipContainer mGameClipContainer;

    GameClipNoVideoContainer mNoVideoContainer;

    int mSystemIndex;

    int mVideoDuration;

    void InsertIntoHistory(FileData* game);

    void GetGame();

    void GetNextGame();

    void GetPreviousGame();

    void StartGameClip();

    void StopGameClipView();

    void ChangeGameClip(Direction direction);

    /*
     * GUI
     */

    void Update(int elapsed) override;

    void Render(const Transform4x4f& parentTrans) override;

    bool ProcessInput(const InputCompactEvent& event) override;

    bool getHelpPrompts(Help& help) override;

  public:

    static const char* getName() { return "gameclip"; }

    static bool IsGameClipEnabled() { return RecalboxConf::Instance().GetScreenSaverType() == RecalboxConf::Screensaver::Gameclip; }

    //! Default constructor
    explicit GameClipView(WindowManager& window, SystemManager& systemManager);

    ~GameClipView() override;

    //! Reinit the view
    void Reset();
};
