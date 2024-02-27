//
// Created by bkg2k on 03/10/23.
//

#include "BatteryOSD.h"

BatteryOSD::BatteryOSD(WindowManager& window, Side side)
  : BaseOSD(window, side, false)
  , mFont(Font::get(Renderer::Instance().Is240p() ? (int)FONT_SIZE_LARGE : (int)FONT_SIZE_MEDIUM, Font::sRecalboxIconPath))
  , mBatteryArea(mFont->sizeText("\uf1b4"))
  , mIcon(0)
  , mColor(0)
  , mHasBattery(Board::Instance().HasBattery())
  , mVisible(!RecalboxConf::Instance().GetBatteryHidden())
{
  // Expand 1 pixel in all directions since the icon is drawn 1 pixel shifted in all directions
  mBatteryArea.Expand(1.f, 1.f);
  RecalboxConf::Instance().Watch(RecalboxConf::sBatteryHidden, *this);
}

void BatteryOSD::Update(int deltaTime)
{
  (void)deltaTime;

  mIcon = 0xf1b4;
  mColor = 0xFFFFFFFF;
  if (!Board::Instance().IsBatteryCharging())
  {
    // icon
    int charge = Board::Instance().BatteryChargePercent();
    if (charge >= 66)      mIcon = 0xF1ba;
    else if (charge >= 33) mIcon = 0xF1b8;
    else if (charge >= 15) mIcon = 0xF1b1;
    else                   mIcon = 0xF1b5;
    // color
    if (charge < 15)
    {
      mColor = 0xFF8000FF;
      if ((charge < 10) && ((SDL_GetTicks() >> 8) & 3) == 0) mColor = 0xFF0000FF;
    }
  }
}

void BatteryOSD::Render(const Transform4x4f& parentTrans)
{
  (void)parentTrans;
  mFont->renderCharacter(mIcon, 2, 1, 1.f, 1.f, 0xFF);
  mFont->renderCharacter(mIcon, 0, 1, 1.f, 1.f, 0xFF);
  mFont->renderCharacter(mIcon, 1, 2, 1.f, 1.f, 0xFF);
  mFont->renderCharacter(mIcon, 1, 0, 1.f, 1.f, 0xFF);
  mFont->renderCharacter(mIcon, 1, 1, 1.f, 1.f, mColor);
}
