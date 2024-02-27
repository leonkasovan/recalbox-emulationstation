//
// Created by bkg2k on 26/09/23.
//

#include "OSDManager.h"
#include "input/InputManager.h"
#include "themes/MenuThemeData.h"

OSDManager::OSDManager(WindowManager& window)
  : Gui(window)
  , mBluetoothOSD(window, BaseOSD::Side::Left)
  , mPadOSD(window, BaseOSD::Side::Left)
  , mFpsOSD(window, BaseOSD::Side::Right)
  , mBatteryOSD(window, BaseOSD::Side::Right)
{
  // Build OSD lists
  mOSDList.Add(&mBluetoothOSD);
  mOSDList.Add(&mPadOSD);
  mOSDList.Add(&mFpsOSD);
  mOSDList.Add(&mBatteryOSD);

  setPosition(0, 0);
  setSize(Renderer::Instance().DisplayWidthAsFloat(), Renderer::Instance().DisplayHeightAsFloat());
}

bool OSDManager::ProcessInput(const InputCompactEvent& event)
{
  for(BaseOSD* osd : mOSDList)
    if (osd->IsActive())
      if (osd->IsInputAware())
        osd->ProcessInput(event);
  return false;
}

void OSDManager::Update(int deltaTime)
{
  for(BaseOSD* osd : mOSDList)
    if (osd->IsActive())
      osd->Update(deltaTime);
}

void OSDManager::Render(const Transform4x4f& parentTrans)
{
  Renderer& renderer = Renderer::Instance();
  Renderer::SetMatrix(parentTrans);

  // Display left
  float x = Board::Instance().CrtBoard().IsCrtAdapterAttached() ?
            Math::round(renderer.DisplayWidthAsFloat() / 20.f) :
            Math::round(renderer.DisplayWidthAsFloat() / 80.f);
  float yl = Board::Instance().CrtBoard().IsCrtAdapterAttached() ?
             Math::round(renderer.DisplayHeightAsFloat() / 20.f) :
             Math::round(renderer.DisplayHeightAsFloat() / 80.f);
  float yr = yl;
  float gap = Math::round(renderer.DisplayHeightAsFloat() / 40.f);
  for(BaseOSD* osd : mOSDList)
    if (osd->IsActive())
      switch(osd->WhichSide())
      {
        case BaseOSD::Side::Left:
        {
          // Moving transformation
          Transform4x4f transform = parentTrans;
          transform.translate(x, yl);
          Renderer::SetMatrix(transform);
          osd->Render(transform);
          // Move
          yl += (float)osd->OSDAreaHeight() + gap;
          break;
        }
        case BaseOSD::Side::Right:
        {
          // Moving transformation
          Transform4x4f transform = parentTrans;
          transform.translate(renderer.DisplayWidthAsFloat() - x - (float)osd->OSDAreaWidth(), yr);
          Renderer::SetMatrix(transform);
          osd->Render(transform);
          // Move
          yr += (float)osd->OSDAreaHeight() + gap;
          break;
        }
      }
}
