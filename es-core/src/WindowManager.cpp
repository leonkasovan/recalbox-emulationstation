#include <utils/locale/LocaleHelper.h>
#include <WindowManager.h>
#include <guis/GuiInfoPopupBase.h>
#include <guis/GuiMsgBoxScroll.h>
#include <guis/GuiMsgBox.h>
#include <themes/MenuThemeData.h>
#include <views/ViewController.h>
#include <usernotifications/NotificationManager.h>
#include "guis/GuiInfoPopup.h"

WindowManager::WindowManager()
  : mOSD(*this)
  , mHelp(*this)
  , mBackgroundOverlay(*this)
  , mInfoPopups(sMaxInfoPopups)
  , mGuiStack(16) // Allocate memory once for all gui
  , mAverageDeltaTime(10)
  , mTimeSinceLastInput(0)
  , mNormalizeNextUpdate(false)
  , mSleeping(false)
  , mRenderedHelpPrompts(false)
  , mDisplayEnabled(true)
{
  auto menuTheme = MenuThemeData::getInstance()->getCurrentTheme();
  mBackgroundOverlay.setImage(menuTheme->menuBackground.fadePath);
}

WindowManager::~WindowManager()
{
  deleteAllGui();
}

bool WindowManager::UpdateHelpSystem()
{
  Gui* gui = peekGui();
  if (gui != nullptr)
  {
    gui->updateHelpPrompts();
    return true;
  }
  return false;
}

void WindowManager::pushGui(Gui* gui)
{
  if (Gui* top = peekGui(); top != nullptr) top->onHide();
  mGuiStack.Push(gui);
  gui->onShow();
  UpdateHelpSystem();
}

void WindowManager::RemoveGui(Gui* gui)
{
  Gui* previousTop = peekGui();
  for(int i = mGuiStack.Count(); --i >= 0;)
    if (mGuiStack[i] == gui)
    {
      gui->onHide();
      mGuiStack.PopAt(i);
    }
  if (Gui* top = peekGui(); top != nullptr && top != previousTop) top->onShow();
}

void WindowManager::deleteClosePendingGui()
{
  Gui* previousTop = peekGui();
  bool deleted = false;
  for(int i = mGuiStack.Count(); --i >= 0;)
    if (mGuiStack[i]->IsPendingForDeletion())
    {
      Gui* gui = mGuiStack.PopAt(i);
      gui->onHide();
      delete gui;
      deleted = true;
    }
  if (Gui* top = peekGui(); top != nullptr && top != previousTop) top->onShow();

  // Refresh help system
  if (deleted)
    UpdateHelpSystem();
}

void WindowManager::deleteAllGui()
{
  for(int i = mInfoPopups.Count(); --i >= 0; )
    delete mInfoPopups[i];
  mInfoPopups.Clear();

  for(int i = mGuiStack.Count(); --i >= 0;)
  {
    Gui* gui = mGuiStack.PopAt(i);
    gui->onHide();
    delete gui;
  }
  mGuiStack.Clear();
}

void WindowManager::displayMessage(const String& message, bool urgent)
{
  if (!urgent && !mMessages.empty()) return;

  mMessages.push_back(message);
}

void WindowManager::displayScrollMessage(const String& title, const String& message, bool urgent)
{
  if (!urgent && !mScrollMessages.empty()) return;

  mScrollTitle.push_back(title);
  mScrollMessages.push_back(message);
}

Gui* WindowManager::peekGui()
{
  if (mGuiStack.Empty())
    return nullptr;

  return mGuiStack.Peek();
}

bool WindowManager::ReInitialize()
{
  // Reinit video
  if (!Renderer::Instance().ReInitialize())
  {
    { LOG(LogError) << "[WindowManager] Renderer failed to initialize!"; }
    return false;
  }
  // Reload resources
  ResourceManager::getInstance()->reloadAll();
  // Update help system
  UpdateHelpSystem();

  return true;
}

bool WindowManager::Initialize(unsigned int width, unsigned int height, bool initRenderer)
{
  if (initRenderer)
  {
    if (!Renderer::Instance().Initialize((int) width, (int) height))
    {
      { LOG(LogError) << "[WindowManager] Renderer failed to initialize!"; }
      return false;
    }
  }

  String glExts = (const char*) glGetString(GL_EXTENSIONS);
  { LOG(LogInfo) << "[WindowManager] Checking available OpenGL extensions..."; }
  { LOG(LogInfo) << "[WindowManager] ARB_texture_non_power_of_two: "
                 << (glExts.Contains("ARB_texture_non_power_of_two") ? "OK" : "MISSING"); }

  //InputManager::Instance().Initialize(this);
  ResourceManager::getInstance()->reloadAll();

  //keep a reference to the default fonts, so they don't keep getting destroyed/recreated
  if (mDefaultFonts.empty())
  {
    mDefaultFonts.push_back(Font::get(FONT_SIZE_SMALL));
    mDefaultFonts.push_back(Font::get(FONT_SIZE_MEDIUM));
    mDefaultFonts.push_back(Font::get(FONT_SIZE_LARGE));
  }

  mBackgroundOverlay.setResize(Renderer::Instance().DisplayWidthAsFloat(), Renderer::Instance().DisplayHeightAsFloat());

  // update our help because font sizes probably changed
  UpdateHelpSystem();

  return true;
}

void WindowManager::Finalize()
{
  //InputManager::Instance().Finalize();
  ResourceManager::getInstance()->unloadAll();
  Renderer::Instance().Finalize();
}

void WindowManager::textInput(const char* text)
{
  if (!mGuiStack.Empty())
    mGuiStack.Peek()->textInput(text);
}

bool WindowManager::ProcessInput(const InputCompactEvent& event)
{
  if (mSleeping && !GameClipView::IsGameClipEnabled())
  {
    // wake up
    DoWake();
    return true;
  }

  mOSD.ProcessInput(event);

  mTimeSinceLastInput = 0;
  if (peekGui() != nullptr)
  {
    peekGui()->ProcessInput(event);
    // Top Gui always consume events
    return true;
  }

  if (KonamiCode(event))
  {
    // Surprise!
  }

  return false;
}

void WindowManager::Update(int deltaTime)
{
  if (!DoNotDisturb())
  {
    if (!mMessages.empty())
    {
      String message = mMessages.back();
      mMessages.pop_back();
      pushGui(new GuiMsgBox(*this, message));
    }
    else if (!mScrollMessages.empty())
    {
      String message = mScrollMessages.back();
      String title = mScrollTitle.back();
      mScrollMessages.pop_back();
      mScrollTitle.pop_back();
      pushGui(new GuiMsgBoxScroll(*this, title, message, _("OK"), []
      {}, "", nullptr, "", nullptr, TextAlignment::Left));
    }
  }

  if (mNormalizeNextUpdate)
  {
    mNormalizeNextUpdate = false;
    if (deltaTime > mAverageDeltaTime)
      deltaTime = mAverageDeltaTime;
  }

  mTimeSinceLastInput += deltaTime;

  // Process GUI pending for deletion
  deleteClosePendingGui();

  mHelp.Update(deltaTime);

  // Process highest GUI
  if (!mGuiStack.Empty())
    mGuiStack.Peek()->Update(deltaTime);

  // Process popups
  InfoPopupsUpdate(deltaTime);

  // Process input OSD
  mOSD.Update(deltaTime);
}

void WindowManager::Render(Transform4x4f& transform)
{
  if (!mDisplayEnabled)
  {
    Renderer::SetMatrix(Transform4x4f::Identity());
    Renderer::DrawRectangle(0, 0, Renderer::Instance().DisplayWidthAsInt(), Renderer::Instance().DisplayHeightAsInt(), 0x000000FF);
    return;
  }

  mRenderedHelpPrompts = false;

  // draw only bottom and top of GuiStack (if they are different)
  if (!mGuiStack.Empty())
  {
    int stackSize = (int)mGuiStack.Count();
    Gui* previous = stackSize > 1 ? mGuiStack[stackSize - 2] : nullptr;
    Gui* top = mGuiStack.Peek();

    mBackgroundOverlay.Render(transform);
    if (top->IsOverlay())
      if (stackSize > 1 && previous != nullptr) previous->Render(transform);

    if (top->MustRenderOverHelpSystem())
      renderHelpPromptsEarly();

    top->Render(transform);
  }

  if (!mRenderedHelpPrompts)
    mHelp.Render(transform);

  unsigned int screensaverTime = (unsigned int) RecalboxConf::Instance().GetScreenSaverTime() * 60000;
  if (mTimeSinceLastInput >= screensaverTime && screensaverTime != 0)
  {
    if (!isProcessing())
    {
      renderScreenSaver();

      // go to sleep
      if (!isSleeping())
        DoSleep();
    }
  }

  // Reset matrix
  Renderer::SetMatrix(Transform4x4f::Identity());
  // Then popups
  InfoPopupsDisplay(transform);
  // Pad OSD
  mOSD.Render(Transform4x4f::Identity());
}

void WindowManager::renderHelpPromptsEarly()
{
  mHelp.Render(Transform4x4f::Identity());
  mRenderedHelpPrompts = true;
}

bool WindowManager::isProcessing()
{
  for(int i = mGuiStack.Count(); --i >= 0;)
    if (mGuiStack[i]->isProcessing())
      return true;
  return false;
}

void WindowManager::exitScreenSaver()
{
  if (Board::Instance().HasBrightnessSupport())
    Board::Instance().SetBrightness(RecalboxConf::Instance().GetBrightness());
}

void WindowManager::renderScreenSaver()
{
  if (Board::Instance().HasSuspendResume() && RecalboxConf::Instance().GetScreenSaverType() == RecalboxConf::Screensaver::Suspend)
  {
    Board::Instance().Suspend();
    DoWake(); // Exit screensaver immediately on resume
  }
  else if (Board::Instance().HasBrightnessSupport())
  {
    RecalboxConf::Screensaver screenSaver = RecalboxConf::Instance().GetScreenSaverType();
    if (screenSaver == RecalboxConf::Screensaver::Black) Board::Instance().SetLowestBrightness();
    else if (screenSaver == RecalboxConf::Screensaver::Dim)
    {
      int brightness = RecalboxConf::Instance().GetBrightness();
      Board::Instance().SetBrightness(brightness >> 1);
    }
  }
  else if (RecalboxConf::Instance().GetScreenSaverType() == RecalboxConf::Screensaver::Gameclip)
  {
    if (mGuiStack.Empty())
      ViewController::Instance().goToGameClipView();
  }
  else
  {
    Renderer::SetMatrix(Transform4x4f::Identity());
    unsigned char opacity = RecalboxConf::Instance().GetScreenSaverType() == RecalboxConf::Screensaver::Dim ? 0xA0 : 0xFF;
    Renderer::DrawRectangle(0, 0, Renderer::Instance().DisplayWidthAsInt(), Renderer::Instance().DisplayHeightAsInt(), 0x00000000 | opacity);
  }
}

bool WindowManager::KonamiCode(const InputCompactEvent& input)
{
  typedef bool (InputCompactEvent::*EventMethodPointer)() const;

  static constexpr int sKonamiLength = 10;
  static EventMethodPointer sKonami[sKonamiLength] =
  {
    &InputCompactEvent::AnyUpPressed,
    &InputCompactEvent::AnyUpPressed,
    &InputCompactEvent::AnyDownPressed,
    &InputCompactEvent::AnyDownPressed,
    &InputCompactEvent::AnyLeftPressed,
    &InputCompactEvent::AnyRightPressed,
    &InputCompactEvent::AnyLeftPressed,
    &InputCompactEvent::AnyRightPressed,
    &InputCompactEvent::BPressed,
    &InputCompactEvent::APressed,
  };
  static int KonamiCount = 0;

  if (!(input.*(sKonami[KonamiCount++]))()) KonamiCount = 0; // Reset on no matching
  if (KonamiCount == sKonamiLength)
  {
    KonamiCount = 0;
    return true; // Yes konami code!
  }

  return false;
}

void WindowManager::RenderAll(bool halfLuminosity)
{
  Transform4x4f transform(Transform4x4f::Identity());
  Render(transform);
  if (halfLuminosity)
  {
    Renderer::SetMatrix(Transform4x4f::Identity());
    Renderer::DrawRectangle(0.f, 0.f, Renderer::Instance().DisplayWidthAsFloat(), Renderer::Instance().DisplayHeightAsFloat(), 0x00000080);
  }
  mOSD.GetFpsOSD().RecordStopFrame();
  Renderer::Instance().SwapBuffers();
  mOSD.GetFpsOSD().RecordStartFrame();
}

void WindowManager::CloseAll()
{
  for(int i = mGuiStack.Count(); --i >= 0;)
    mGuiStack[i]->Close();
}

void WindowManager::DoWake()
{
  mTimeSinceLastInput = 0;
  if (mSleeping)
  {
    mSleeping = false;
    exitScreenSaver();
    NotificationManager::Instance().Notify(Notification::WakeUp, String(mTimeSinceLastInput));
  }
}

void WindowManager::DoSleep()
{
  if( RecalboxConf::Instance().GetScreenSaverType() == RecalboxConf::Screensaver::Gameclip && !mGuiStack.Empty())
    return;
  
  if (!mSleeping)
  {
    mSleeping = true;
    NotificationManager::Instance().Notify(Notification::Sleep, String(mTimeSinceLastInput));
  }
}

void WindowManager::InfoPopupsShrink()
{
  int types = 0;
  for(int i = mInfoPopups.Count(); --i >= 0; )
    if ((types & (1 << (int)mInfoPopups[i]->Type())) == 0)
      types |= (1 << (int)mInfoPopups[i]->Type());
    else
      mInfoPopups.Delete(i);

  // Security
  while(mInfoPopups.Count() >= sMaxInfoPopups)
    InfoPopupsRemove(0);
}

void WindowManager::InfoPopupRetarget()
{
  int gap = (int)(Renderer::Instance().DisplayHeightAsFloat() * 0.01f);
  if (gap < 2) gap = 2;
  int targetOffset = 0;
  for(int i = 0; i < mInfoPopups.Count(); i++)
  {
    mInfoPopups[i]->SetOffset(targetOffset);
    targetOffset += (int)mInfoPopups[i]->getSize().y() + gap;
  }
}

void WindowManager::InfoPopupAdd(GuiInfoPopupBase* infoPopup, bool first)
{
  infoPopup->Initialize();
  if (first) mInfoPopups.Insert(infoPopup, 0);
  else mInfoPopups.Add(infoPopup);
  InfoPopupsShrink();
  InfoPopupRetarget();
}

void WindowManager::InfoPopupAddRegular(const String& message, int duration, PopupType icon, bool first)
{
  GuiInfoPopupBase* infoPopup = new GuiInfoPopup(*this, message, duration, icon);
  infoPopup->Initialize();
  if (first) mInfoPopups.Insert(infoPopup, 0);
  else mInfoPopups.Add(infoPopup);
  InfoPopupsShrink();
  InfoPopupRetarget();
}

bool WindowManager::InfoPopupIsShown(GuiInfoPopupBase* infoPopup)
{
  for(int i = mInfoPopups.Count(); --i >= 0;)
    if (mInfoPopups[i] == infoPopup)
      return true;
  return false;
}

void WindowManager::InfoPopupRemove(GuiInfoPopupBase* infoPopup)
{
  for(int i = mInfoPopups.Count(); --i >= 0;)
    if (mInfoPopups[i] == infoPopup)
      InfoPopupsRemove(i);
}

void WindowManager::InfoPopupsRemove(int index)
{
  GuiInfoPopupBase* popup = mInfoPopups[index];
  mInfoPopups.Delete(index); // Delete pointer
  if (!popup->SelfProcessed())
    delete popup; // Delete object

  // Move other popups
  InfoPopupRetarget();
}

void WindowManager::InfoPopupsUpdate(int delta)
{
  if (!DoNotDisturb())
    for(int i = mInfoPopups.Count(); --i >= 0;)
    {
      mInfoPopups[i]->Update(delta);
      if (mInfoPopups[i]->TimeOut() && !mInfoPopups[i]->SelfProcessed())
        InfoPopupsRemove(i);
    }
}

void WindowManager::InfoPopupsDisplay(Transform4x4f& transform)
{
  if (!DoNotDisturb())
    for(int i = mInfoPopups.Count(); --i >= 0;)
      mInfoPopups[i]->Render(transform);
}

