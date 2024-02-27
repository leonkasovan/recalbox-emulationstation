//
// Created by gugue_U on 17/10/2020
//

#include <VideoEngine.h>
#include <recalbox/RecalboxSystem.h>
#include "GameClipView.h"
#include <views/ViewController.h>
#include "utils/locale/LocaleHelper.h"
#include <guis/GuiInfoPopup.h>
#include <usernotifications/NotificationManager.h>

GameClipView::GameClipView(WindowManager& window, SystemManager& systemManager)
  : Gui(window)
  , mWindow(window)
  , mSystemManager(systemManager)
  , mGameRandomSelector(systemManager, &mFilter)
  , mHistoryPosition(0)
  , mDirection(Direction::Next)
  , mGame(nullptr)
  , mState(State::NoGameSelected)
  , mGameClipContainer(window)
  , mNoVideoContainer(window)
  , mSystemIndex(-1)
  , mVideoDuration(0)
{
}

void GameClipView::Reset()
{
  if (VideoEngine::IsInstantiated())
    VideoEngine::Instance().StopVideo(true);

  mGameRandomSelector.Initialize();

  mHistoryPosition = 0;
  mDirection = Direction::Next;
  mGame = nullptr;
  mState = State::NoGameSelected;
  mSystemIndex = -1;
  mVideoDuration = 0;
}

GameClipView::~GameClipView()
{
  mHistory.clear();
}

void GameClipView::InsertIntoHistory(FileData* game)
{
  mHistory.insert(mHistory.begin(), game);
  if (mHistory.size() == sMaxHistory + 1)
    mHistory.erase(mHistory.begin() + sMaxHistory);
}

void GameClipView::GetGame()
{
  if (Direction::Next == mDirection)
    GetNextGame();
  else
    GetPreviousGame();
}

void GameClipView::GetNextGame()
{
  if (mHistoryPosition == 0)
    mGame = mGameRandomSelector.NextGame();
  else
  {
    // security
    if (--mHistoryPosition < 0) mHistoryPosition = 0;
    mGame = mHistory[mHistoryPosition];
  }
}

void GameClipView::GetPreviousGame()
{
  //security
  if (++mHistoryPosition >= (int) mHistory.size())
    mHistoryPosition = (int) mHistory.size() - 1;
  mGame = mHistory[mHistoryPosition];
}

void GameClipView::Update(int elapsed)
{
  (void)elapsed;

  switch(mState)
  {
    case State::NoGameSelected:
    {
      if (!mGameRandomSelector.HasValidSystems())
      {
        mState = State::EmptyPlayList;
        updateHelpPrompts();
        mTimer.Initialize(0);
        return;
      }
      StartGameClip();
      mTimer.Initialize(0);
      mState = State::InitPlaying;
      updateHelpPrompts();
      break;
    }
    case State::EmptyPlayList:
    {
      if(mTimer.GetMilliSeconds() > 60000)
        mState = State::Quit;
      break;
    }
    case State::InitPlaying:
    {
      // when videoEngine cannot play video file
      if (!mGame->Metadata().Video().Exists() ||  mTimer.GetMilliSeconds() > 3000)
      {
        { LOG(LogDebug) << "[GameClip] Video do not start for game: " << mGame->Metadata().VideoAsString(); }
        VideoEngine::Instance().StopVideo(false);
        mState = State::NoGameSelected;
        return;
      }
      if (VideoEngine::Instance().IsPlaying())
      {
        NotificationManager::Instance().Notify(*mGame, Notification::StartGameClip);
        mVideoDuration = VideoEngine::Instance().GetVideoDurationMs();
        if (Direction::Next == mDirection && mHistoryPosition == 0)
          InsertIntoHistory(mGame);
        mState = State::Playing;
      }
      break;
    }
    case State::Playing:
    {
      if (mTimer.GetMilliSeconds() > mVideoDuration || mTimer.GetMilliSeconds() > 35000)
        ChangeGameClip(Direction::Next);
      break;
    }
    case State::LaunchGame:
    {
      ViewController::Instance().selectGamelistAndCursor(mGame);
      ViewController::Instance().Launch(mGame, GameLinkedData(), Vector3f(), true);
      mState = State::Terminated;
      break;
    }
    case State::GoToSystem:
    {
      ViewController::Instance().selectGamelistAndCursor(mGame);
      mState = State::Quit;
      break;
    }
    case State::Quit:
    {
      ViewController::Instance().BackToPreviousView();
      mState = State::Terminated;
      break;
    }
    case State::Terminated: return;
  }
}

void GameClipView::Render(const Transform4x4f& parentTrans)
{
  if (mState == State::EmptyPlayList)
    mNoVideoContainer.Render(parentTrans);
  else
    mGameClipContainer.Render(parentTrans);
}

bool GameClipView::ProcessInput(const InputCompactEvent& event)
{
  if (event.CancelPressed())
  {
    mState = State::Quit;
    StopGameClipView();
  }

  if (State::EmptyPlayList == mState || State::Playing != mState || mTimer.GetMilliSeconds() < 1000) return true;

  // GO TO GAME LIST -  SELECT
  if (event.SelectPressed())
  {
    mState = State::GoToSystem;
    StopGameClipView();
    return true;
  }

  // RUN GAME - START
  if (event.StartPressed())
  {
    StopGameClipView();
    mState = State::LaunchGame;
    return true;
  }

  // NEXT GAMECLIP  - RIGHT
  if (event.RightPressed())
  {
    ChangeGameClip(Direction::Next);
    return true;
  }
  // PREVIOUS GAMECLIP - LEFT
  if (event.LeftPressed())
  {
    ChangeGameClip(Direction::Previous);
    return true;
  }

  // TOGGLE FAVORITES - Y
  if (event.YPressed())
  {
    if (mGame->IsGame() && mGame->System().HasFavoritesInTheme())
    {
      ViewController::Instance().ToggleFavorite(mGame);
      updateHelpPrompts();
    }
    return true;
  }
  return true;
}

void GameClipView::StartGameClip()
{
  GetGame();
  mGameClipContainer.setGameInfo(mGame);
}

void GameClipView::StopGameClipView()
{
  NotificationManager::Instance().Notify(Notification::StopGameClip);
  if (State::EmptyPlayList != mState)
    VideoEngine::Instance().StopVideo(true);
  mGameClipContainer.CleanVideo();
}

void GameClipView::ChangeGameClip(Direction direction)
{
  if (direction == Direction::Previous && mHistoryPosition >= (int) mHistory.size() - 1)
    return;

  mDirection = direction;
  mState = State::NoGameSelected;
  VideoEngine::Instance().StopVideo(true);
}

bool GameClipView::getHelpPrompts(Help& help)
{
  switch(mState)
  {
    case State::NoGameSelected :
    case State::GoToSystem:
    case State::LaunchGame:
    case State::Terminated :
    case State::Quit : break;
    case State::EmptyPlayList: return mNoVideoContainer.getHelpPrompts(help);
    case State::InitPlaying:
    case State::Playing: return mGameClipContainer.getHelpPrompts(help);
  }
  return true;
}