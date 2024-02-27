//
// Created by bkg2k on 21/11/2019.
//

#include <Renderer.h>
#include <utils/locale/LocaleHelper.h>
#include "SplashView.h"

#define FONT_SIZE_LOADING ((unsigned int)(0.065f * Math::min(Renderer::Instance().DisplayHeightAsFloat(), Renderer::Instance().DisplayWidthAsFloat())))

SplashView::SplashView(WindowManager& window)
  : Gui(window)
  , mLogo(window, true, true)
  , mLoading(window, _("LOADING..."), Font::get(FONT_SIZE_MEDIUM), 0)
  , mSystemCount(0)
  , mSystemLoaded(0)
{
  mIsRecalboxRGBHat = (Board::Instance().CrtBoard().GetCrtAdapter() == CrtAdapterType::RGBDual
      || Board::Instance().CrtBoard().GetCrtAdapter() == CrtAdapterType::RGBJamma)
      && Board::Instance().CrtBoard().IsCrtAdapterAttached();
  mIsRGBJamma = Board::Instance().CrtBoard().GetCrtAdapter() == CrtAdapterType::RGBJamma
      && Board::Instance().CrtBoard().IsCrtAdapterAttached();
  mPosition.Set(0,0,0);
  mSize.Set(Renderer::Instance().DisplayWidthAsFloat(), Renderer::Instance().DisplayHeightAsFloat());

  mLogo.setResize(Renderer::Instance().DisplayWidthAsFloat() * (mIsRecalboxRGBHat ? 0.5f : 0.3f), 0.0f);
  mLogo.setImage(Path(mIsRecalboxRGBHat ? (mIsRGBJamma ? ":/crt/logojamma.svg" : ":/crt/logo.png") : ":/splash.svg"));
  mLogo.setPosition((Renderer::Instance().DisplayWidthAsFloat() - mLogo.getSize().x()) / 2,
                     (Renderer::Instance().DisplayHeightAsFloat() - mLogo.getSize().y()) / 2 * (mIsRGBJamma ? 1 : 0.6f));

  mLoading.setHorizontalAlignment(TextAlignment::Center);
  mLoading.setSize(Renderer::Instance().DisplayWidthAsFloat(), Renderer::Instance().DisplayHeightAsFloat() / 10.0f);
  mLoading.setPosition(0.0f, Renderer::Instance().DisplayHeightAsFloat() * 0.8f, 0.0f);
  mLoading.setColor(mIsRecalboxRGBHat ? 0x9A9A9AFF : 0x656565FF);
}

void SplashView::Render(const Transform4x4f& parentTrans)
{
  Transform4x4f trans = (parentTrans * getTransform()).round();
  Renderer::SetMatrix(trans);

  Renderer::DrawRectangle(0, 0, Renderer::Instance().DisplayWidthAsInt(), Renderer::Instance().DisplayHeightAsInt(), mIsRecalboxRGBHat ? 0x35495EFF : 0xFFFFFFFF);

  int w = (int)(Renderer::Instance().DisplayWidthAsFloat() / (Renderer::Instance().Is480pOrLower() ? 5.0f : 10.0f));
  int h = (int)(Renderer::Instance().DisplayHeightAsFloat() / (Renderer::Instance().Is480pOrLower() ? 70.0f : 80.0f));
  int x = (int)((Renderer::Instance().DisplayWidthAsFloat() - (float)w) / 2.0f);
  int y = (int)(Renderer::Instance().DisplayHeightAsFloat() * 0.9f);
  Renderer::DrawRectangle(x, y, w, h, mIsRecalboxRGBHat ? 0x404040FF : 0xC0C0C0FF);
  if (mSystemCount != 0)
  {
    int max = Math::clampi(mSystemCount, 1, INT32_MAX);
    int val = Math::clampi(mSystemLoaded, 0, max);
    w = (w * val) / max;
    Renderer::DrawRectangle(x, y, w, h, mIsRecalboxRGBHat ? 0xA0A0A0FF : 0x606060FF);
  }

  mLogo.Render(trans);
  mLoading.Render(trans);

  Component::Render(trans);
}

void SplashView::SetMaximum(int maximum)
{
  mSystemCount = maximum;
  mWindow.DoWake(); // Kill screensaver
  mWindow.RenderAll();
}

void SplashView::SetProgress(int value)
{
  mSystemLoaded = value;
  mWindow.DoWake(); // Kill screensaver
  mWindow.RenderAll();
}

void SplashView::Increment()
{
  if (++mSystemLoaded >= mSystemCount) mSystemLoaded = mSystemCount - 1;
  mWindow.DoWake(); // Kill screensaver
  mWindow.RenderAll();
}

void SplashView::Quit()
{
  mSystemLoaded = mSystemCount = 0;
  mLoading.setText(_("PLEASE WAIT..."));
}

void SplashView::SystemLoadingPhase(ISystemLoadingPhase::Phase phase)
{
  switch(phase)
  {
    case Phase::RegularSystems: mLoading.setText(_("LOADING SYSTEMS...")); break;
    case Phase::VirtualSystems: mLoading.setText(_("LOADING VIRTUAL SYSTEMS...")); break;
    case Phase::Completed: mLoading.setText(_("INITIALIZING INTERFACE...")); break;
    default: break;
  }
}

