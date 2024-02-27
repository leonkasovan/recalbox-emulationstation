//
// Created by bkg2k on 13/02/2021.
//
// As part of the RECALBOX Project
// http://www.recalbox.com
//

#include "WizardRG353X.h"
#include <RecalboxConf.h>

Path WizardRG353X::OnImageRequired(int page)
{
  switch((Pages)page)
  {
    case Pages::VolumeButtons:
    case Pages::BritghtnessButtons: return Path(sImagePath) / sVolumeImageFile;
    case Pages::FunctionKey: return Path(sImagePath) / sFunctionKey;
    case Pages::Intro:
    case Pages::PowerOff:
    case Pages::PowerSuspend:
    case Pages::Final: return Path(sImagePath) / sIntroImageFile;
    case Pages::Count: break;
  }

  return Path();
}

String WizardRG353X::OnTextRequired(int page)
{
  switch((Pages)page)
  {
    case Pages::Intro: return _("Welcome to RECALBOX for Anbernic RG!\nThis little presentation will show you how to use all the special buttons available all around the screen.\n\nPress any button to start!");
    case Pages::VolumeButtons: return  _("On the left side of the console, there are 2 buttons marked with a '-' and a '+'.\nUse them to raise or lower the volume at any time in the Recalbox interface or in-game.\n\nPress button B to continue");
    case Pages::FunctionKey: return  _("On the top side of the console, there is a button marked 'F'.\nThis is the Hotkey button\nTo quit a game press both F + START\nPress button B to continue");
    case Pages::BritghtnessButtons: return  _("Use the function F button and a volume button to adjust brightness\nPress button B to continue");
    case Pages::PowerSuspend: return _("Just a few words about the POWER button.\nMake a short press, just like a mouse click, and your console will enter sleep mode. Make another short press and your console will restart instanly! Work in Recalbox interface and in-game!\n\nPress button B to continue.");
    case Pages::PowerOff: return _("If you push the POWER button more than 2 seconds, this will power-off your console. If you do so in-game, Recalbox will close the current emulator gracefully.\nJust in case, holding the POWER button down more than 5s perform an hard power-off.\n\nPress button B to continue.");
    case Pages::Final: return  _("Now you're ready to start your RETROGAMING experience with Recalbox! Press button B to start... and PLAY AGAIN!");
    case Pages::Count: break;
  }

  return String();
}

SimpleWizardBase::Move WizardRG353X::OnKeyReceived(int page, const InputCompactEvent& event)
{
  if (event.CancelPressed()) return Move::Backward;

  switch((Pages)page)
  {
    case Pages::Intro:
    {
      if (event.AnyButtonPressed()) return Move::Foreward;
      break;
    }
    case Pages::VolumeButtons:
    {
      if (event.AnyButtonPressed()) return Move::Foreward;
      break;
    }
    case Pages::FunctionKey:
    case Pages::BritghtnessButtons:
    case Pages::PowerSuspend:
    case Pages::PowerOff:
    {
      if (event.ValidPressed()) return Move::Foreward;
      break;
    }
    case Pages::Final:
    {
      if (event.ValidPressed())
      {
        RecalboxConf::Instance().SetFirstTimeUse(false);
        return Move::Close;
      }
      break;
    }
    case Pages::Count: break;
  }

  if (event.UpPressed()) ChangeLanguage(true);
  if (event.DownPressed()) ChangeLanguage(false);

  return Move::None;
}
