//
// Created by bkg2k on 02/10/23.
//

#include "PadOSD.h"
#include <input/InputManager.h>

PadOSD::PadOSD(WindowManager& window, Side side)
  : BaseOSD(window, side, true)
  , mMapper(InputManager::Instance().Mapper())
  , mFont(Font::get(Renderer::Instance().DisplayHeightAsInt() / 32))
  , mPadGlyph()
  , mAlpha { sMinAlpha, sMinAlpha, sMinAlpha, sMinAlpha, sMinAlpha, sMinAlpha, sMinAlpha, sMinAlpha, sMinAlpha, sMinAlpha }
  , mPadChar(0)
  , mPadCount(0)
  , mActive(false)
  , mForcedActive(false)
{
  RecalboxConf::Instance().Watch(RecalboxConf::sPadOSD, *this);
  UpdatePadIcon();
  UpdateActiveFlag();
}

void PadOSD::UpdatePadIcon()
{
  mPadChar = 0xF25E;
  switch(RecalboxConf::Instance().GetPadOSDType())
  {
    case RecalboxConf::PadOSDType::MD: mPadChar = 0xF26C; break;
    case RecalboxConf::PadOSDType::XBox: mPadChar = 0xF2F0; break;
    case RecalboxConf::PadOSDType::PSX: mPadChar = 0xF2C8; break;
    case RecalboxConf::PadOSDType::N64: mPadChar = 0xF260; break;
    case RecalboxConf::PadOSDType::DC: mPadChar = 0xF26E; break;
    case RecalboxConf::PadOSDType::Snes:
    default: break;
  }
  mPadGlyph = mFont->Character(mPadChar);
}

bool PadOSD::ProcessInput(const InputCompactEvent& event)
{
  // Pad alpha
  if (event.Device().IsPad())
    if (event.AnythingPressed() || event.AnythingReleased())
      if (int padIndex = InputManager::Instance().Mapper().PadIndexFromDeviceIdentifier(event.RawEvent().Device()); padIndex >= 0)
        mAlpha[padIndex] = sMaxAlpha;
  return false;
}

void PadOSD::Update(int deltaTime)
{
  // Update pads
  if (!mActive && !mForcedActive) return;
  // Set alphas
  mPadCount = InputManager::Instance().Mapper().ConnectedPadCount();
  for(int i = Input::sMaxInputDevices; --i >= 0; )
    mAlpha[i] = Math::clampi(mAlpha[i] -= deltaTime / 4, i < mPadCount ? sMinAlpha : 0, sMaxAlpha);
}

void PadOSD::Render(const Transform4x4f& transform)
{
  (void)transform;

  static int flashing = 0;
  InputManager& inputManager = InputManager::Instance();
  int step = Renderer::Instance().DisplayHeightAsInt() / 32;
  int w = (int) mPadGlyph.advance.x() + Renderer::Instance().DisplayWidthAsInt() / 160;
  int bh = (int) mPadGlyph.bearing.y();
  const InputMapper::PadList padList = mMapper.GetPads();
  for (int i = mPadCount; --i >= 0;)
  {
    int y = i * (step + 1);
    mFont->renderCharacter(mPadChar, 0, (float) y, 1.f, 1.f, sColor | mAlpha[i]);
    InputDevice& device = inputManager.GetDeviceConfigurationFromIndex(padList[i].mIndex);
    if (device.HasBatteryLevel() && (device.BatteryLevel() > 15 || ((flashing >> 3) & 3) != 0))
    {
      float hr = (float) bh / (float) step;
      mFont->renderCharacter(device.BatteryLevelIcon(), (float)w, (float) y, hr, hr, sColor | mAlpha[i]);
    }
  }
  flashing++;
}

int PadOSD::OSDAreaWidth() const
{
  return 2 * (int) mPadGlyph.advance.x();
}

int PadOSD::OSDAreaHeight() const
{
  return ((Renderer::Instance().DisplayHeightAsInt() / 32) + 1) * mPadCount;
}

void PadOSD::UpdateActiveFlag()
{
  mActive = RecalboxConf::Instance().GetPadOSD() || mForcedActive;
}
