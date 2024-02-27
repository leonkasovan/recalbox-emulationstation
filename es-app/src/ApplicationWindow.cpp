//
// Created by bkg2k on 19/11/2019.
//

#include "ApplicationWindow.h"
#include "RotationManager.h"

bool ApplicationWindow::ProcessInput(const InputCompactEvent& event)
{
  if (event.IsKeyboard() && event.KeyUp() && (event.RawEvent().Code() == SDLK_F4))
  {
    WindowManager::ProcessInput(event); // Force window wakeup
    mClosed = true;
    return true;
  }
  InputCompactEvent rotated(event);
  if (RotationManager::ShouldRotateFrontendControls()) rotated.Rotate();
  if (WindowManager::ProcessInput(rotated)) return true;
  return mViewController.ProcessInput(rotated);
}

void ApplicationWindow::Rotate(RotationType rotation)
{
  {LOG(LogDebug) << "[ApplicationWindow] Starting rotation"; }
  Renderer::Instance().Rotate(rotation);
  {LOG(LogDebug) << "[ApplicationWindow] Finalizing windows"; }
  Finalize();
  {LOG(LogDebug) << "[ApplicationWindow] Init windows with renderer size"; }
  Initialize(Renderer::Instance().RealDisplayWidthAsInt(),Renderer::Instance().RealDisplayHeightAsInt());
}

void ApplicationWindow::Update(int deltaTime)
{
  mViewController.Update(deltaTime);
  WindowManager::Update(deltaTime);
}

void ApplicationWindow::Render(Transform4x4f& transform)
{
  mViewController.Render(transform);
  WindowManager::Render(transform);

  if (mActiveOSD)
    mOSD.Render(transform);
}

bool ApplicationWindow::UpdateHelpSystem()
{
  if (!WindowManager::UpdateHelpSystem())
    mViewController.updateHelpPrompts();
  return true;
}

void ApplicationWindow::EnableOSDImage(const Path& imagePath, float x, float y, float width, float height, float alpha, bool autoCenter)
{
  mOSD.setImage(imagePath);
  if (autoCenter)
  {
    mOSD.setOrigin(0.5f, 0.5f);
    x = y = 0.5f;
  }
  mOSD.setPosition(x * Renderer::Instance().DisplayWidthAsFloat(), y * Renderer::Instance().DisplayHeightAsFloat());
  mOSD.setResize(width * Renderer::Instance().DisplayWidthAsFloat(), height * Renderer::Instance().DisplayHeightAsFloat());
  mOSD.setKeepRatio(true);
  mOSD.setOpacity((unsigned char)(alpha * 255));
  mActiveOSD = true;
}

void ApplicationWindow::DisableOSDImage()
{
  mActiveOSD = false;
}
